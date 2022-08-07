#pragma once

//for the file update_tab.hpp
enum class contentType
{
    ams_cfw,
    app,
    firmwares,
    sigpatches,
};

const std::string APP_VER = "1.0.0"; 

constexpr std::string_view contentTypeNames[4]{"ams_cfw", "app", "firmwares", "sigpatches"};

//Path for the download
const std::string AMS_DOWNLOAD_PATH =     "/config/AtmoPackUpdater/ams.zip";
const std::string SIG_DOWNLOAD_PATH =     "/config/AtmoPackUpdater/sig.zip";
const std::string FIR_DOWNLOAD_PATH =     "/config/AtmoPackUpdater/fir.zip";
const std::string APP_DOWNLOAD_PATH =     "/config/AtmoPackUpdater/app.nro";
const std::string DOWNLOAD_PATH = "/config/AtmoPackUpdater/";
const std::string FORWARDER_PATH = "/config/AtmoPackUpdater/amssu-forwarder.nro";
const std::string DAYBREAK_PATH = "/switch/daybreak.nro";

const std::string ROMFS_FORWARDER = "romfs:/forwarder/amssu-forwarder.nro";

//PATH
const std::string AMS_PATH = "/atmosphere/";
const std::string CONTENTS_PATH = "contents/";
const std::string ROOT = "/";

//URL for the JSON file
const std::string NXLINKS_URL = "https://raw.githubusercontent.com/PoloNX/nx-links/master/nx-links.json";
const std::string APP_URL = "https://github.com/PoloNX/AtmoPackUpdater/releases/latest/download/AtmoPackUpdater.nro";
