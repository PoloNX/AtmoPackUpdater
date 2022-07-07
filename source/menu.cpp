#include <iostream>
#include <string>
#include <array>

#include <switch.h>

#include "menu.hpp"

const std::array<std::string, 4> OPTION_LIST{
    "= Update le CFW",
    "= Update l'application",
    "= Update les sigpatches",
    "= Telecharger le dernier firmware"
};

constexpr std::string_view APP_VER = "0.0.5";
constexpr int CURSOR_LIST_MAX = 3;

namespace menu{
    void refreshScreen(int &cursor){
    consoleClear();

    std::cout << "AtmoPackUpdater : v" << APP_VER << " by PoloNX" << std::endl << std::endl;
    std::cout << "Appuyez sur (A) pour selectionner une option" << std::endl << std::endl;
    std::cout << "Appuyez sur (+) pour quiiter l'application" << std::endl << std::endl << std::endl << std::endl;

    if (cursor > CURSOR_LIST_MAX)
        cursor = 0;

    else if (cursor < 0)
        cursor = CURSOR_LIST_MAX;

    for (int i = 0; i < CURSOR_LIST_MAX + 1; ++i){
        std::cout << '[' << ((cursor == i) ? 'X' : ' ') << ']' << OPTION_LIST[i] << std::endl;
    }
}
}