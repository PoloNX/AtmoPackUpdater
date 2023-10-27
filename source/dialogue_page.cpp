#include "dialogue_page.hpp"

#include <algorithm>
#include <filesystem>
#include <switch.h>
#include <worker_page.hpp>

#include "fs.hpp"
#include "progress_event.hpp"
#include "main_frame.hpp"
#include "utils.hpp"
#include "daybreak_utils.hpp"
#include "reboot.hpp"

namespace i18n = brls::i18n;
using namespace i18n::literals;



void DialoguePage::CreateView()
{
    this->button1 = (new brls::Button(brls::ButtonStyle::REGULAR))->setLabel("menu/dialogue/yes"_i18n);
    this->button1->setParent(this);
    this->button2 = (new brls::Button(brls::ButtonStyle::REGULAR))->setLabel("menu/dialogue/no"_i18n);
    this->button2->setParent(this);

    this->instantiateButtons();

    this->label->setHorizontalAlign(NVG_ALIGN_CENTER);
    this->label->setParent(this);

    this->navigationMap.add(
        this->button1,
        brls::FocusDirection::RIGHT,
        this->button2);

    this->navigationMap.add(
        this->button2,
        brls::FocusDirection::LEFT,
        this->button1);

    this->registerAction("", brls::Key::B, [this] { return true; });
}

void DialoguePage::draw(NVGcontext* vg, int x, int y, unsigned width, unsigned height, brls::Style* style, brls::FrameContext* ctx)
{
    this->label->frame(ctx);
    this->button1->frame(ctx);

    auto end = std::chrono::high_resolution_clock::now();
    auto missing = std::max(1l - std::chrono::duration_cast<std::chrono::seconds>(end - start).count(), 0l);
    auto text = std::string("menus/common/no"_i18n);
    if (missing > 0) {
        this->button2->setLabel(text + " (" + std::to_string(missing) + ")");
        this->button2->setState(brls::ButtonState::DISABLED);
    }
    else {
        this->button2->setLabel(text);
        this->button2->setState(brls::ButtonState::ENABLED);
    }
    this->button2->invalidate();
    this->button2->frame(ctx);
}

void DialoguePage::layout(NVGcontext* vg, brls::Style* style, brls::FontStash* stash)
{
    this->label->setWidth(this->width);
    this->label->invalidate(true);
    this->label->setBoundaries(
        this->x + this->width / 2 - this->label->getWidth() / 2,
        this->y + (this->height - this->label->getHeight() - this->y - style->CrashFrame.buttonHeight) / 2,
        this->label->getWidth(),
        this->label->getHeight());
    this->button1->setBoundaries(
        this->x + this->width / 2 - style->CrashFrame.buttonWidth / 2 - 200,
        this->y + (this->height - style->CrashFrame.buttonHeight * 3),
        style->CrashFrame.buttonWidth,
        style->CrashFrame.buttonHeight);
    this->button1->invalidate();

    this->button2->setBoundaries(
        this->x + this->width / 2 - style->CrashFrame.buttonWidth / 2 + 200,
        this->y + (this->height - style->CrashFrame.buttonHeight * 3),
        style->CrashFrame.buttonWidth,
        style->CrashFrame.buttonHeight);
    this->button2->invalidate();

    start = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(150);
}

brls::View* DialoguePage::getDefaultFocus()
{
    return this->button1;
}

brls::View* DialoguePage::getNextFocus(brls::FocusDirection direction, brls::View* currentView)
{
    return this->navigationMap.getNextFocus(direction, currentView);
}

