#include <switch.h>

#include <cstdlib>

#include "main_frame.hpp"

#include <borealis.hpp>

namespace i18n = brls::i18n;
using namespace i18n::literals;

int main() {

    if (!brls::Application::init("AtmoPackUpdater")) {
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

    brls::TabFrame* rootFrame = new brls::TabFrame();
    rootFrame->setTitle("AtmoPackUpdater");
    brls::Application::pushView(new MainFrame());

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


