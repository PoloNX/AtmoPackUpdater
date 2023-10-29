#include "daybreak_page.hpp"
#include "main_frame.hpp"
#include "utils.hpp"
#include "worker_page.hpp"
#include "progress_event.hpp"
#include "reboot.hpp"
#include "confirm_page.hpp"
#include <sys/stat.h>

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

void DaybreakPage::DaybreakInit() {
    Result rc = 0;

	if (R_FAILED(rc = spsmInitialize())) {
		fatalThrow(rc);
	}

	if (R_FAILED(rc = plInitialize(PlServiceType_User))) {
		fatalThrow(rc);
	}

	if (R_FAILED(rc = splInitialize())) {
		fatalThrow(rc);
	}

	if (R_FAILED(rc = nsInitialize())) {
		fatalThrow(rc);
	}

	if (R_FAILED(rc = hiddbgInitialize())) {
		fatalThrow(rc);
	}
}

void DaybreakPage::DaybreakExit() {
    hiddbgExit();
	nsExit();
	splExit();
	plExit();
	spsmExit();
	amssuExit();
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

    Result DaybreakPage::GetUpdateInformation() {
        Result rc = 0;
        /* Attempt to get the update information. */
        if (R_FAILED(rc = amssuGetUpdateInformation(&m_update_info, g_update_path))) {
            if (rc == 0x1a405) {
                brls::Logger::error("Update not found: 0x{:08X}", rc);
            } else {
                brls::Logger::error("Failed to get update information: 0x{:08X}", rc);
            }
            return rc;
        }
        /* Print update information. */
        brls::Logger::info("Update version: {}.{}.{}", (m_update_info.version >> 26) & 0x1f, (m_update_info.version >> 20) & 0x1f, (m_update_info.version >> 16) & 0xf);
        printf("\n");
        if (m_update_info.exfat_supported) {
            brls::Logger::info("Update supports exFAT");
        } else {
            brls::Logger::info("Update does not support exFAT");
        }
        brls::Logger::info("Firmware variations: {}", m_update_info.num_firmware_variations);
        /* Mark as having obtained update info. */
        m_has_info = true;
        return rc;
    }
}

void DaybreakPage::ValidateUpdate() {
    Result rc = 0;

    if(R_FAILED(rc = amssuValidateUpdate(&m_validation_info, g_update_path))) {
        brls::Logger::error("Failed to validate update: 0x{:08X}", rc);
        return;
    }

    if (R_SUCCEEDED(m_validation_info.exfat_result)) {
        if(R_FAILED(m_validation_info.exfat_result)) { 
            const u32 version = m_validation_info.invalid_key.version;
            brls::Logger::error("Invalid exFAT key: 0x{:016X}", version);
            brls::Logger::error("Missing Contents:\n- Program id: {}\n- Version: {}.{}.{}", m_validation_info.invalid_key.id, (version >> 26) & 0x1f, (version >> 20) & 0x1f, (version >> 16) & 0xf);

            for (size_t i = 0; i < sizeof(NcmContentId); i++) {
                brls::Logger::error("{}", m_validation_info.invalid_content_id.c[i]);
            }
        }
    }
    else {
        const u32 version = m_validation_info.invalid_key.version;
        brls::Logger::error("Can't valid update. Result : {}", m_validation_info.result);
        brls::Logger::error("Missing Contents:\n- Program id: {}\n- Version: {}.{}.{}", m_validation_info.invalid_key.id, (version >> 26) & 0x1f, (version >> 20) & 0x1f, (version >> 16) & 0xf);
        for (size_t i = 0; i < sizeof(NcmContentId); i++) {
            brls::Logger::error("{}", m_validation_info.invalid_content_id.c[i]);
        }
    }

    m_has_validated = true;
}

Result IsPathBottomLevel(const char *path, bool *out) {
    Result rc = 0;
    FsFileSystem *fs;
    char translated_path[FS_MAX_PATH] = {};
    if(!static_cast<bool>(fsdevTranslatePath(path, &fs, translated_path) != -1)) {
        std::abort();
    }
    FsDir dir;
    if (R_FAILED(rc = fsFsOpenDirectory(fs, translated_path, FsDirOpenMode_ReadDirs, &dir))) {
        return rc;
    }

    s64 entry_count;
    if (R_FAILED(rc = fsDirGetEntryCount(&dir, &entry_count))) {
        return rc;
    }

    *out = entry_count == 0;
    fsDirClose(&dir);
    return rc;
}

