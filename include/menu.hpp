#pragma once
#include <iostream>
#include <array>
#include <string>

#include <switch.h>

class menu{
public:
    menu();
    void refreshScreen(int &cursor);
private:
    void getLastFirm();
    void getLastPack();
    void getCurrentPack();
    std::string lastFirm = "";
    std::string lastPack = "";
    std::string currentPack = "";
};

 
