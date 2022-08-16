#include <switch.h>

#include <cstdlib>

#include "main_frame.hpp"
#include "constants.hpp"
#include "utils.hpp"
#include "warning_page.hpp"

#include <borealis.hpp>

namespace i18n = brls::i18n;
using namespace i18n::literals;

int main() {

    if (!brls::Application::init(fmt::format("{}{}", "AtmoPackUpdater v", APP_VER))) {
        brls::Logger::error("Unable to init Borealis application");
        return EXIT_FAILURE;
    }

    i18n::loadTranslations();

    setsysInitialize();
    plInitialize(PlServiceType_User);
    nsInitialize();
    socketInitializeDefault();
    nxlinkStdio();
    pmdmntInitialize();
    pminfoInitialize();
    splInitialize();
    romfsInit();

    brls::Logger::setLogLevel(brls::LogLevel::DEBUG);
    brls::Logger::debug("Start");

    if (!util::isExfat()) {
        brls::Application::pushView(new MainFrame());
    } else {
        brls::Application::pushView(new WarningPage("menu/error/exfat"_i18n));
    }


    while (brls::Application::mainLoop()) {
        ;
    }

    romfsExit();
    splExit();
    pminfoExit();
    pmdmntExit();
    socketExit();
    nsExit();
    setsysExit();
    plExit();
    return EXIT_SUCCESS;
}


