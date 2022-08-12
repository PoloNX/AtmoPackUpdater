#pragma once

#include <borealis.hpp>
#include "constants.hpp"

class SettingsTab : public brls::List {
private:
    brls::ListItem* listItem;
    void createList();

public:
    SettingsTab();
};