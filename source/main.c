#include <stdio.h>
#include <stdarg.h>
#include <unistd.h> // chdir
#include <dirent.h> // mkdir
#include <switch.h>

#include "download.h"
#include "unzip.h"
#include "reboot.h"

#define ROOT                    "/"
#define APP_PATH                "/switch/"
#define APP_OUTPUT              "/switch/AtmoPackUpdater.nro"

#define APP_VERSION             "0.0.4"
#define CURSOR_LIST_MAX         2


const char *OPTION_LIST[] =
{
    "= Update AtmoPack-Vanilla",
    "= Update AtmoPack-Full",
    "= Update l'application",
    "= Update les sigpatches"

};

void refreshScreen(int cursor)
{
    consoleClear();

    printf("\x1B[36mAtmoPackUpdater: v%s", APP_VERSION);
    printf(" by ItolalJustice and edited by PoloNX\n\n\n");
    printf("\033[0;33mAppuyez sur (A) pour selectionner une option\n\n");
    printf("Appuyez sur (+) pour quitter l'application\033[0;37m\n\n\n");

    for (int i = 0; i < CURSOR_LIST_MAX + 1; i++)
        printf("[%c] %s\n\n", cursor == i ? 'X' : ' ', OPTION_LIST[i]);

    consoleUpdate(NULL);
}

void printDisplay(const char *text, ...)
{
    va_list v;
    va_start(v, text);
    vfprintf(stdout, text, v);
    va_end(v);
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

    // main menu
    refreshScreen(cursor);

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
            refreshScreen(cursor);
        }

        // move cursor up...
        if (kDown & HidNpadButton_Up)
        {
            if (cursor == 0) cursor = CURSOR_LIST_MAX;
            else cursor--;
            refreshScreen(cursor);
        }

        if (kDown & HidNpadButton_A)
        {
            switch (cursor)
            {
            case UP_CFW:
                if (downloadFile(CFW_URL, TEMP_FILE, OFF)){
                    unzip("/switch/temp.zip");
                    remove(APP_OUTPUT);
                    remove(TEMP_FILE);
                    remove(TEMP_FILE_HB);
                    rename(TEMP_FILE_HB, APP_OUTPUT);
                    printDisplay("\033[0;32m\nFini!\n\nRedemarage automatique dans 5 secondes :)\n");
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

                    printDisplay("\033[0;32m\nFini!\n\nRedemarage de l'app dans 5 secondes:)\n");
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
                    unzip("/switch/AtmoPackUpdater/temp.zip");
                    remove(TEMP_FILE);
                    rebootNow();
                }
                else
                {
                    printDisplay("\033[0;31mUne erreure est survenue lors du telechargement des sigpatches. etes vous connecte a internet ?\033[0;37m\n");
                    remove(TEMP_FILE);
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
