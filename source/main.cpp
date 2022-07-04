#include <stdio.h>
#include <stdarg.h>
#include <unistd.h> // chdir
#include <dirent.h> // mkdir
#include <switch.h>
#include <iostream>

#include "download.h"
#include "unzip.h"
#include "reboot.h"
#include "json.hpp"

#define ROOT                    "/"
#define APP_PATH                "/switch/"
#define APP_OUTPUT              "/switch/AtmoPackUpdater.nro"

#define APP_VERSION             "0.0.3"
#define CURSOR_LIST_MAX         3

std::string getFirmwareName(){
    nlohmann::ordered_json json;
    
    getRequest(FIR_URL, json);
    //return "internet non détecté";
    if (json.empty()){
        return "internet non detecte";
    }
    auto object = json[0];
    std::string assets = object.at("assets_url");

    getRequest(assets, json);

    object = json[0];

    return object.at("name");
}

const char *OPTION_LIST[] =
{
    "= Update le CFW",
    "= Update l'application",
    "= Update les sigpatches",
    "= Telecharger le dernier firmware"
};

void printDisplay(const char *text, ...)
{
    va_list v;
    va_start(v, text);
    vfprintf(stdout, text, v);
    va_end(v);
    consoleUpdate(NULL);
}

void refreshScreen(int cursor, std::string name)
{
    consoleClear();

    printf("\x1B[36mAtmoPackUpdater: v%s", APP_VERSION);
    printf(" by ItolalJustice and edited by PoloNX\n\n");
    printf("Dernier firmware en date : ");
    printf("\033[0;31m%s", name.c_str());
    printf("\033[0;33m\n\nAppuyez sur (A) pour selectionner une option\n\n");
    printf("Appuyez sur (+) pour quitter l'application\033[0;37m\n\n\n");

    for (int i = 0; i < CURSOR_LIST_MAX + 1; i++)
        printf("[%c] %s\n\n", cursor == i ? 'X' : ' ', OPTION_LIST[i]);

    consoleUpdate(NULL);
}

int appInit()
{
    consoleInit(NULL);
    socketInitializeDefault();
    nxlinkStdio();
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);
    romfsInit();    //Init of romfs

    return 0;
}

void appExit()
{
    socketExit();
    consoleExit(NULL);
}

int main(int argc, char **argv)
{
    // init stuff
    appInit();
    PadState pad;
    padInitializeDefault(&pad);
    mkdir(APP_PATH, 0777);

    // change directory to root (defaults to /switch/)
    chdir(ROOT);

    // set the cursor position to 0
    short cursor = 0;

    std::string name = getFirmwareName();
    
    // main menu
    refreshScreen(cursor, name);

    // muh loooooop
    while(appletMainLoop())
    {
        padUpdate(&pad);
        u64 kDown = padGetButtonsDown(&pad);

        // move cursor down...
        if (kDown & HidNpadButton_Down)
        {
            if (cursor == CURSOR_LIST_MAX) cursor = 0;
            else cursor++;
            refreshScreen(cursor, name);
        }

        // move cursor up...
        if (kDown & HidNpadButton_Up)
        {
            if (cursor == 0) cursor = CURSOR_LIST_MAX;
            else cursor--;
            refreshScreen(cursor, name);
        }

        if (kDown & HidNpadButton_A)
        {
            switch (cursor)
            {
            case UP_CFW:
                if (downloadFile(CFW_URL, TEMP_FILE, OFF)){
                    unzip(TEMP_FILE);
                    //remove(APP_OUTPUT);
                    //remove(TEMP_FILE);
                    //rename(TEMP_FILE_HB, APP_OUTPUT);
                    //remove(TEMP_FILE_HB);
                    printDisplay("\033[0;32m\nFini!\n\nRedemarrage automatique dans 5 secondes :)\n");
                    sleep(5);
                    rebootNow();
                }
                
                else
                {
                    printDisplay("\033[0;31mUne erreure est survenue lors du telechargement du cfw. etes vous connecte a internet ?\033[0;37m\n");
                    remove(TEMP_FILE);
                }

                break;

            case UP_APP:
                if (downloadFile(APP_URL, TEMP_FILE_HB, OFF))
                {
                    remove(APP_OUTPUT);
                    rename(TEMP_FILE_HB, APP_OUTPUT);
                    remove(TEMP_FILE_HB);

                    printDisplay("\033[0;32m\nFini!\n\nRedemarrage de l'app dans 5 secondes:)\n");
                    sleep(5);
                    appExit();
                    return 0;
                }
                else
                {
                    printDisplay("\033[0;31mUne erreure est survenue lors du telechargement de l'app. etes vous connecte a internet ?\033[0;37m\n");
                }
                break;
            case UP_SIG:
                if (downloadFile(SIG_URL, TEMP_FILE, OFF))
                {
                    unzip(TEMP_FILE);
                    remove(TEMP_FILE);

                    printDisplay("\033[0;32m\nFini!\n\nRedemarrage de la console dans 5 secondes:)\n");
                    sleep(5);
                    rebootNow();
                }
                else
                {
                    printDisplay("\033[0;31mUne erreure est survenue lors du telechargement des sigpatches. etes vous connecte a internet ?\033[0;37m\n");
                    remove(TEMP_FILE);
                }
            case UP_FIR:
                nlohmann::ordered_json json;
                getRequest(FIR_URL, json);

                if (json == nullptr){
                    printDisplay("\033[0;31mUne erreure est survenue lors du telechargement de la mise a jour. etes vous connecte a internet ?\033[0;37m\n");
                }

                else{
                    auto object = json[0];

                    std::string assets = object.at("assets_url");

                    nlohmann::ordered_json json_assets;
                    getRequest(assets, json_assets);

                    object = json_assets[0];
                    std::string url = object.at("browser_download_url");

                    if (downloadFile(url.c_str(), TEMP_FILE, OFF)){
                        unzip(TEMP_FILE);
                        printDisplay("\033[0;32m\nTéléchargement du firmware teminé. Retrouvez le dans le dossier à la racine de votre carte sd :)\033[0;32m");
                    }

                    else{
                        printDisplay("\033[0;31mUne erreure est survenue lors du telechargement de la mise a jour. Ouvrez une issue sur github si l'erreur perciste.\033[0;37m\n");
                    }
                    
                }
                

            }
        }
        
        // exit...
        if (kDown & HidNpadButton_Plus) break;
    }

    // cleanup then exit
    appExit();
    return 0;
}
