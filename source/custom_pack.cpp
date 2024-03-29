#include "custom_pack.hpp"
#include "utils.hpp"
#include "worker_page.hpp"
#include "confirm_page.hpp"

#include <iostream>
#include <fstream>

namespace i18n = brls::i18n;
using namespace i18n::literals;

std::vector<Pack> CustomPack::getPacks() {
    std::vector<Pack> packs;
    std::ifstream file("/config/AtmoPackUpdater/config.json");
    if(file.is_open()) {
        nlohmann::json j = util::getConfig();
    
        if(j.find("custom-pack") != j.end() && j["custom-pack"].is_array()) {
            for(auto& element : j["custom-pack"]) {
                if (element.find("name") != element.end() && element.find("url") != element.end()) {
                    std::string name = element["name"];
                    std::string url = element["url"];
                    packs.push_back(Pack(name, url));
                    brls::Logger::info("Added pack: " + name);
                }
                else {
                    brls::Logger::error("Invalid Pack Format");
                    continue;
                }
            }
        }
        else {
            brls::Logger::error("No custom-pack array");
            j["custom-pack"] = nlohmann::json::array();
        }

        std::cout << j.dump(4) << std::endl;
    
        util::setConfig(j);
    }
    else {
        chdir("sdmc:/");
        util::cp("romfs:/config/config.json", "/config/AtmoPackUpdater/config.json");
    }
    

    return packs;
}

void CustomPack::addPack(Pack& pack) {
    nlohmann::json j;
    std::ifstream file("/config/AtmoPackUpdater/config.json");
    if(file.is_open()) {
        file >> j;
        file.close();
        if (j.find("custom-pack") != j.end() && j["custom-pack"].is_array()) {
            nlohmann::json new_pack;
            new_pack["name"] = pack.getName();
            new_pack["url"] = pack.getUrl();
            j["custom-pack"].push_back(new_pack);

            std::ofstream outFile("/config/AtmoPackUpdater/config.json");
            if(outFile.is_open()) {
                outFile << j.dump(4);
                outFile.close();
                brls::Logger::info("Added pack: " + pack.getName());
            } else {
                brls::Logger::error("Failed to open config file for writing");
            }
        } else {
            brls::Logger::error("Invalid or missing 'custom-pack' field in the configuration file");
        }
    } else {
        brls::Logger::error("Failed to open config file for reading");
    }
}

void CustomPack::deletePack(Pack pack) {
    brls::Logger::debug("Deleting pack: {}", pack.getName());
    nlohmann::json j;
    std::ifstream file("/config/AtmoPackUpdater/config.json");
    if(file.is_open()) {
        brls::Logger::debug("Opened config file for reading");
        file >> j;
        file.close();
        brls::Logger::debug("Closed config file for reading");
        if(j.find("custom-pack") != j.end() && j["custom-pack"].is_array()) {
            brls::Logger::info("Deleting pack: " + pack.getName());
            for (auto it = j["custom-pack"].begin(); it != j["custom-pack"].end(); ++it) {
                if ((*it)["name"] == pack.getName() && (*it)["url"] == pack.getUrl()) {
                    j["custom-pack"].erase(it);
                    break;
                }
            }
            std::ofstream outfile("/config/AtmoPackUpdater/config.json");
            outfile << j;
            outfile.close();
        } else {
            brls::Logger::error("Invalid or missing 'custom-pack' field in the configuration file");
        }
    } else {
        brls::Logger::error("Failed to open config file for reading");
    }
}

CustomPack::CustomPack() {

    m_label = new brls::Label(brls::LabelStyle::REGULAR, "", true);
    m_label->setText("menu/update/subtitle_custom_pack"_i18n);
    m_label->setHorizontalAlign(NVG_ALIGN_CENTER);
    this->addView(m_label);

    m_label = new brls::Label(brls::LabelStyle::DESCRIPTION, "menu/description/custom-pack"_i18n, true);
    this->addView(m_label);

    Packs = getPacks();

    for(auto pack : Packs) {
        m_item = new brls::ListItem(pack.getName(), "", pack.getUrl());
        m_item->getClickEvent()->subscribe([this, pack](brls::View* view){
            
            brls::Logger::debug("Downloading pack: {}", pack.getName());
            brls::StagedAppletFrame* stagedFrame = new brls::StagedAppletFrame();
            brls::Logger::debug("Created stagged frame");

            stagedFrame->setTitle(fmt::format("{}{}", "menu/update/download_text"_i18n, pack.getName()));

            const std::string text("menu/update/download_text"_i18n + "\n" + pack.getName() + "\n" + "menu/update/link_text"_i18n + pack.getUrl());

            stagedFrame->addStage(new ConfirmPage(stagedFrame, text, true));

            stagedFrame->addStage(new WorkerPage(stagedFrame, "menu/update/download"_i18n, [pack]() {
                long status_code;
                util::downloadArchive(pack.getUrl(), contentType::ams_cfw, status_code, false);
            }));

            stagedFrame->addStage(new WorkerPage(stagedFrame, "menu/update/extract_text"_i18n, []() {
                util::extractArchive(contentType::ams_cfw);
            }));

            std::string doneMsg = "menu/update/download_finish"_i18n;
            doneMsg += ("\n" + "menu/update/apply_cfw"_i18n);
            stagedFrame->addStage(new ConfirmPage(stagedFrame, doneMsg, true, true, util::isErista()));  

            brls::Application::pushView(stagedFrame);
        });
        m_item->registerAction("menu/custom-pack/delete"_i18n, brls::Key::Y, [this, pack]{
            brls::Logger::debug("Before deletePack");
            deletePack(pack);
            util::restartApp();
            return true;
        });
        this->addView(m_item);
    }

    m_item = new brls::ListItem("menu/custom_pack/add"_i18n);
    m_item->getClickEvent()->subscribe([&](brls::View* view){
        std::string title, url;
        brls::Swkbd::openForText([&title](std::string text) { title = text; }, "menu/custom_pack/add_title"_i18n, "", 256, "", 0, "menu/custom_pack/submit"_i18n, "menu/custom_pack/title"_i18n);
        brls::Swkbd::openForText([&url](std::string text) { url = text; }, "menu/custom_pack/add_link"_i18n, "", 256, "", 0, "menu/custom_pack/submit"_i18n, "menu/custom_pack/url"_i18n);
        
        Pack pack(title, url);
        addPack(pack);
        util::restartApp();
    });
    this->addView(m_item);
}