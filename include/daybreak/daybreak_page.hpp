#pragma once

#include <borealis.hpp>
#include <switch.h>
#include <dialogue_page.hpp>
#include "ams_su.h"

class DaybreakPage : public DialoguePage {
private:
    void instantiateButtons() override;
    std::string text;
    brls::StagedAppletFrame* frame;

    static constexpr u32 ExosphereApiVersionConfigItem = 65000;
    static constexpr u32 ExosphereHasRcmBugPatch	   = 65004;
    static constexpr u32 ExosphereEmummcType		   = 65007;
    static constexpr u32 ExosphereSupportedHosVersion  = 65011;

    u32 g_supported_version = std::numeric_limits<u32>::max();

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
    bool m_has_validated;
    enum class InstallState {
        NeedsDraw,
        NeedsSetup,
        NeedsPrepare,
        AwaitingPrepare,
        NeedsApply,
        AwaitingReboot,
    };
    static constexpr size_t UpdateTaskBufferSize = 0x100000;
    InstallState m_install_state;
    AsyncResult m_prepare_result;
    float m_progress_percent = 0.0f;

    /* Functions to update firmware */
    u32 EncodeVersion(u32 major, u32 minor, u32 micro, u32 relstep = 0);
    bool DaybreakInitializeMenu();
    Result GetUpdateInformation();
    void ValidateUpdate();
    void MarkForReboot();
    Result TransitionUpdateState();
    void InstallUpdate();
    void DaybreakInit();
    void DaybreakExit();
public:
    DaybreakPage(brls::StagedAppletFrame* frame, const std::string& text) : DialoguePage(), text(text), frame(frame) { CreateView(); }
    void draw(NVGcontext* vg, int x, int y, unsigned width, unsigned height, brls::Style* style, brls::FrameContext* ctx) override;
};
