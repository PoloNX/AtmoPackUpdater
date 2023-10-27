#include "daybreak_utils.hpp"
#include <switch.h>
#include <stdio.h>
#include "progress_event.hpp"

constinit u32 g_supported_version;

bool g_initialized;

// Update install state. 
char g_update_path[FS_MAX_PATH];
bool g_reset_to_factory;
bool g_exfat_supported;
bool g_use_exfat;

// variables to record some results 
AmsSuUpdateInformation m_update_info;

AmsSuUpdateValidationInfo m_validation_info;
bool m_has_drawn;
bool m_has_info;
bool m_has_validated;

InstallState m_install_state;
AsyncResult m_prepare_result;
float m_progress_percent;

u32 EncodeVersion(u32 major, u32 minor, u32 micro, u32 relstep) {
	return ((major & 0xFF) << 24) | ((minor & 0xFF) << 16) | ((micro & 0xFF) << 8) | ((relstep & 0xFF) << 8);
}

bool InitializeMenu() {
	Result rc = 0;

	/* Attempt to get the exosphere version. */
	u64 version;
	if (R_FAILED(rc = splGetConfig(static_cast<SplConfigItem>(ExosphereApiVersionConfigItem), &version))) {
		printf("Atmosphere not found. Daybreak requires Atmosphere to be installed. %d", rc);
		return false;
	}

	const u32 version_micro = (version >> 40) & 0xff;
	const u32 version_minor = (version >> 48) & 0xff;
	const u32 version_major = (version >> 56) & 0xff;

	/* Validate the exosphere version. */
	const bool ams_supports_sysupdate_api = EncodeVersion(version_major, version_minor, version_micro) >= EncodeVersion(0, 14, 0);
	if (!ams_supports_sysupdate_api) {
		printf("Outdated Atmosphere version. Daybreak requires Atmosphere 0.14.0 or later. %d", rc);
		return false;
	}

	/* Ensure DayBreak is ran as a NRO. */
	if (envIsNso()) {
		printf("Unsupported Environment. Please launch Daybreak via the Homebrew menu. %d", rc);

		return false;
	}

	/* Attempt to get the supported version. */
   if (R_SUCCEEDED(rc = splGetConfig(static_cast<SplConfigItem>(ExosphereSupportedHosVersion), &version))) {
		g_supported_version = static_cast<u32>(version);
	}

	/* Initialize ams:su. */
	if (R_FAILED(rc = amssuInitialize())) {
		fatalThrow(rc);
	}

	return true;
}

Result GetUpdateInformation() {
	Result rc = 0;
	/* Attempt to get the update information. */
	if (R_FAILED(rc = amssuGetUpdateInformation(&m_update_info, g_update_path))) {
		if (rc == 0x1a405) {
			printf("No update found in folder.\nEnsure your ncas are named correctly!\nResult: 0x%08x\n", rc);
		} else {
			printf("Failed to get update information.\nResult: 0x%08x\n", rc);
		}
		return rc;
	}
	/* Print update information. */
	printf("- Version: %d.%d.%d\n", (m_update_info.version >> 26) & 0x1f, (m_update_info.version >> 20) & 0x1f, (m_update_info.version >> 16) & 0xf);
	if (m_update_info.exfat_supported) {
		printf("- exFAT: Supported\n");
	} else {
		printf("- exFAT: Unsupported\n");
	}
	printf("- Firmware variations: %d\n", m_update_info.num_firmware_variations);
	consoleUpdate(NULL);
	/* Mark as having obtained update info. */
	m_has_info = true;
	return rc;
}

void ValidateUpdate() {
	Result rc = 0;

	/* Validate the update. */
	if (R_FAILED(rc = amssuValidateUpdate(&m_validation_info, g_update_path))) {
		printf("Failed to validate update.\nResult: 0x%08x\n", rc);
		return;
	}

	/* Check the result. */
	if (R_SUCCEEDED(m_validation_info.result)) {
		printf("Update is valid!\n");

		if (R_FAILED(m_validation_info.exfat_result)) {
		   const u32 version = m_validation_info.invalid_key.version;
			printf("exFAT Validation failed with result: 0x%08x\n", m_validation_info.exfat_result);
			printf("Missing content:\n- Program id: %016lx\n- Version: %d.%d.%d\n", m_validation_info.invalid_key.id, (version >> 26) & 0x1f, (version >> 20) & 0x1f, (version >> 16) & 0xf);

			/* Log the missing content id. */
			printf("- Content id: ");
			for (size_t i = 0; i < sizeof(NcmContentId); i++) {
				printf("%02x", m_validation_info.invalid_content_id.c[i]);
			}
			printf("\n");
		}
	} else {
		/* Log the missing content info. */
		const u32 version = m_validation_info.invalid_key.version;
		printf("Validation failed with result: 0x%08x\n", m_validation_info.result);
		printf("Missing content:\n- Program id: %016lx\n- Version: %d.%d.%d\n", m_validation_info.invalid_key.id, (version >> 26) & 0x1f, (version >> 20) & 0x1f, (version >> 16) & 0xf);

		/* Log the missing content id. */
		printf("- Content id: ");
		for (size_t i = 0; i < sizeof(NcmContentId); i++) {
			printf("%02x", m_validation_info.invalid_content_id.c[i]);
		}
		printf("\n");
	}
	/* Mark validation as being complete. */
	m_has_validated = true;
}

