#include "daybreak_page.hpp"
#include "main_frame.hpp"

namespace i18n = brls::i18n;
using namespace i18n::literals;

void DaybreakPage::instantiateButtons()
{
    this->button2->getClickEvent()->subscribe([this](View* view) {
        if (!frame->isLastStage())
            frame->nextStage();
        else {
            brls::Application::pushView(new MainFrame());
        }
    });

    this->button1->getClickEvent()->subscribe([this](View* view) {
        this->InstallUpdate();
        if (!frame->isLastStage())
            frame->nextStage();
        else {
            brls::Application::pushView(new MainFrame());
        }
    });

    this->label = new brls::Label(brls::LabelStyle::DIALOG, fmt::format("{}\n\n{}", this->text, "menu/dialogue/launch_daybreak"_i18n), true);
}

void DaybreakPage::draw(NVGcontext* vg, int x, int y, unsigned width, unsigned height, brls::Style* style, brls::FrameContext* ctx)
{
    this->label->frame(ctx);
    this->button1->frame(ctx);
    this->button2->frame(ctx);
}

u32 DaybreakPage::EncodeVersion(u32 major, u32 minor, u32 micro, u32 relstep)
{
    return ((major & 0xFF) << 24) | ((minor & 0xFF) << 16) | ((micro & 0xFF) << 8) | ((relstep & 0xFF) << 8);
}

extern "C" {
    bool DaybreakPage::DaybreakInitializeMenu() {
        Result rc = 0;
        g_initialized = true;

        u64 version;
        if (R_FAILED(rc = splGetConfig(static_cast<SplConfigItem>(ExosphereApiVersionConfigItem), &version))) {
            brls::Logger::error("Failed to get Exosphere API version: 0x{:08X}", rc);
            return false;
        }
        brls::Logger::info("Exosphere API version: 0x{:016X}", version);

        const u32 version_micro = (version >> 40) & 0xff;
        const u32 version_minor = (version >> 48) & 0xff;
        const u32 version_major = (version >> 46) & 0xff;

        const bool ams_supports_sysupdate_api = EncodeVersion(version_major, version_minor, version_micro) >= EncodeVersion(0, 14, 0);
        if(!ams_supports_sysupdate_api) {
            brls::Logger::error("Atmosphere does not support the system update API");
            return false;
        }
        brls::Logger::info("Atmosphere supports the system update API");

        if (envIsNso()) {
            brls::Logger::error("Cannot run this from an NSO");
            return false;
        }
        brls::Logger::info("Running from a NRO");

        if (R_FAILED(rc = amssuInitialize())) {
            brls::Logger::error("Failed to initialize AMS system update: 0x{:08X}", rc);
            fatalThrow(rc);
            return false;
        }
        brls::Logger::info("Initialized AMS system update");

        return true;
    }
}

void DaybreakPage::InstallUpdate() {
    strcpy(this->g_update_path, "sdmc:/firmware/");

    DaybreakInitializeMenu();

    return;
}