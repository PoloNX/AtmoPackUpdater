#include <curl/curl.h>
#include <string>

#include <json.hpp>
#include <switch.h>

const std::string CFW_URL = "https://github.com/THZoria/AtmoPack-Vanilla/releases/latest/download/AtmoPack-Vanilla_Latest.zip";
const std::string CFW_API = "https://api.github.com/repos/THZoria/AtmoPack-Vanilla/releases";
const std::string APP_URL = "https://github.com/PoloNX/AtmoPackUpdater/releases/latest/download/AtmoPackUpdater.nro";
const std::string SIG_URL = "aHR0cHM6Ly9naXRodWIuY29tL1BIUmV0cm9HYW1lcnMvc2lnbmF0dXJlX2dwZC9yZWxlYXNlcy9sYXRlc3QvZG93bmxvYWQvc2lnbmF0dXJlX2dwZC56aXA=";
const std::string FIR_URL = "https://api.github.com/repos/THZoria/NX_Firmware/releases";

const std::string TEMP_FILE =     "/switch/temp.zip";
const std::string TEMP_FILE_HB =  "/switch/temp.zip";
const std::string APP_OUTPUT =    "/switch/AtmoPackUpdater.nro";

namespace net{
    bool downloadFile(const std::string &url, const std::string &output, const bool api);
    long getRequest(const std::string &url, nlohmann::ordered_json& res, const std::vector<std::string>& headers = {}, const std::string& body = "");
}
