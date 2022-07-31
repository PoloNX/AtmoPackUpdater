#pragma once
#include <iostream>
#include <array>
#include <string>

#include <switch.h>

class menu{
public:
    menu();
    void refreshScreen(int &cursor);
    void getLastFirm();
    void getLastPack();
    void getCurrentPack();
private:
    std::string lastFirm = "";
    std::string lastPack = "";
    std::string currentPack = "";
};

 
