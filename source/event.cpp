#include <switch.h>
#include <json.hpp>
#include <filesystem>

#include <stdio.h>
#include <minizip/unzip.h>
#include <string.h>
#include <dirent.h>
#include <switch.h>
#include <sys/types.h>
#include <sys/stat.h>


#include "event.hpp"
#include "menu.hpp"
#include "download.hpp"
#include "unzip.hpp"
#include "reboot.hpp"

bool cp(char *filein, char *fileout) {
	FILE *exein, *exeout;
	exein = fopen(filein, "rb");
	if (exein == NULL) {
		/* handle error */
		perror("file open for reading");
		return false;
	}
	exeout = fopen(fileout, "wb");
	if (exeout == NULL) {
		/* handle error */
		perror("file open for writing");
		return false;
	}
	size_t n, m;
	unsigned char buff[8192];
	do {
		n = fread(buff, 1, sizeof buff, exein);
		if (n) m = fwrite(buff, 1, n, exeout);
		else   m = 0;
	}
	while ((n > 0) && (n == m));
	if (m) {
		perror("copy");
		return false;
	}
	if (fclose(exeout)) {
		perror("close output file");
		return false;
	}
	if (fclose(exein)) {
		perror("close input file");
		return false;
	}
	return true;
}

namespace event{
    void checkInput(PadState &pad, int &cursor, bool &isOpen, menu &menu){
        padUpdate(&pad);
        u64 kDown = padGetButtonsDown(&pad);

        if (kDown & HidNpadButton_Down){
            ++cursor;
            menu.refreshScreen(cursor);
        }

        if (kDown & HidNpadButton_Up){
            --cursor;
            menu.refreshScreen(cursor);
        }

        if (kDown & HidNpadButton_A){
            switch(cursor){
                case UP_CFW:
                    if(net::downloadFile(CFW_URL, TEMP_FILE, false)){
                        extract::unzip(TEMP_FILE, "/");
                        remove(APP_OUTPUT.c_str());
                        rename(TEMP_FILE_HB.c_str(), APP_OUTPUT.c_str());
                        menu.refreshScreen(cursor);
                        std::cout << "\n\nTelechargement du pack termine. redemarrage de la console dans 3 secondes..." << std::endl;
                        consoleUpdate(NULL);
                        sleep(3);
                        reboot::rebootNow();
                    }
                    
                    break;

                case UP_APP:
                    if(net::downloadFile(APP_URL, TEMP_FILE_HB, false)){
                        menu.refreshScreen(cursor);
                        std::cout << "\n\nTelechargement de l'app termine. redemarrage de l'app dans 3 secondes..." << std::endl;
                        consoleUpdate(NULL);
                        mkdir("/switch/AtmoPackUpdater/", 0777);
                        cp((char*) "romfs:/forwarder/amssu-forwarder.nro", (char*) "/switch/AtmoPackUpdater/amssu-forwarder.nro");
                        envSetNextLoad("switch/AtmoPackUpdater/amssu-forwarder.nro", "\"/switch/AtmoPackUpdater/amssu-forwarder.nro\"");
                        sleep(3);
                        isOpen = false;
                    }
                    break;

                case UP_SIG:
                    if (net::downloadFile(SIG_URL, TEMP_FILE, false)){
                        extract::unzip(TEMP_FILE, "/");
                        menu.refreshScreen(cursor);
                        std::cout << "\n\nTelechargement des sigpatches temine. redemarrage de la console dans 3 secondes..." << std::endl;
                        consoleUpdate(NULL);
                        sleep(3);
                        reboot::rebootNow();
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
                            menu.refreshScreen(cursor);
                            std::cout << "\n\nTelechargement du dernier firmware temine! Veuillez l'installer avec DayBreak.\n\nVoulez vous supprimer votre theme ?\n\n[A] -> Oui\n[B] -> Non" << std::endl;
                            consoleUpdate(NULL);
                            envSetNextLoad("switch/daybreak.nro", "\"/switch/daybreak.nro\"");
                            consoleUpdate(NULL);
                            while(true){
                                padUpdate(&pad);
                                u64 kDownTheme = padGetButtonsDown(&pad);
                                if(kDownTheme & HidNpadButton_A){
                                    std::filesystem::remove_all("atmosphere/contents/0100000000001000/romfs/lyt");
                                    std::filesystem::remove_all("atmosphere/contents/0100000000001007/romfs/lyt");
                                    std::filesystem::remove_all("atmosphere/contents/0100000000001013/romfs/lyt");
                                    break;
                                }

                                else if(kDownTheme & HidNpadButton_B){
                                    break;
                                }
                            }
                            isOpen = false;
                        }
                        else{
                            menu.refreshScreen(cursor);
                            std::cout << "\n\nUne erreur de connexion est survenue." << std::endl;
                            consoleUpdate(NULL);
                            sleep(5);
                            menu.refreshScreen(cursor);
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