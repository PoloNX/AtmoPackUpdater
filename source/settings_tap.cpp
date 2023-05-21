#include "settings_tab.hpp"
#include "utils.hpp"
#include "reboot.hpp"

#include <string>
#include <filesystem>
#include <fstream>

namespace i18n = brls::i18n;
using namespace i18n::literals;

constexpr unsigned int str2int(const char* str, int h = 0)
{
    return !str[h] ? 5381 : (str2int(str, h+1) * 33) ^ str[h];
}

SettingsTab::SettingsTab() : brls::List(){
    this->createList();
}

void SettingsTab::createList() {
    for (auto name : contentSettings) {
        std::string title;

        switch(str2int(name.c_str())) {
            case str2int("dns"): {
                title = "menu/settings/90dns"_i18n;
                listItem = new brls::ListItem(title);
                listItem->setHeight(50);

                listItem->getClickEvent()->subscribe([](brls::View* view) {
                    if (util::set90dns()){
                        brls::Application::notify("menu/settings/90dns_set"_i18n);
                    }
                    else {
                        brls::Application::notify("menu/settings/90dns_set_fail"_i18n);
                    }
                }); 
                this->addView(listItem);
                break;
            }

            case str2int("theme"): {
                title = "menu/settings/theme"_i18n;
                listItem = new brls::ListItem(title);
                listItem->setHeight(50);

                listItem->getClickEvent()->subscribe([](brls::View* view) {
                    if (util::deleteTheme()){
                        brls::Application::notify("menu/settings/theme_delete"_i18n);
                    }
                    else {
                        brls::Application::notify("menu/settings/theme_delete_fail"_i18n);
                    }
                }); 
                this->addView(listItem);
                break;
            }

            case str2int("clear"): {
                title = "menu/settings/clear"_i18n;
                listItem = new brls::ListItem(title);
                listItem->setHeight(50);

                listItem->getClickEvent()->subscribe([](brls::View* view) {
                    util::extractArchive(contentType::app);
                    brls::Application::notify("menu/settings/clear_fail"_i18n);
                });
                this->addView(listItem);
                break;
            }

            case str2int("reboot"): {
                title = "menu/settings/reboot"_i18n;
                listItem = new brls::ListItem(title);
                listItem->setHeight(50);

                listItem->getClickEvent()->subscribe([](brls::View* view) {
                    if(util::isErista()) {
                        util::showDialogBox("menu/dialog/reboot"_i18n, "menu/dialog/yes"_i18n, "menu/dialog/no"_i18n);
                    }
                    else
                        brls::Application::notify("menu/settings/reboot_mariko"_i18n);
                });
                this->addView(listItem);
                break;
            }

            case str2int("auto-update"): {
                title = "menu/settings/auto-update"_i18n;
                brls::SelectListItem* selectItem = new brls::SelectListItem(
                    title,
                    {
                        "menu/dialogue/yes"_i18n,
                        "menu/dialogue/no"_i18n,
                    }
                );
                selectItem->setHeight(50);

                nlohmann::json json;
                std::ifstream file("/config/AtmoPackUpdater/config.json");
                file >> json;
                file.close();

                bool autoUpdateValue = json["auto-update"].get<bool>();

                // Définir l'index sélectionné en fonction de la valeur de "auto-update"
                if (autoUpdateValue) {
                    selectItem->setSelectedValue(0); // Index 0 pour "menu/dialogue/yes"
                } else {
                    selectItem->setSelectedValue(1); // Index 1 pour "menu/dialogue/no"
                }
                
                selectItem->getValueSelectedEvent()->subscribe([=](size_t selection) {
                    // Mettre à jour la valeur dans l'objet JSON en fonction de la sélection
                    nlohmann::json config;
                    std::ifstream file("/config/AtmoPackUpdater/config.json");
                    file>>config;
                    file.close();
                    if (selection==0) {
                        config["auto-update"] = true;
                    }
                    else {
                        config["auto-update"] = false;
                    }
                    std::ofstream outFile("/config/AtmoPackUpdater/config.json");
                    outFile << config.dump(4); // Écriture formatée avec indentation de 4 espaces
                    outFile.close();
                });
                this->addView(selectItem);
                break;
            }
        }
    }

}
