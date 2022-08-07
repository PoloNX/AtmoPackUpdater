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

    void extractArchive(contentType type) {
        switch(type) {
            case contentType::ams_cfw:
                extract::unzip(AMS_DOWNLOAD_PATH, ROOT);
                break;
            case contentType::sigpatches:
                extract::unzip(SIG_DOWNLOAD_PATH, ROOT);
                break;
            case contentType::firmwares:
                extract::unzip(FIR_DOWNLOAD_PATH, ROOT);
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

}