#ifndef _DOWNLOAD_H_
#define _DOWNLOAD_H_

#define APP_URL         "https://github.com/PoloNX/AtmoPackUpdater/releases/latest/download/AtmoPackUpdater.nro"
#define CFW_URL         "https://github.com/THZoria/AtmoPack-Vanilla/releases/latest/download/AtmoPack-Vanilla_Latest.zip"
#define SIG_URL         "https://github.com/ITotalJustice/patches/releases/latest/download/SigPatches.zip"
//#define FIR_URL         "https://github.com/THZoria/NX_Firmware/releases/latest/download/Firmware.zip"

#define TEMP_FILE       "/switch/temp.zip"
#define TEMP_FILE_HB    "/switch/temp.nro"


#define ON              1
#define OFF             0


#include <stdbool.h>

//
bool downloadFile(const char *url, const char *output, int api);

#endif
