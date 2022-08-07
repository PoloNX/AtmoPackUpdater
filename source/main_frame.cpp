#include "main_frame.hpp"
#include "update_tab.hpp"
#include "about_tab.hpp"
#include "download.hpp"

namespace i18n = brls::i18n;
using namespace i18n::literals;

MainFrame::MainFrame() : TabFrame() {
    // Create a tabbed frame with a single tab

    nlohmann::ordered_json nxlinks;
    net::getRequest(NXLINKS_URL, nxlinks);

    this->setTitle("AtmoPackUpdater");

    this->addTab("Update le pack", new UpdateTab(contentType::ams_cfw, nxlinks));

    this->addTab("Update l'app", new UpdateTab(contentType::app, nxlinks));

    this->addTab("Update le firmware", new UpdateTab(contentType::firmwares, nxlinks));

    this->addTab("Update les sigpatches", new UpdateTab(contentType::sigpatches, nxlinks));

    this->addTab("Crédits", new CreditsTab());

    this->registerAction("", brls::Key::B, [this] { return true; });
}