void DialoguePage_fw::instantiateButtons()
{
    this->button2->getClickEvent()->subscribe([this](View* view) {
        if (!frame->isLastStage())
            frame->nextStage();
        else {
            brls::Application::pushView(new MainFrame());
        }
    });

    this->button1->getClickEvent()->subscribe([this](View* view) {
        if(!util::isErista()) {
            return;
        }

        //constinit u32 g_supported_version = std::numeric_limits<u32>::max();

        bool g_initialized = false;

        /* Update install state. */
        char g_update_path[FS_MAX_PATH];
        bool g_reset_to_factory = false;
        bool g_exfat_supported = false;
        bool g_use_exfat = false;

        /* variables to record some results */
        AmsSuUpdateInformation m_update_info;

        AmsSuUpdateValidationInfo m_validation_info;
        bool m_has_drawn;
        bool m_has_info;
        bool m_has_validated = true;

        InstallState m_install_state;
        AsyncResult m_prepare_result;
        float m_progress_percent = 0.0f;

        if (!InitializeMenu()) {
            brls::Logger::error("Failed to initialize menu");
            return;
        }
        brls::StagedAppletFrame* appletFrame = new brls::StagedAppletFrame();

        char current_path[FS_MAX_PATH];
        strncpy(current_path,"sdmc:/firmware/", sizeof(current_path));
        snprintf(g_update_path, sizeof(g_update_path), "%s", current_path);
        m_has_drawn = true;

        appletFrame->addStage(new WorkerPage(appletFrame, "menu/update/download"_i18n, [this]() {
            Result rc = 0;
            if (!R_FAILED(GetUpdateInformation())) {
                printf("Validating update, this may take a moment...\n");
            }
        }));

        if (m_has_info && m_has_drawn && !m_has_validated) {
		    ValidateUpdate();
	    }
        if (!m_has_validated || R_FAILED(m_validation_info.result)) {
            printf("Unsuccesful validation of the update.\n");
            return;
        }

        g_exfat_supported = m_update_info.exfat_supported && R_SUCCEEDED(m_validation_info.exfat_result);
        if (!g_exfat_supported) {
            g_use_exfat = false;
        }
        else if (g_exfat_supported) {
            int g_use_exfat_int = util::showDialogBoxBlocking("menu/dialogue/exfat"_i18n, "yes", "no");
            if (g_use_exfat_int == 0) {
                g_use_exfat = true;
            } else {
                g_use_exfat = false;
            }
        } else {
            g_use_exfat = false;
        }

        bool choose_fat32_forced = false;
        if (!g_exfat_supported && g_use_exfat) {
            util::showDialogBoxBlocking("menu/dialogue/fat32"_i18n, "ok");
            g_use_exfat = false;
        }
        //Warn the user if they're updating with exFAT supposed to be supported but not present/corrupted.
        if (g_use_exfat == true && m_update_info.exfat_supported && R_FAILED(m_validation_info.exfat_result)) {
            printf("Error: exFAT firmware is missing or corrupt.\n");
            return;
        }

        //TODO : Factory reset question

        const u32 version = m_validation_info.invalid_key.version;
        bool force_update_not_supported = false;
        if (EncodeVersion((version >> 26) & 0x1f, (version >> 20) & 0x1f, (version >> 16) & 0xf) > g_supported_version) {
            int force_update_int = util::showDialogBoxBlocking("menu/dialogue/force_update"_i18n, "yes", "no");
            if (force_update_int == 0) {
                force_update_not_supported = true;
            } else {
                force_update_not_supported = false;
            }
            if (!force_update_not_supported) {
                printf("Error: firmware is too new and not forced for installation, update is canceled.\n");
                return;
            }
        }

        int g_reset_to_factory_int = util::showDialogBoxBlocking("menu/dialogue/reset_to_factory"_i18n, "yes", "no");
        if (g_reset_to_factory_int == 0) {
            g_reset_to_factory = true;
        } else {
            g_reset_to_factory = false;
        }

        hiddbgDeactivateHomeButton();

        m_install_state = InstallState::NeedsDraw;
        if (m_install_state == InstallState::NeedsDraw) {
            printf("Beginning update setup...\n");
            consoleUpdate(NULL);
            m_install_state = InstallState::NeedsSetup;
        }
        if (m_install_state != InstallState::NeedsDraw && m_install_state != InstallState::AwaitingReboot) {
            TransitionUpdateState();
        }
        if (m_install_state != InstallState::NeedsSetup && m_install_state != InstallState::AwaitingReboot) {
            TransitionUpdateState();
        }
        if (m_install_state != InstallState::NeedsPrepare && m_install_state != InstallState::AwaitingReboot) {
            appletFrame->addStage(new WorkerPage(appletFrame, "menu/update/prepare"_i18n, [this, m_install_state]() {
                ProgressEvent::instance().setStep(0);
                while (1) {
                    if (m_install_state == InstallState::NeedsApply || m_install_state == InstallState::AwaitingReboot) {
                        break;
                    }
                    TransitionUpdateState();
                    consoleUpdate(NULL);
                }
            }));
        }
        if (m_install_state != InstallState::AwaitingPrepare && m_install_state != InstallState::AwaitingReboot) {
            TransitionUpdateState();
        }
       
        reboot::rebootNow();

        brls::Application::pushView(appletFrame);
    });

    this->label = new brls::Label(brls::LabelStyle::DIALOG, fmt::format("{}\n\n{}", this->text, "menu/dialogue/launch_daybreak"_i18n), true);
}

void DialoguePage_fw::draw(NVGcontext* vg, int x, int y, unsigned width, unsigned height, brls::Style* style, brls::FrameContext* ctx)
{
    this->label->frame(ctx);
    this->button1->frame(ctx);
    this->button2->frame(ctx);
}

void DialoguePage_theme::instantiateButtons() {
    this->button1->getClickEvent()->subscribe([this](View* view) {
        util::deleteTheme();
        if (!frame->isLastStage()){
            frame->nextStage();
        }
        else {
            brls::Application::pushView(new MainFrame());
        }
    });

    this->button2->getClickEvent()->subscribe([this](View* view) {
        if (!frame->isLastStage())
            frame->nextStage();
        else {
            brls::Application::pushView(new MainFrame());
        }
    });

    this->label = new brls::Label(brls::LabelStyle::DIALOG, fmt::format("{}\n\n{}", this->text, "menu/dialogue/delete_theme"_i18n), true);
}

void DialoguePage_theme::draw(NVGcontext* vg, int x, int y, unsigned width, unsigned height, brls::Style* style, brls::FrameContext* ctx)
{
    this->label->frame(ctx);
    this->button1->frame(ctx);
    this->button2->frame(ctx);
}