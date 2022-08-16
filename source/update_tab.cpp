#include "update_tab.hpp"
#include "download.hpp"
#include "confirm_page.hpp"
#include "worker_page.hpp"
#include "utils.hpp"
#include "reboot.hpp"
#include "dialogue_page.hpp"

#include <algorithm>
#include <iostream>
#include <string>
#include <filesystem>

namespace i18n = brls::i18n;
using namespace i18n::literals;

UpdateTab::UpdateTab(contentType type, const nlohmann::ordered_json& nxlinks) : brls::List(), type(type), nxlinks(nxlinks) {
    this->setDescription();
    this->createList();
}

const nlohmann::ordered_json getValueFromKey(const nlohmann::ordered_json& jsonFile, const std::string& key)
{
    return (jsonFile.find(key) != jsonFile.end()) ? jsonFile.at(key) : nlohmann::ordered_json::object();
}

void UpdateTab::setDescription() {
    setDescription(this->type);
}

void createSubTitle(brls::Label* &subtitle, const std::string& text) {
    subtitle->setText(text);
    subtitle->setHorizontalAlign(NVG_ALIGN_CENTER);
}


void UpdateTab::setDescription(contentType type) {
    brls::Label* description = new brls::Label(
        brls::LabelStyle::DESCRIPTION,
        "",
        true
    );

    brls::Label* subTitle = new brls::Label(
        brls::LabelStyle::REGULAR,
        "",
        true
    );

    std::vector<std::pair<std::string, std::string>> links = net::getLinksFromJson(getValueFromKey(this->nxlinks, contentTypeNames[(int)type].data()));
    
    switch(type) {
        case contentType::ams_cfw: {
            std::string currentVersion = util::getPackVersion();
            createSubTitle(subTitle, "menu/update/subtitle_ams"_i18n);
            description->setText(fmt::format("{}{}{}{}", "menu/description/pack"_i18n, currentVersion, "menu/description/version"_i18n, links.size() ? links[1].second : "menu/error/version_not_found"_i18n));
            break;
        }
        case contentType::app: {
            createSubTitle(subTitle, "menu/update/subtitle_app"_i18n);
            description->setText(fmt::format("{}{}{}{}", "menu/description/app"_i18n, APP_VER, "menu/description/version"_i18n, links.size() ? links[1].second : "menu/error/version_not_found"_i18n));
            break;
        }
        case contentType::firmwares: {
            createSubTitle(subTitle, "menu/update/subtitle_firmwares"_i18n);
            SetSysFirmwareVersion ver;
            description->setText(fmt::format("{}{}", "menu/description/firmwares"_i18n, R_SUCCEEDED(setsysGetFirmwareVersion(&ver)) ? ver.display_version : "Impossible de déterminer la version actuelle"));
            break;
        }
        case contentType::sigpatches: {
            createSubTitle(subTitle, "menu/update/subtitle_sigpatches"_i18n);
            description->setText("menu/description/sigpatches"_i18n);
            break;
        }
    }

    this->addView(subTitle);
    this->addView(description);
}

void UpdateTab::createList() {
    createList(this->type);
}

void UpdateTab::createList(contentType type) {
    //Create a vector wich contain all the links
    std::vector<std::pair<std::string, std::string>> links = net::getLinksFromJson(getValueFromKey(this->nxlinks, contentTypeNames[(int)type].data()));;

    if (links.size()) {
        for (const auto& link : links) {
            //Create some strings from json
            std::string title = link.first;
            if (title == "version") {
                continue;
            }
            const std::string url = link.second;
            const std::string text("menu/update/download_text"_i18n + "\n" +title + "\n" + "menu/update/link_text"_i18n + url);
            //Create one button with the name of the release
            listItem = new brls::ListItem(link.first);
            listItem->setHeight(50);
            //Get Click Event
            listItem->getClickEvent()->subscribe([this, type, text, url, title](brls::View* view) {
                //Create a Staged Applet Frame
                brls::StagedAppletFrame* stagedFrame = new brls::StagedAppletFrame();
                //Set a title
                stagedFrame->setTitle(fmt::format("{}{}","menu/update/download_text"_i18n, contentTypeNames[(int)type].data()));
                //Create a Confirm Page
                stagedFrame->addStage(new ConfirmPage(stagedFrame, text, true));
                //Create a Download Page
                stagedFrame->addStage(new WorkerPage(stagedFrame, "menu/update/download"_i18n, [this, type, url]() {util::downloadArchive(url, type); }));
                //Create an extract Page
                if (type != contentType::app) {
                    stagedFrame->addStage(new WorkerPage(stagedFrame, "menu/update/extract_text"_i18n, [this, type]() {
                        util::extractArchive(type);
                    }));
                }

                //Done messages
                std::string doneMsg = "menu/update/download_finish"_i18n;
                switch(type) {
                    case contentType::firmwares: {
                        std::string contentsPath = util::getContentsPath();
                        for (const auto& tid : {"0100000000001000/romfs/lyt", "0100000000001007/romfs/lyt", "0100000000001013/romfs/lyt"}) {
                            if (std::filesystem::exists(contentsPath + tid) && !std::filesystem::is_empty(contentsPath + tid)) {
                                stagedFrame->addStage(new DialoguePage_theme(stagedFrame, (doneMsg + "menu/update/theme_installed"_i18n)));
                                break;
                            }
                        }
                        if (std::filesystem::exists(DAYBREAK_PATH)) {
                            stagedFrame->addStage(new DialoguePage_fw(stagedFrame, doneMsg));
                        }
                        else {
                            stagedFrame->addStage(new ConfirmPage(stagedFrame, doneMsg, true));
                        }
                        break;
                    }
                    case contentType::sigpatches:
                        doneMsg += ("\n" + "menu/update/apply_patch"_i18n);
                        stagedFrame->addStage(new ConfirmPage(stagedFrame, doneMsg, true, true, util::isErista()));
                        break;
                    case contentType::ams_cfw:
                        doneMsg += ("\n" + "menu/update/apply_cfw"_i18n);
                        stagedFrame->addStage(new ConfirmPage(stagedFrame, doneMsg, true, true, util::isErista()));
                        break;    
                    case contentType::app:
                        doneMsg += ("\n" + "menu/update/apply_app"_i18n);
                        stagedFrame->addStage(new ConfirmPage(stagedFrame, doneMsg, true, false, util::isErista(), true));
                        break;
                }
                brls::Application::pushView(stagedFrame);
            });

            this->addView(listItem);
        }
    }
    else {
        this->displayNotFound();
    }
}

void UpdateTab::displayNotFound()
{
    brls::Label* notFound = new brls::Label(
        brls::LabelStyle::SMALL,
        "menu/error/links_not_found"_i18n,
        true);
    notFound->setHorizontalAlign(NVG_ALIGN_CENTER);
    this->addView(notFound);
}