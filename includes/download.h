#ifndef _DOWNLOAD_H_
#define _DOWNLOAD_H_

#define APP_URL         "https://github.com/PoloNX/AtmoPackUpdater/releases/latest/download/AtmoPackUpdater.nro"
#define CFW_URL         "https://github.com/THZoria/AtmoPack-Vanilla/releases/latest/download/AtmoPack-Vanilla_Latest.zip"
#define SIG_URL         "https://github.com/ITotalJustice/patches/releases/latest/download/SigPatches.zip"
#define FIR_URL         "https://api.github.com/repos/THZoria/NX_Firmware/releases"

#define TEMP_FILE       "/switch/temp.zip"
#define TEMP_FIRM       "/switch/temp.zip"
#define TEMP_FILE_HB    "/switch/temp.nro"


#define ON              1
#define OFF             0


#include <stdbool.h>
#include "json.hpp"

//
bool downloadFile(const char *url, const char *output, int api);

long getRequest(const std::string& url, nlohmann::ordered_json& res, const std::vector<std::string>& headers = {}, const std::string& body = "");
std::vector<std::pair<std::string, std::string>> getLinks(const std::string& url);
std::vector<std::pair<std::string, std::string>> getLinksFromJson(const nlohmann::ordered_json& json_object);

#endif
