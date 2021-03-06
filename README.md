## AtmoPackUpdater 🖥️

AtmoPackUpdater is an homebrew for Nintendo Switch which update your cfw with the pack of [THZoria](https://github.com/THZoria/THZoria)

## Features 🌟

- Download the last update of [AtmosphereVanillaPack](https://github.com/THZoria/AtmoPack-Vanilla)
- Download the last update of this app
- Download the last [sigpatches](https://github.com/ITotalJustice/patches)
- Download the last [firmware](https://github.com/THZoria/NX_Firmware)

## How to build 🏗️

To build the project you have to download devkitPro, install devkitPro and follow these commands :

``sudo pacman -S switch-curl switch-zlib``  
``git clone https://github.com/PoloNX/AtmoPackUpdater``  
``cd AtmoPackUpdater/amssu-rcm``  
``make``  
``cd ..``  
``cp amssu-rcm/output/ams_rcm.bin romfs/payload/ams_rcm.bin``  
``make``  


## How to test beta 🛠️

To test the beta of the project you have to go on [actions](https://github.com/PoloNX/AtmoPackUpdater/actions) and click on the last commmit. Below you have the download link.
 
## Issues 🚩 

There is no issue :o (For how long?)

## Credits 📜 

- Thanks [SciresM](https://github.com/SciresM) for [reboot_to_payload](https://github.com/Atmosphere-NX/Atmosphere/tree/master/troposphere/reboot_to_payload)
- Thanks [THZoria](https://github.com/THZoria/THZoria) for his [pack](https://github.com/THZoria/AtmoPack-Vanilla)
- Thanks [Team Neptune](https://github.com/Team-Neptune]) for your [rcm payload](https://github.com/Team-Neptune/DeepSea-Updater/tree/master/rcm)
- Thanks [HamletDuFromage](https://github.com/HamletDuFromage) for your help with cURL and json-cpp.
- Thanks [ITotalJustice](https://github.com/ITotalJustice/sigpatch-updater/releases) for your [nro](https://github.com/ITotalJustice/sigpatch-updater/)

## if you like my works you can star this repo 🌟
