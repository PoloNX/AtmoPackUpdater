#include <iostream>
#include <string>
#include <array>
#include <fstream>

#include <json.hpp>
#include <switch.h>

#include "download.hpp"
#include "menu.hpp"

const std::array<std::string, 5> OPTION_LIST{
    "= Update le CFW",
    "= Update l'application",
    "= Update les sigpatches",
    "= Telecharger le dernier firmware",
    "= Raffraichir la page"
};

constexpr std::string_view APP_VER = "0.0.6";
constexpr int CURSOR_LIST_MAX = 4;

menu::menu(){
    getLastFirm();
    getLastPack();
    getCurrentPack();
}

void menu::getCurrentPack(){
    std::fstream txtfile;
    chdir("/");
    txtfile.open("version.txt", std::ios::in);
    if (txtfile.is_open()){
        std::string version;
        getline(txtfile, version);
        if (version[19] == 'v') {
           version.erase(19, 1);
        }
        currentPack = version;
    }
    else{
        currentPack = "impossible de determiner la version de votre pack";
    }
}

void menu::getLastPack(){
    nlohmann::ordered_json json;
    net::getRequest(CFW_API, json);
    std::cout << json;
    if(json.empty()){
        lastPack = "internet non detecte";
        return;
    }
    auto object = json[0];
    std::string name = object.at("tag_name");
    if (name[0] == 'v'){
        name.erase(0, 1);
    }
    lastPack = name;
}

void menu::getLastFirm(){
    nlohmann::ordered_json json;
    net::getRequest(FIR_URL, json);
    if (json.empty()){
        lastFirm = "internet non detecte";
        return;
    }
    auto object = json[0];
    std::string assets = object.at("assets_url");
    net::getRequest(FIR_URL, json);
    if (json.empty()){
        lastFirm = "internet non detecte";
        return;
    }
    object = json[0];
    std::string temp = 
    lastFirm = object.at("name");
}

void menu::refreshScreen(int &cursor){
    consoleClear();

    std::cout << "\033[1;36mAtmoPackUpdater : v" << APP_VER << " by PoloNX" << std::endl << std::endl;

    std::cout << "\033[1;36mDernier firmware en date  :" << "\033[1;31m - " << lastFirm << " -" <<std::endl << std::endl;

    std::cout << "\033[1;36mDerniere version du pack  :" << "\033[1;31m - AtmoPack-Vanilla "<< lastPack << " -" << std::endl;
    std::cout << "\033[1;36mVersion acctuelle du pack : \033[1;31m" << currentPack << std::endl << std::endl;

    std::cout << "\033[1;33mAppuyez sur (A) pour selectionner une option" << std::endl;
    std::cout << "\033[1;33mAppuyez sur (+) pour quiter l'application" << std::endl << std::endl << std::endl << std::endl;

    if (cursor > CURSOR_LIST_MAX)
        cursor = 0;

    else if (cursor < 0)
        cursor = CURSOR_LIST_MAX;

    for (int i = 0; i < CURSOR_LIST_MAX + 1; ++i){
        std::cout << "\033[1;36m"<<'[' << ((cursor == i) ? 'X' : ' ') << ']' << OPTION_LIST[i] << std::endl << std::endl;
    }
}