Result DaybreakPage::TransitionUpdateState() {
    Result rc = 0;
	if (m_install_state == InstallState::NeedsSetup) {
		/* Setup the update. */
        brls::Logger::info("UpdateTaskBufferSize : {}", UpdateTaskBufferSize);
        brls::Logger::info("Update path : {}", g_update_path);
        brls::Logger::info("Use exfat : {}", g_use_exfat);
		if (R_FAILED(rc = amssuSetupUpdate(nullptr, UpdateTaskBufferSize, g_update_path, g_use_exfat))) {
			//printf(language_vars["lng_db_install_process_setup_error"], rc);
			brls::Logger::error("Failed to setup update: 0x{:08X}", rc);
			MarkForReboot();
			return rc;
		}

		/* Log setup completion. */
        brls::Logger::info("Setup complete");
		m_install_state = InstallState::NeedsPrepare;
	} else if (m_install_state == InstallState::NeedsPrepare) {
		/* Request update preparation. */
		if (R_FAILED(rc = amssuRequestPrepareUpdate(&m_prepare_result))) {
            brls::Logger::error("Failed to request update preparation: 0x{:08X}", rc);
			MarkForReboot();
			return rc;
		}

		/* Log awaiting prepare. */
		brls::Logger::info("Awaiting prepare...");
		m_install_state = InstallState::AwaitingPrepare;
	} else if (m_install_state == InstallState::AwaitingPrepare) {
		/* Check if preparation has a result. */
		if (R_FAILED(rc = asyncResultWait(&m_prepare_result, 0)) && rc != 0xea01) {
            brls::Logger::error("Failed to get update preparation result: 0x{:08X}", rc);
			MarkForReboot();
			return rc;
		} else if (R_SUCCEEDED(rc)) {
			if (R_FAILED(rc = asyncResultGet(&m_prepare_result))) {
                brls::Logger::error("Failed to get update preparation result: 0x{:08X}", rc);
				MarkForReboot();
				return rc;
			}
		}

		/* Check if the update has been prepared. */
		bool prepared;
		if (R_FAILED(rc = amssuHasPreparedUpdate(&prepared))) {
            brls::Logger::error("Failed to get update preparation result: 0x{:08X}", rc);
			MarkForReboot();
			return rc;
		}

		/* Mark for application if preparation complete. */
		if (prepared) {
            brls::Logger::info("Preparation complete");
            brls::Logger::info("Applying update...");
			m_install_state = InstallState::NeedsApply;
			return rc;
		}

		/* Check update progress. */
		NsSystemUpdateProgress update_progress = {};
		if (R_FAILED(rc = amssuGetPrepareUpdateProgress(&update_progress))) {
            brls::Logger::error("Failed to get update preparation progress: 0x{:08X}", rc);
			MarkForReboot();
			return rc;
		}

        ProgressEvent::instance().setStep(int(update_progress.current_size));
        ProgressEvent::instance().setTotalSteps(int(update_progress.total_size));

		// Update progress percent.
		if (static_cast<float>(update_progress.total_size) > 0.0f) {
			m_progress_percent = (static_cast<float>(update_progress.current_size) / static_cast<float>(update_progress.total_size)) * 100.0f;
			printf("\r* Update progress : %3.0f %s * \n", m_progress_percent, "%");
			// consoleUpdate(&logs_console);
		} else {
			m_progress_percent = 0.0f;
			printf("\r* Update progress : %3.0f %s * \n", m_progress_percent, "%");
			// consoleUpdate(&logs_console);
		}


		printf("\r* %10.0f on %10.0f  *", static_cast<float>(update_progress.current_size), static_cast<float>(update_progress.total_size));
		printf("\r* %i *", (static_cast<int>(update_progress.current_size) / static_cast<int>(update_progress.total_size)) * 100);
		printf("\r* Update progress : %3.0f %s *", m_progress_percent, "%");
		// consoleUpdate(&logs_console);
	} else if (m_install_state == InstallState::NeedsApply) {
		/* Apply the prepared update. */
		if (R_FAILED(rc = amssuApplyPreparedUpdate())) {
            brls::Logger::error("Failed to apply update: 0x{:08X}", rc);
		} else {
			/* Log success. */
            brls::Logger::info("Update applied successfully");

			if (g_reset_to_factory) {
				if (R_FAILED(rc = nsResetToFactorySettingsForRefurbishment())) {
					/* Fallback on ResetToFactorySettings. */
					if (rc == MAKERESULT(Module_Libnx, LibnxError_IncompatSysVer)) {
						if (R_FAILED(rc = nsResetToFactorySettings())) {
                            brls::Logger::error("Failed to reset to factory settings: 0x{:08X}", rc);
							MarkForReboot();
							return rc;
						}
					} else {
                        brls::Logger::info("Failed to reset to factory settings for refurbishment: 0x{:08X}", rc);
						MarkForReboot();
						return rc;
					}
				}
                brls::Logger::info("Reset to factory settings successful");
			}
		}

		MarkForReboot();
		return rc;
	}

	return rc;
}

