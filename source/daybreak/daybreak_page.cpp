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

void DaybreakPage::InstallUpdate() {
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

    bool bottom_level;
    if(R_FAILED(rc = IsPathBottomLevel(current_path, &bottom_level))) {
        fatalThrow(rc);
    }

    m_has_drawn = true;

    if(!R_FAILED(GetUpdateInformation())) {
        brls::Logger::info("Validating update...");
    }

    this->DaybreakExit();
    return;
}