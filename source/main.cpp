#include <iostream>
#include <string>
#include <vector>

#include <switch.h>

#include "menu.hpp"
#include "event.hpp"

int main(){
    consoleInit(NULL);
    socketInitializeDefault();
    nxlinkStdio();
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);

    PadState pad;
    padInitializeDefault(&pad);

    bool isOpen = true;
    int cursor = 0;
    menu::refreshScreen(cursor);

    while (appletMainLoop() && isOpen)
    {
        event::checkInput(pad, cursor, isOpen);
        consoleUpdate(NULL);
    }

    // Deinitialize and clean up resources used by the console (important!)
    socketExit();
    consoleExit(NULL);
    return 0;
}