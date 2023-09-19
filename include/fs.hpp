#pragma once

#include <switch.h>

#include <json.hpp>
#include <set>

namespace fs {
    bool removeDir(const std::string& path);
    bool removeFile(const std::string& path);
    nlohmann::ordered_json parseJsonFile(const std::string& path);
    void writeJsonToFile(nlohmann::json& data, const std::string& path);
    bool copyFile(const std::string& from, const std::string& to);
    std::string copyFiles(const std::string& path);
    void createTree(std::string path);
    std::set<std::string> readLineByLine(const std::string& path);
    Result getFreeStorageSD(s64& free);
    std::vector<std::string> splitString(const std::string& s, char delimiter);
}  // namespace fs