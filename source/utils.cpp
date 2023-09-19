#include "utils.hpp"
#include "progress_event.hpp"
#include "download.hpp"
#include "extract.hpp"
#include "fs.hpp"
#include "reboot.hpp"

#include <switch.h>

#include <iostream>
#include <chrono>
#include <dirent.h>
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

    void downloadArchive(const std::string& url, contentType type, bool homebrew)
    {
        long status_code;
        downloadArchive(url, type, status_code, homebrew);
    }

    void downloadArchive(const std::string& url, contentType type, long& status_code, bool homebrew)
    {
        createTree(DOWNLOAD_PATH);
        switch (type) {
            case contentType::sigpatches:
                status_code = net::downloadFile(url, SIG_DOWNLOAD_PATH, false);
                break;
            case contentType::firmwares:
                fs::removeDir(FIRMWARE_PATH);
                status_code = net::downloadFile(url, FIR_DOWNLOAD_PATH, false);
                break;
            case contentType::app:
                status_code = net::downloadFile(url, APP_DOWNLOAD_PATH, false);
                break;
            case contentType::ams_cfw:
                status_code = net::downloadFile(url, AMS_DOWNLOAD_PATH, false);
                break;
            case contentType::homebrew: {
                std::size_t last_slash_pos = url.find_last_of("/");
                std::string filename = url.substr(last_slash_pos + 1);
                if(homebrew) {
                    std::string foldername = filename.substr(0, filename.find_last_of("."));
                    int choice = showDialogBoxBlocking("menu/dialog/homebrew_path"_i18n, fmt::format("{}{}", SWITCH_PATH, filename), fmt::format("{}{}/{}", SWITCH_PATH, foldername, filename));
                    std::string output;
                    if(choice == 0) {
                        output = fmt::format("{}{}", SWITCH_PATH, filename);
                        brls::Logger::debug("Output = {}", output);
                        
                        fs::removeDir(fmt::format("{}{}", SWITCH_PATH, foldername));
                        status_code = net::downloadFile(url, output, false);
                    }
                    else {
                        output = fmt::format("{}{}/{}", SWITCH_PATH, foldername, filename);
                        std::string downloadPath = fmt::format("{}{}", SWITCH_PATH, foldername);
                        std::string oldPath = fmt::format("{}{}", SWITCH_PATH, filename);

                        brls::Logger::debug("Output = {}", output);

                        brls::Logger::debug("Remove file : {}", oldPath);
                        fs::removeFile(oldPath);

                        brls::Logger::debug("Download path : {}", downloadPath);
                        std::filesystem::create_directories(downloadPath);
                        status_code = net::downloadFile(url, output, false);
                    }
                }
                else
                    status_code = net::downloadFile(url, SYSMODULES_DOWNLOAD_PATH, false);
                break;
            }
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

    int showDialogBoxBlocking(const std::string& text, const std::string& opt)
    {
        brls::Dialog* dialog = new brls::Dialog(text);
        brls::GenericEvent::Callback callback = [dialog](brls::View* view) {
            dialog->close();
        };
        dialog->addButton(opt, callback);
        dialog->setCancelable(true);
        dialog->open();
        while(true) {
            std::this_thread::sleep_for(std::chrono::microseconds(10));
        }
        std::this_thread::sleep_for(std::chrono::microseconds(800000));
        return 0;
    }

    int showDialogBox(const std::string& text, const std::string& opt1, const std::string& opt2) {
        brls::Dialog* dialog = new brls::Dialog(text);
        brls::GenericEvent::Callback closeCallBack = [dialog](brls::View* view) {
            dialog->close();
        };

        brls::GenericEvent::Callback rebootCallBack = [dialog](brls::View* view) {
            dialog->close();
            reboot::rebootNow();
        };

        dialog->addButton(opt1, rebootCallBack);
        dialog->addButton(opt2, closeCallBack);

        dialog->setCancelable(true);
        dialog->open();

        return 0;
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
            case contentType::firmwares: {
                //Detect sysmodule and delete them if the user want
                const std::filesystem::path contents_dir{AMS_PATH + CONTENTS_PATH};

                if (std::filesystem::exists(contents_dir)) {
                    if (!contents_dir.empty()) {
                        auto contents_json = nlohmann::ordered_json::array();
                        for (auto const& ent : std::filesystem::directory_iterator{contents_dir}){
                            std::ifstream sysconfig(std::string(ent.path()) + "/toolbox.json");
                            if (!sysconfig.fail()) {
                                auto data = nlohmann::ordered_json::parse(sysconfig);
                                contents_json.push_back(data);
                            }
                        }
                        if (contents_json.size()) {
                            std::string content = "menu/dialog/sysmodules"_i18n;
                            for (auto i : contents_json) {
                                content += i["name"].get<std::string>() + ", ";
                            }
                            int deletesysmodules = showDialogBoxBlocking(content, "menu/dialog/yes"_i18n, "menu/dialog/no"_i18n);

                            if (deletesysmodules == 0) {
                                for (auto i : contents_json) {
                                    std::string path = AMS_PATH + CONTENTS_PATH + i["tid"].get<std::string>();
                                    fs::removeDir(path);
                                }
                            }
                        }
                    }
                }
             
                extract::unzip(FIR_DOWNLOAD_PATH, "/firmware/", 1);
                break;
            }

            case contentType::homebrew:
                extract::unzip(SYSMODULES_DOWNLOAD_PATH, ROOT, 0);
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

    std::string getAppPath() {
         if (envHasArgv()) {
            std::string argv = (char*)envGetArgv();
            return fs::splitString(argv, '\"')[1].substr(5);
        }
        return NRO_PATH;
    }

    //bool isExfat() {
    //    bool exfat_support = fsIsExFatSupported(&exfat_support);
    //    return exfat_support;
    //}

    bool isApplet() {
        AppletType at = appletGetAppletType();
	    return at != AppletType_Application && at != AppletType_SystemApplication;
    }

    std::vector<int> split_version(const std::string& version) {
        std::vector<int> components;
        std::string::size_type pos = 0;
        while (pos < version.size()) {
            std::string::size_type next_pos = version.find('.', pos);
            if (next_pos == std::string::npos) {
                next_pos = version.size();
            }
            std::string component_str = version.substr(pos, next_pos - pos);
            int component = std::stoi(component_str);
            components.push_back(component);
            pos = next_pos + 1;
        }
        return components;
    }

    bool is_older_version(const std::string& version1, nlohmann::json json) {
        if (!json.empty()) {
            std::string version2 = json.at("app").at("version").get<std::string>();

            std::vector<int> components1 = split_version(version1);
            std::vector<int> components2 = split_version(version2);

            // Compare the components of the versions
            for (size_t i = 0; i < components1.size(); i++) {
                if (components1[i] < components2[i]) {
                    return true;
                } else if (components1[i] > components2[i]) {
                    return false;
                }
            }

            // If all components are equal, the versions are the same
            return false;
        }
        return false;
    }
    const nlohmann::ordered_json getValueFromKey(const nlohmann::ordered_json& jsonFile, const std::string& key)
    {
        return (jsonFile.find(key) != jsonFile.end()) ? jsonFile.at(key) : nlohmann::ordered_json::object();
    }
}