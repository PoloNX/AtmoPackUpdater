#include "main_frame.hpp"
#include "update_tab.hpp"
#include "about_tab.hpp"
#include "download.hpp"
#include "utils.hpp"
#include "confirm_page.hpp"
#include "settings_tab.hpp"
#include "constants.hpp"
#include <iostream>
#include <fstream>
#include "worker_page.hpp"

namespace i18n = brls::i18n;
using namespace i18n::literals;

MainFrame::MainFrame() : TabFrame() {
    nlohmann::ordered_json json;
    net::getRequest(NXLINKS_URL, json);
    initializeFromJSON(json);
}

MainFrame::MainFrame(nlohmann::ordered_json nxlinks) : TabFrame() {
    initializeFromJSON(nxlinks);
}

void MainFrame::initializeFromJSON(const nlohmann::ordered_json& nxlinks) {
    // Initialiser les membres de MainFrame à partir de l'objet JSON ici
    this->setTitle(fmt::format("{}{}", "AtmoPackUpdater v", APP_VER));
    this->setIcon(BOREALIS_ASSET("icon/icon.png"));
    std::vector<bool> tabsAccepted;
    if(!nxlinks.empty()) {
        for (auto i : nxlinks.at("tab")) {
            tabsAccepted.push_back(i.get<bool>());
        }
        if (util::is_older_version(APP_VER, nxlinks)) {
            brls::Application::setCommonFooter("menu/footer/update_available"_i18n);
        }
    }
    else {
        for (auto i = 0; i < 5; ++i) {
            tabsAccepted.push_back(true);
        }
        brls::Application::setCommonFooter("menu/footer/no_internet"_i18n);
    }
    
    if (tabsAccepted[0])
        this->addTab("menu/tab/pack"_i18n, new UpdateTab(contentType::ams_cfw, nxlinks));
    if (tabsAccepted[1])
        this->addTab("menu/tab/app"_i18n, new UpdateTab(contentType::app, nxlinks));
    if (tabsAccepted[2])
        this->addTab("menu/tab/homebrew"_i18n, new UpdateTab(contentType::homebrew, nxlinks));
    if (tabsAccepted[3])
        this->addTab("menu/tab/firmwares"_i18n, new UpdateTab(contentType::firmwares, nxlinks));
    if (tabsAccepted[4])
        this->addTab("menu/tab/sigpatches"_i18n, new UpdateTab(contentType::sigpatches, nxlinks));

    this->addSeparator();

    this->addTab("menu/tab/settings"_i18n, new SettingsTab());

    this->addTab("menu/tab/credits"_i18n, new CreditsTab());

    this->registerAction("", brls::Key::B, [this] { return true; });
}
