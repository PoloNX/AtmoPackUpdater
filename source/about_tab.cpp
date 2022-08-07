#include "about_tab.hpp"

namespace i18n = brls::i18n;
using namespace i18n::literals;

CreditsTab::CreditsTab () : brls::List () {

    //Subtitle
    brls::Label* subTitle = new brls::Label(
        brls::LabelStyle::REGULAR,
        "menu/about/credits"_i18n,
        true);
    
    subTitle->setHorizontalAlign(NVG_ALIGN_CENTER);
    this->addView(subTitle);

    // Copyright
    brls::Label* copyright = new brls::Label(
        brls::LabelStyle::DESCRIPTION,
        ("menu/about/copyright"_i18n + "\n"),
        true);
    copyright->setHorizontalAlign(NVG_ALIGN_CENTER);
    this->addView(copyright);

    // Links
    this->addView(new brls::Header("Remerciement à :"));
    brls::Label* links = new brls::Label(
        brls::LabelStyle::SMALL,
        "menu/about/thanks"_i18n,
        true);
    this->addView(links);
}