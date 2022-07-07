#include <curl/curl.h>
#include <string>

#include <json.hpp>
#include <switch.h>

const std::string CFW_URL = "https://github.com/THZoria/AtmoPack-Vanilla/releases/latest/download/AtmoPack-Vanilla_Latest.zip";

const std::string TEMP_FILE = "/switch/temp.zip";

namespace net{
    bool downloadFile(const std::string &url, const std::string &output, const bool api);
    long getRequest(const std::string &url, nlohmann::ordered_json& res, const std::vector<std::string>& headers = {}, const std::string& body = "");
}