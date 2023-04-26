#pragma once

constexpr int ON = 1;
constexpr int OFF = 0;

#include "constants.hpp"
#include <json.hpp>

namespace net {

    long downloadFile(const std::string& url, std::vector<std::uint8_t>& res, const std::string& output = "", int api = OFF);
    long downloadFile(const std::string& url, const std::string& output = "", int api = OFF);
    std::vector<std::pair<std::string, std::string>> getLinks(const std::string& url);
    std::vector<std::pair<std::string, std::string>> getLinksFromJson(const nlohmann::ordered_json& json_object, contentType type, std::vector<homebrew_label>& homebrews);
        std::vector<std::pair<std::string, std::string>> getLinksFromJson(const nlohmann::ordered_json& json_object, contentType type);
    std::string fetchTitle(const std::string& url);
    long downloadPage(const std::string& url, std::string& res, const std::vector<std::string>& headers = {}, const std::string& body = "");
    long getRequest(const std::string& url, nlohmann::ordered_json& res, const std::vector<std::string>& headers = {}, const std::string& body = "");

}  // namespace net