void DaybreakPage::MarkForReboot() {
	m_install_state = InstallState::AwaitingReboot;
}

void DaybreakPage::InstallUpdate() {
    brls::StagedAppletFrame* appletFrame = new brls::StagedAppletFrame();
    appletFrame->setIcon("romfs:/icon/daybreak.png");

    
    this->DaybreakInit();
    chdir("sdmc:/");

    DaybreakInitializeMenu();

    Result rc = 0;
    u64 hardware_type;
    u64 has_rcm_bug_patch;
    u64 is_emummc;

    if (R_FAILED(rc = splGetConfig(SplConfigItem_HardwareType, &hardware_type))) {
        brls::Logger::error("Failed to get hardware type: 0x{:08X}", rc);
        return;
    }
    brls::Logger::info("Hardware type: 0x{:016X}", hardware_type);

    if (R_FAILED(rc = splGetConfig(static_cast<SplConfigItem>(ExosphereHasRcmBugPatch), &has_rcm_bug_patch))) {
        brls::Logger::error("Failed to get RCM bug patch status: 0x{:08X}", rc);
        return;
    }
    brls::Logger::info("RCM bug patch status: 0x{:016X}", has_rcm_bug_patch);

    if (R_FAILED(rc = splGetConfig(static_cast<SplConfigItem>(ExosphereEmummcType), &is_emummc))) {
        brls::Logger::error("Failed to get emummc type: 0x{:08X}", rc);
        return;
    }
    brls::Logger::info("Emummc type: 0x{:016X}", is_emummc);

    const bool is_erista = hardware_type == 0 || hardware_type == 1;
    if (is_erista && has_rcm_bug_patch && !is_emummc) {
        brls::Logger::info("We're on a patched unit");
        return;
    }

    char current_path[FS_MAX_PATH] = {};
    const int path_len = snprintf(current_path, sizeof(current_path), "%s%s/", "/", "firmware");
    if(!static_cast<bool>(path_len >= 0 && path_len < static_cast<int>(sizeof(current_path)))) {
        std::abort();
    }
    
    brls::Logger::info("Firmware path : {}", current_path);

    struct stat buf;
    brls::Logger::info(std::to_string(stat(current_path, &buf)));

    m_has_drawn = true;

    snprintf(g_update_path, sizeof(g_update_path), "%s", current_path);
    
    if(!R_FAILED(GetUpdateInformation())) {
        brls::Logger::info("Validating update...");
    }

    if (m_has_info && m_has_drawn && !m_has_validated) {
        ValidateUpdate();
    }

    if (!m_has_validated || R_FAILED(m_validation_info.result)) {
        brls::Logger::error("Failed to validate update, m_has_validated : {}, m_validation_info.result : {}", m_has_validated, m_validation_info.result);
        return;
    }

    g_exfat_supported = m_update_info.exfat_supported && R_SUCCEEDED(m_validation_info.exfat_result);
    if(!g_exfat_supported) {
        brls::Logger::info("Exfat not supported");
        g_use_exfat = false;
    }
    else {
        appletFrame->addStage(new ExFatPage(appletFrame, "menu/dialogue/exfat"_i18n, g_use_exfat));
    }

    appletFrame->addStage(new ResetPage(appletFrame, "menu/dialogue/reset"_i18n, g_reset_to_factory));

    appletFrame->addStage(new WorkerPage(appletFrame, "menu/dialogue/update"_i18n, [this]() {
        //ProgressEvent::instance().setTotalSteps(100); //Percents
        //ProgressEvent::instance().setStep(0);
        hiddbgDeactivateHomeButton();
        m_install_state = InstallState::NeedsDraw;
        if(m_install_state == InstallState::NeedsDraw) {
            brls::Logger::info("1- Install state : {}", m_install_state);
            m_install_state = InstallState::NeedsSetup;
        }

        if (m_install_state != InstallState::NeedsDraw && m_install_state != InstallState::AwaitingReboot) {
            brls::Logger::info("2- Install state : {}", m_install_state);
            TransitionUpdateState();
        }

        if (m_install_state != InstallState::NeedsSetup && m_install_state != InstallState::AwaitingReboot) {
		    brls::Logger::info("3- Install state : {}", m_install_state);
            TransitionUpdateState();
	    }

        if (m_install_state != InstallState::NeedsPrepare && m_install_state != InstallState::AwaitingReboot) {
            brls::Logger::info("4- Install state : {}", m_install_state);
            while (1) {
                if (m_install_state == InstallState::NeedsApply || m_install_state == InstallState::AwaitingReboot) {
                    break;
                }
                TransitionUpdateState();
                printf("\r* Update progress : %3.0f%s *\n", m_progress_percent, "%");
            }
	    }

        if (m_install_state != InstallState::AwaitingPrepare && m_install_state != InstallState::AwaitingReboot) {
            brls::Logger::info("5- Install state : {}", m_install_state);
            TransitionUpdateState();
        }


        if (m_install_state == InstallState::AwaitingReboot) {
            brls::Logger::info("6- Install state : {}", m_install_state);
            brls::Logger::info("Rebooting...");
            //reboot::rebootNow();
            ProgressEvent::instance().finished();
            this->DaybreakExit();
        }
    }));

    std::string doneMsg = "menu/update/download_finish"_i18n + "menu/update/apply_firmware"_i18n;
    
    appletFrame->addStage(new ConfirmPage(appletFrame, doneMsg, true, true, util::isErista()));

    appletFrame->setTitle(fmt::format("Daybreak Installing firmware {}.{}.{}", (m_update_info.version >> 26) & 0x1f, (m_update_info.version >> 20) & 0x1f, (m_update_info.version >> 16) & 0xf));

    brls::Application::pushView(appletFrame);
    
    return;
}

