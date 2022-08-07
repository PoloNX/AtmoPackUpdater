#include "about_tab.hpp"

CreditsTab::CreditsTab () : brls::List () {

    //Subtitle
    brls::Label* subTitle = new brls::Label(
        brls::LabelStyle::REGULAR,
        "AtmoPackUpdater Crédits",
        true);
    
    subTitle->setHorizontalAlign(NVG_ALIGN_CENTER);
    this->addView(subTitle);

    // Copyright
    brls::Label* copyright = new brls::Label(
        brls::LabelStyle::DESCRIPTION,
        ("copyright © 2022 PoloNX\n"),
        true);
    copyright->setHorizontalAlign(NVG_ALIGN_CENTER);
    this->addView(copyright);

    // Links
    this->addView(new brls::Header("Remerciement à :"));
    brls::Label* links = new brls::Label(
        brls::LabelStyle::SMALL,
        "THZoria for your AtmoPack-Vanilla\n\nSciresM for reboot_to_payload\n\nTeam Neptune for your rcm payload\n\nHamletDuFromage for your .nro which helped me.",
        true);
    this->addView(links);
}