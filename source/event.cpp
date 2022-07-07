#include <switch.h>
#include <json.hpp>

#include "event.hpp"
#include "menu.hpp"
#include "download.hpp"
#include "unzip.hpp"

namespace event{
    void checkInput(PadState &pad, int &cursor, bool &isOpen){
        padUpdate(&pad);
        u64 kDown = padGetButtonsDown(&pad);

        if (kDown & HidNpadButton_Down){
            ++cursor;
            menu::refreshScreen(cursor);
        }

        if (kDown & HidNpadButton_Up){
            --cursor;
            menu::refreshScreen(cursor);
        }

        if (kDown & HidNpadButton_A){
            switch(cursor){
                case UP_CFW:
                    if(net::downloadFile(CFW_URL, TEMP_FILE, false)){
                        extract::unzip(TEMP_FILE, "/");
                        remove(APP_OUTPUT.c_str());
                        rename(TEMP_FILE_HB.c_str(), APP_OUTPUT.c_str());
                        menu::refreshScreen(cursor);
                        std::cout << "\n\nTelechargement du pack termine. redemarrage de la console dans 3 secondes..." << std::endl;
                        consoleUpdate(NULL);
                        sleep(3);
                        isOpen = false;
                    }
                    
                    break;

                case UP_APP:
                    if(net::downloadFile(APP_URL, TEMP_FILE_HB, false)){
                        remove(APP_OUTPUT.c_str());
                        rename(TEMP_FILE_HB.c_str(), APP_OUTPUT.c_str());
                        menu::refreshScreen(cursor);
                        std::cout << "\n\nTelechargement de l'app termine. redemarrage de l'app dans 3 secondes..." << std::endl;
                        consoleUpdate(NULL);
                        sleep(3);
                        isOpen = false;
                    }
                    break;

                case UP_SIG:
                    if (net::downloadFile(SIG_URL, TEMP_FILE, false)){
                        extract::unzip(TEMP_FILE, "/");
                        menu::refreshScreen(cursor);
                        std::cout << "\n\nTelechargement des sigpatches temine. redemarrage de la console dans 3 secondes..." << std::endl;
                        consoleUpdate(NULL);
                        sleep(3);
                        isOpen = false;
                    }
                    break;

                case UP_FIR:
                    nlohmann::ordered_json json;
                    net::getRequest(FIR_URL, json);
                    if (json.empty()){
                        std::cout << "\n\nUne erreur de connexion est survenue." << std::endl;
                    }
                    else{
                        auto object = json[0];
                        std::string assets = object.at("assets_url");
                        json.clear();
                        net::getRequest(assets, json);
                        object = json[0];
                        std::string url = object.at("browser_download_url");
                        if (net::downloadFile(url, TEMP_FILE, false)){
                            extract::unzip(TEMP_FILE, "/");
                            menu::refreshScreen(cursor);
                            std::cout << "\n\nTelechargement du dernier firmware temine! Veuillez l'installer avec DayBreak." << std::endl;
                            consoleUpdate(NULL);
                        }
                        else{
                            menu::refreshScreen(cursor);
                            std::cout << "\n\nUne erreur de connexion est survenue." << std::endl;
                            consoleUpdate(NULL);
                            sleep(5);
                            menu::refreshScreen(cursor);
                        }
                    }
                    break;
            }
        }

        if (kDown & HidNpadButton_Plus){
            isOpen = false;
        }   
    }
}