#include "update_tab.hpp"
#include "download.hpp"
#include "confirm_page.hpp"
#include "worker_page.hpp"
#include "utils.hpp"
#include "reboot.hpp"
#include "dialogue_page.hpp"

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
    

    std::vector<std::pair<std::string, std::string>> links = net::getLinksFromJson(getValueFromKey(this->nxlinks, contentTypeNames[(int)type].data()));;

    switch(type) {
        case contentType::ams_cfw: {
            std::string currentVersion = util::getPackVersion();
            createSubTitle(subTitle, "Mettre à jour AtmoPack-Vanilla");
            description->setText(fmt::format("{}{}{}{}", "menu/description/pack"_i18n, currentVersion, "menu/description/version"_i18n, links[1].second));
            break;
        }
        case contentType::app: {
            createSubTitle(subTitle, "Mettre à jour l'Application");
            description->setText(fmt::format("{}{}{}{}", "menu/description/app"_i18n, APP_VER, "menu/description/version"_i18n, links[1].second));
            break;
        }
        case contentType::firmwares: {
            createSubTitle(subTitle, "Mettre à jour le Firmware");
            SetSysFirmwareVersion ver;
            description->setText(fmt::format("{}{}", "menu/description/firmwares"_i18n, R_SUCCEEDED(setsysGetFirmwareVersion(&ver)) ? ver.display_version : "Impossible de déterminer la version actuelle"));
            break;
        }
        case contentType::sigpatches: {
            createSubTitle(subTitle, "Mettre à jour les Sigpacthes");
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
            if (title == "version"){
                continue;
            }
            const std::string url = link.second;
            const std::string text("Téléchargement de " + title + " Lien compet:\n " + url);
            //Create one button with the name of the release
            listItem = new brls::ListItem(link.first);
            listItem->setHeight(50);
            //Get Click Event
            listItem->getClickEvent()->subscribe([this, type, text, url, title](brls::View* view) {
                //Create a Staged Applet Frame
                brls::StagedAppletFrame* stagedFrame = new brls::StagedAppletFrame();
                stagedFrame->setTitle(fmt::format("Téléchargement de {}", contentTypeNames[(int)type].data()));
                //Create a Confirm Page
                stagedFrame->addStage(new ConfirmPage(stagedFrame, text, true));
                //Create a Download Page
                stagedFrame->addStage(new WorkerPage(stagedFrame, "Téléchargement...", [this, type, url]() {util::downloadArchive(url, type); }));
                //Create an extract Page
                if (type != contentType::app){
                    stagedFrame->addStage(new WorkerPage(stagedFrame, "Extraction en cours..", [this, type]() {
                        util::extractArchive(type);
                    }));
                }

                //Done messages
                std::string doneMsg = "Téléchargement terminé !";
                switch(type) {
                    case contentType::firmwares: {
                        std::string contentsPath = util::getContentsPath();
                        for (const auto& tid : {"0100000000001000", "0100000000001007", "0100000000001013"}) {
                            if (std::filesystem::exists(contentsPath + tid) && !std::filesystem::is_empty(contentsPath + tid)) {
                                doneMsg += "\nIl semblerait que vous avez installé un thème personnalisé, cela pourrait empêcher le système de démarrer après la mise à jour de votre firmware.\nIl est conseillé de le supprimer avant d'effectuer la mise à jour.";
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
                        doneMsg += "\nVotre console dois redémarrer pour appliquer les patchs de signature.";
                        stagedFrame->addStage(new ConfirmPage(stagedFrame, doneMsg, true, true, util::isErista()));
                        break;
                    case contentType::ams_cfw:
                        doneMsg += "\nVotre console dois redémarrer pour appliquer les patchs de signature.";
                        stagedFrame->addStage(new ConfirmPage(stagedFrame, doneMsg, true, true, util::isErista()));
                        break;    
                    case contentType::app:
                        doneMsg += "\nL'hombrew doit redémarrer.";
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
        "Aucun résultat",
        true);
    notFound->setHorizontalAlign(NVG_ALIGN_CENTER);
    this->addView(notFound);
}