#include <switch.h>

#include "event.hpp"
#include "menu.hpp"
#include "download.hpp"

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
                    net::downloadFile(CFW_URL, TEMP_FILE, false);
                    break;
                case UP_APP:
                    std::cout << "app" << std::endl;
                    break;
                case UP_SIG:
                    std::cout << "sig" << std::endl;
                    break;
                case UP_FIR:
                    std::cout << "firm" << std::endl;
                    break;
            }
        }

        if (kDown & HidNpadButton_Plus){
            isOpen = false;
        }   
    }
}