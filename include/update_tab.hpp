#pragma once

#include <borealis.hpp>
#include <json.hpp>
#include "constants.hpp"

class UpdateTab : public brls::List {
private:
    brls::ListItem* listItem;
    nlohmann::ordered_json nxlinks;
    contentType type;
    void createList();
    void createList(contentType type);
    void setDescription();
    void setDescription(contentType type);
    void displayNotFound();

public:
    UpdateTab(const contentType type, const nlohmann::ordered_json& nxlinks = nlohmann::ordered_json::object());
};