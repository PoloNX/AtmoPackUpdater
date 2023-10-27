#pragma once

#include <switch.h>
#include "daybreak.h"
#include <limits>

static constexpr u32 ExosphereApiVersionConfigItem = 65000;
static constexpr u32 ExosphereHasRcmBugPatch	   = 65004;
static constexpr u32 ExosphereEmummcType		   = 65007;
static constexpr u32 ExosphereSupportedHosVersion  = 65011;

extern constinit u32 g_supported_version;

extern bool g_initialized;

/* Update install state. */
extern char g_update_path[FS_MAX_PATH];
extern bool g_reset_to_factory;
extern bool g_exfat_supported;
extern bool g_use_exfat;

/* variables to record some results */
extern AmsSuUpdateInformation m_update_info;

extern AmsSuUpdateValidationInfo m_validation_info;
extern bool m_has_drawn;
extern bool m_has_info;
extern bool m_has_validated;
enum class InstallState {
	NeedsDraw,
	NeedsSetup,
	NeedsPrepare,
	AwaitingPrepare,
	NeedsApply,
	AwaitingReboot,
};
static constexpr size_t UpdateTaskBufferSize = 0x100000;
extern InstallState m_install_state;
extern AsyncResult m_prepare_result;
extern float m_progress_percent;

Result TransitionUpdateState();
void MarkForReboot();
void ValidateUpdate();
Result GetUpdateInformation();
u32 EncodeVersion(u32 major, u32 minor, u32 micro, u32 relstep = 0);
bool InitializeMenu();