void ExFatPage::instantiateButtons() {
    this->button1->getClickEvent()->subscribe([this](View* view) {
        use_exfat = true;
        if (!frame->isLastStage())
            frame->nextStage();
        else {
            brls::Application::pushView(new MainFrame());
        }
        brls::Logger::info("Exfat choice : {}", use_exfat);
    });

    this->button2->getClickEvent()->subscribe([this](View* view) {
        use_exfat = false;
        if (!frame->isLastStage())
            frame->nextStage();
        else {
            brls::Application::pushView(new MainFrame());
        }
        brls::Logger::info("Exfat choice : {}", use_exfat);
    });

    this->label = new brls::Label(brls::LabelStyle::DIALOG, fmt::format("{}\n\n{}", this->text, "menu/dialogue/exfat"_i18n), true);
}


void ExFatPage::draw(NVGcontext* vg, int x, int y, unsigned width, unsigned height, brls::Style* style, brls::FrameContext* ctx)
{
    this->label->frame(ctx);
    this->button1->frame(ctx);
    this->button2->frame(ctx);
}

void ResetPage::instantiateButtons() {
    this->button1->getClickEvent()->subscribe([this](View* view) {
        reset_to_factory = true;
        if (!frame->isLastStage())
            frame->nextStage();
        else {
            brls::Application::pushView(new MainFrame());
        }
        brls::Logger::info("Reset choice : {}", reset_to_factory);
    });

    this->button2->getClickEvent()->subscribe([this](View* view) {
        reset_to_factory = false;
        if (!frame->isLastStage())
            frame->nextStage();
        else {
            brls::Application::pushView(new MainFrame());
        }
        brls::Logger::info("Reset choice : {}", reset_to_factory);
    });

    this->label = new brls::Label(brls::LabelStyle::DIALOG, fmt::format("{}\n\n{}", this->text, "menu/dialogue/reset"_i18n), true);
}


void ResetPage::draw(NVGcontext* vg, int x, int y, unsigned width, unsigned height, brls::Style* style, brls::FrameContext* ctx)
{
    this->label->frame(ctx);
    this->button1->frame(ctx);
    this->button2->frame(ctx);
}

