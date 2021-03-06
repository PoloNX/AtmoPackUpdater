#include <switch.h>

#include "menu.hpp"

constexpr int UP_CFW = 0;
constexpr int UP_APP = 1;
constexpr int UP_SIG = 2;
constexpr int UP_FIR = 3;
constexpr int REFRESH = 4;


namespace event{
    void checkInput(PadState &pad, int &cursor, bool &isOpen, menu &menu);
}