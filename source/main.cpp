#include <iostream>
#include <string>
#include <vector>

#include <switch.h>

#include "menu.hpp"
#include "event.hpp"

void init(){
    consoleInit(NULL);
    socketInitializeDefault();
    nxlinkStdio();
    romfsInit();
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);
}
 
int main(){
    init();

    PadState pad;
    padInitializeDefault(&pad);

    std::cout << "Chargement..." << std::endl;
    consoleUpdate(NULL);

    bool isOpen = true;
    int cursor = 0;
    menu menu;
    menu.refreshScreen(cursor);

    while (appletMainLoop() && isOpen)
    {
        event::checkInput(pad, cursor, isOpen, menu);
        consoleUpdate(NULL);
    }

    // Deinitialize and clean up resources used by the console (important!)
    socketExit();
    consoleExit(NULL);
    return 0;
}