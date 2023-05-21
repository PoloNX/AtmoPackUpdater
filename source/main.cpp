#include <switch.h>

#include <cstdlib>

#include "main_frame.hpp"
#include "constants.hpp"
#include "utils.hpp"
#include "warning_page.hpp"
#include "confirm_page.hpp"
#include <filesystem>
#include <borealis.hpp>
#include "worker_page.hpp"
#include "download.hpp"
#include <fstream>

namespace i18n = brls::i18n;
using namespace i18n::literals;

int main() {

    if (!brls::Application::init(fmt::format("{}{}", "AtmoPackUpdater v", APP_VER))) {
        brls::Logger::error("Unable to init Borealis application");
        return EXIT_FAILURE;
    }

    const std::string currentLocale = i18n::getCurrentLocale();
    if (currentLocale != "fr" && currentLocale != "en-US" && currentLocale != "es") 
        i18n::loadTranslations("en-US");
    else 
        i18n::loadTranslations();

    setsysInitialize();
    plInitialize(PlServiceType_User);
    nsInitialize();
    socketInitializeDefault();
    nxlinkStdio();
    pmdmntInitialize();
    pminfoInitialize();
    splInitialize();
    romfsInit();

    brls::Logger::setLogLevel(brls::LogLevel::DEBUG);

    if (!std::filesystem::exists("sdmc:/config/AtmoPackUpdater/config.json")) {
        chdir("sdmc:/");
        util::cp("romfs:/config/config.json", "/config/AtmoPackUpdater/config.json");
    }

    nlohmann::ordered_json nxlinks;
    net::getRequest(NXLINKS_URL, nxlinks);
    if (util::is_older_version(APP_VER, nxlinks.at("app")["version"])) {
        std::ifstream file("/config/AtmoPackUpdater/config.json"); //No condition is_open because created just before if it doesn't exist
        nlohmann::json json;
        file >> json;
        file.close();
        if (json.at("auto-update").get<bool>() == true) {
            if(!nxlinks.empty()) {
                std::vector<homebrew_label> homebrews;
                std::vector<std::pair<std::string, std::string>> links = net::getLinksFromJson(util::getValueFromKey(nxlinks, contentTypeNames[(int)contentType::app].data()), contentType::app, homebrews);
                std::string title = links[0].first;
                std::string url = links[0].second;

                brls::StagedAppletFrame* stagedFrame = new brls::StagedAppletFrame();
                stagedFrame->setTitle(fmt::format("{}", "menu/update/auto-update"_i18n));

                stagedFrame->addStage(new WorkerPage(stagedFrame, "menu/update/download"_i18n, [title, url]() {
                    util::downloadArchive(url, contentType::app, false);
                }));
                stagedFrame->addStage(new WorkerPage(stagedFrame, "menu/update/extract_text"_i18n, []() {
                    util::extractArchive(contentType::app);
                }));
                std::string doneMsg = "menu/update/download_finish"_i18n + "\n" + "menu/update/apply_app"_i18n;
                stagedFrame->addStage(new ConfirmPage(stagedFrame, doneMsg, true, false, util::isErista(), true));
                brls::Application::pushView(stagedFrame);
            }
        }
    }

    else if (!util::isApplet()) {
        brls::Application::pushView(new MainFrame(nxlinks));
    } else {
        brls::Application::pushView(new WarningPage("menu/error/applet"_i18n));
    }

    while (brls::Application::mainLoop()) {
        ;
    }

    romfsExit();
    splExit();
    pminfoExit();
    pmdmntExit();
    socketExit();
    nsExit();
    setsysExit();
    plExit();
    return EXIT_SUCCESS;
}