void MarkForReboot() {
	m_install_state = InstallState::AwaitingReboot;
}

Result TransitionUpdateState() {
	Result rc = 0;
	if (m_install_state == InstallState::NeedsSetup) {
		/* Setup the update. */
		if (R_FAILED(rc = amssuSetupUpdate(nullptr, UpdateTaskBufferSize, g_update_path, g_use_exfat))) {
			printf("Failed to setup update.\nResult: 0x%08x\n", rc);
			MarkForReboot();
			return rc;
		}

		/* Log setup completion. */
		printf("Update setup complete.\n");
		m_install_state = InstallState::NeedsPrepare;
	} else if (m_install_state == InstallState::NeedsPrepare) {
		/* Request update preparation. */
		if (R_FAILED(rc = amssuRequestPrepareUpdate(&m_prepare_result))) {
			printf("Failed to request update preparation.\nResult: 0x%08x\n", rc);
			MarkForReboot();
			return rc;
		}

		/* Log awaiting prepare. */
		printf("Preparing update...\n");
		m_install_state = InstallState::AwaitingPrepare;
	} else if (m_install_state == InstallState::AwaitingPrepare) {
		/* Check if preparation has a result. */
		if (R_FAILED(rc = asyncResultWait(&m_prepare_result, 0)) && rc != 0xea01) {
			printf("Failed to check update preparation result.\nResult: 0x%08x\n", rc);
			MarkForReboot();
			return rc;
		} else if (R_SUCCEEDED(rc)) {
			if (R_FAILED(rc = asyncResultGet(&m_prepare_result))) {
				printf("Failed to prepare update.\nResult: 0x%08x\n", rc);
				MarkForReboot();
				return rc;
			}
		}

		/* Check if the update has been prepared. */
		bool prepared;
		if (R_FAILED(rc = amssuHasPreparedUpdate(&prepared))) {
			printf("Failed to check if update has been prepared.\nResult: 0x%08x\n", rc);
			MarkForReboot();
			return rc;
		}

		/* Mark for application if preparation complete. */
		if (prepared) {
			printf("\nUpdate preparation complete.\nApplying update...\n");
			m_install_state = InstallState::NeedsApply;
			return rc;
		}

		/* Check update progress. */
		NsSystemUpdateProgress update_progress = {};
		if (R_FAILED(rc = amssuGetPrepareUpdateProgress(&update_progress))) {
			printf("Failed to check update progress.\nResult: 0x%08x\n", rc);
			MarkForReboot();
			return rc;
		}

		ProgressEvent::instance().setTotalSteps(update_progress.total_size);

		// Update progress percent.
		if (static_cast<float>(update_progress.total_size) > 0.0f) {
			ProgressEvent::instance().setStep(update_progress.current_size);
			m_progress_percent = (static_cast<float>(update_progress.current_size) / static_cast<float>(update_progress.total_size)) * 100.0f;
			// printf("\r* Update progress : %3.0f %s *", m_progress_percent, "%");
			// consoleUpdate(NULL);
		} else {
			ProgressEvent::instance().setStep(0);
			m_progress_percent = 0.0f;
			// printf("\r* Update progress : %3.0f %s *", m_progress_percent, "%");
			// consoleUpdate(NULL);
		}
		// printf("\r* %10.0f on %10.0f  *", static_cast<float>(update_progress.current_size), static_cast<float>(update_progress.total_size));
		// printf("\r* %i *", (static_cast<int>(update_progress.current_size) / static_cast<int>(update_progress.total_size)) * 100);
		// printf("\r* Update progress : %3.0f %s *", m_progress_percent, "%");
		// consoleUpdate(NULL);
	} else if (m_install_state == InstallState::NeedsApply) {
		/* Apply the prepared update. */
		if (R_FAILED(rc = amssuApplyPreparedUpdate())) {
			printf("Failed to apply update.\nResult: 0x%08x\n", rc);
		} else {
			/* Log success. */
			printf("Update applied successfully.\n");

			if (g_reset_to_factory) {
				if (R_FAILED(rc = nsResetToFactorySettingsForRefurbishment())) {
					/* Fallback on ResetToFactorySettings. */
					if (rc == MAKERESULT(Module_Libnx, LibnxError_IncompatSysVer)) {
						if (R_FAILED(rc = nsResetToFactorySettings())) {
							printf("Failed to reset to factory settings.\nResult: 0x%08x\n", rc);
							MarkForReboot();
							return rc;
						}
					} else {
						printf("Failed to reset to factory settings for refurbishment.\nResult: 0x%08x\n", rc);
						MarkForReboot();
						return rc;
					}
				}

				printf("Successfully reset to factory settings.%d\n", rc);
			}
		}

		MarkForReboot();
		return rc;
	}

	return rc;
}