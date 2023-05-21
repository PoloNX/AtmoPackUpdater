#pragma once

#include <borealis.hpp>
#include <json.hpp>

class MainFrame : public brls::TabFrame {
public:
    MainFrame(nlohmann::ordered_json nxlinks);
    MainFrame();
    void initializeFromJSON(const nlohmann::ordered_json& json);
};