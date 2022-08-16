#include "main_frame.hpp"
#include "update_tab.hpp"
#include "about_tab.hpp"
#include "download.hpp"
#include "utils.hpp"
#include "settings_tab.hpp"
#include <iostream>

namespace i18n = brls::i18n;
using namespace i18n::literals;

MainFrame::MainFrame() : TabFrame() {
    // Create a tabbed frame with a single tab
    this->setTitle(fmt::format("{}{}", "AtmoPackUpdater v", APP_VER));
    this->setIcon(BOREALIS_ASSET("icon/icon.png"));

    nlohmann::ordered_json nxlinks;
    net::getRequest(NXLINKS_URL, nxlinks);

    this->addTab("menu/tab/pack"_i18n, new UpdateTab(contentType::ams_cfw, nxlinks));

    this->addTab("menu/tab/app"_i18n, new UpdateTab(contentType::app, nxlinks));

    this->addTab("menu/tab/firmwares"_i18n, new UpdateTab(contentType::firmwares, nxlinks));

    this->addTab("menu/tab/sigpatches"_i18n, new UpdateTab(contentType::sigpatches, nxlinks));

    this->addSeparator();

    this->addTab("menu/tab/settings"_i18n, new SettingsTab());

    this->addTab("menu/tab/credits"_i18n, new CreditsTab());

    this->registerAction("", brls::Key::B, [this] { return true; });
}