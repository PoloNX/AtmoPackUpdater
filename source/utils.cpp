#include "utils.hpp"
#include "progress_event.hpp"
#include "download.hpp"
#include "extract.hpp"
#include "fs.hpp"

#include <switch.h>

#include <iostream>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <thread>

namespace i18n = brls::i18n;
using namespace i18n::literals;

namespace util {
    void createTree(std::string path)
    {
        std::string delimiter = "/";
        size_t pos = 0;
        std::string token;
        std::string directories("");
        while ((pos = path.find(delimiter)) != std::string::npos) {
            token = path.substr(0, pos);
            directories += token + "/";
            std::filesystem::create_directory(directories);
            path.erase(0, pos + delimiter.length());
        }
    }

    void downloadArchive(const std::string& url, contentType type)
    {
        long status_code;
        downloadArchive(url, type, status_code);
    }

    void downloadArchive(const std::string& url, contentType type, long& status_code)
    {
        createTree(DOWNLOAD_PATH);
        switch (type) {
            case contentType::sigpatches:
                status_code = net::downloadFile(url, SIG_DOWNLOAD_PATH, false);
                break;
            case contentType::firmwares:
                status_code = net::downloadFile(url, FIR_DOWNLOAD_PATH, false);
                break;
            case contentType::app:
                status_code = net::downloadFile(url, APP_DOWNLOAD_PATH, false);
                break;
            case contentType::ams_cfw:
                status_code = net::downloadFile(url, AMS_DOWNLOAD_PATH, false);
                break;
            default:
                break;
        }
        ProgressEvent::instance().setStatusCode(status_code);
    }

    std::string getContentsPath()
    {
        return std::string(AMS_PATH) + std::string(CONTENTS_PATH);
    }

    std::string getErrorMessage(long status_code) {
        std::string res;
        switch (status_code) {
            case 500:
                res = fmt::format("{0:}: Internal Server Error", status_code);
                break;
            case 503:
                res = fmt::format("{0:}: Service Temporarily Unavailable", status_code);
                break;
            default:
                res = fmt::format("error: {0:}", status_code);
                break;
        }
        return res;
    }

    std::string getPackVersion() {
        std::fstream txtfile;
        chdir("/");
        txtfile.open("version.txt", std::ios::in);
        if (txtfile.is_open()) {
            std::string version;
            std::getline(txtfile, version);
            txtfile.close();

            version.erase(0, 18);
            version.pop_back();

            return version;
        }

        else {
            return "";
        }
    }

    int showDialogBoxBlocking(const std::string& text, const std::string& opt1, const std::string& opt2)
    {
        int result = -1;
        brls::Dialog* dialog = new brls::Dialog(text);
        brls::GenericEvent::Callback callback1 = [dialog, &result](brls::View* view) {
            result = 0;
            dialog->close();
        };
        brls::GenericEvent::Callback callback2 = [dialog, &result](brls::View* view) {
            result = 1;
            dialog->close();
        };
        dialog->addButton(opt1, callback1);
        dialog->addButton(opt2, callback2);
        dialog->setCancelable(false);
        dialog->open();
        while (result == -1) {
            std::this_thread::sleep_for(std::chrono::microseconds(10));
        }
        std::this_thread::sleep_for(std::chrono::microseconds(800000));
        return result;
    }

    void extractArchive(contentType type) {
        switch(type) {
            case contentType::ams_cfw: {
                int overwriteInis = showDialogBoxBlocking("menu/dialog/overwrite_ini"_i18n, "menu/dialog/yes"_i18n, "menu/dialog/no"_i18n);
                extract::unzip(AMS_DOWNLOAD_PATH, ROOT, overwriteInis);
                break;
            }
            case contentType::sigpatches:
                extract::unzip(SIG_DOWNLOAD_PATH, ROOT, 1);
                break;
            case contentType::firmwares:
                extract::unzip(FIR_DOWNLOAD_PATH, ROOT, 1);
                break;
            case contentType::app:
                cp("romfs:/forwarder/amssu-forwarder.nro", "/config/AtmoPackUpdater/amssu-forwarder.nro");
                envSetNextLoad(FORWARDER_PATH.c_str(), fmt::format("\"{}\"", FORWARDER_PATH).c_str());
                romfsExit();
                brls::Application::quit();
                break;
        }
    }

    bool isErista()
    {
        SetSysProductModel model;
        setsysGetProductModel(&model);
        return (model == SetSysProductModel_Nx || model == SetSysProductModel_Copper);
    }

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

    bool set90dns() {
        Result res = 0;
        SetRegion region;
        u32 europeDns = 0xdb8daca3; // 163.172.141.219
        u32 americaDns = 0x4d79f6cf; // 207.246.121.77

        u32 primaryDns = 0;
        u32 secondaryDns = 0;
        res = setInitialize();
        if (res){
            setExit();
            setsysExit();
            return false;
        }
        else {
            res = setsysInitialize();
            if (res){
                setExit();
                setsysExit();
                return false;
            }
            else {
                res = setGetRegionCode(&region);
                if (res){
                    setExit();
                    setsysExit();
                    return false;
                }
                else {
                    if (region <= SetRegion_CHN){
                        if (region == SetRegion_USA){
                            primaryDns = americaDns;
                            secondaryDns = europeDns;
                        }
                        else {
                            primaryDns = europeDns;
                            secondaryDns = americaDns;
                        }
                    }
                    else {
                        primaryDns = europeDns;
                        secondaryDns = americaDns;
                    }
                    
                }
            }
        }

        SetSysNetworkSettings* wifiSettings = (SetSysNetworkSettings*) malloc(sizeof(SetSysNetworkSettings) * 0x200);

        if (wifiSettings != NULL){
            s32 entryCount = 0;
            res = setsysGetNetworkSettings(&entryCount, wifiSettings, 0x200);
            if (res){
                free(wifiSettings);
                setExit();
                setsysExit();
                return false;
            }
            else {
                for (int i = 0; i < entryCount; i++){
                    wifiSettings[i].primary_dns = primaryDns;
                    wifiSettings[i].secondary_dns = secondaryDns;
                    wifiSettings[i].auto_settings &= ~SetSysAutoSettings_AutoDns;
                }

                if (entryCount){
                    res = setsysSetNetworkSettings(wifiSettings, entryCount);
                    if (res){
                        free(wifiSettings);
                        setExit();
                        setsysExit();
                        return false;
                    }
                }
            }
        }
        free(wifiSettings);

        setExit();
        setsysExit();
        return true;
    }

    bool deleteTheme() {
        std::string contentsPath = util::getContentsPath();
        bool themeDeleted = false;
        for (const auto& tid : {"0100000000001000/romfs/lyt", "0100000000001007/romfs/lyt", "0100000000001013/romfs/lyt"}) {
            if (std::filesystem::exists(contentsPath + tid) && !std::filesystem::is_empty(contentsPath + tid)) {
                themeDeleted = true;
            }
        }
        if (themeDeleted){
            std::filesystem::remove_all("atmosphere/contents/0100000000001000/romfs/lyt");
            std::filesystem::remove_all("atmosphere/contents/0100000000001007/romfs/lyt");
            std::filesystem::remove_all("atmosphere/contents/0100000000001013/romfs/lyt");
            return true;
        }
        else    
            return false;
    }

}