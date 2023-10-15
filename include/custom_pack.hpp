#pragma once

#include <borealis.hpp>
#include <json.hpp>
#include "constants.hpp"

class Pack {
private:
    std::string m_name;
    std::string m_url;
public:
    std::string getName() const { return m_name; } 
    std::string getUrl() const { return m_url; } 
    Pack(const std::string name, const std::string url):m_name(name), m_url(url) {}
};

class CustomPack : public brls::List {
private:
    brls::ListItem* m_item;
    brls::Label* m_label;
    std::vector<Pack> Packs;
    void addPack(Pack& pack);
    void deletePack(Pack pack);
    std::vector<Pack> getPacks();
public:
    CustomPack();
};