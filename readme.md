## AtmoPackUpdater 🖥️

AtmoPackUpdater is an homebrew for Nintendo Switch which update your cfw with the pack of [THZoria](https://github.com/THZoria/THZoria)

## Features 🌟

- Download the latest update of [AtmosphereVanillaPack](https://github.com/THZoria/AtmoPack-Vanilla)
- Download the latest update of this application
- Download the latest [sigpatches](https://github.com/ITotalJustice/patches)
- Download the [firmware](https://github.com/THZoria/NX_Firmware) of your choice

## Screenshot 🎦

![in-une-version-graphique-de-atmopackupdater-dispo-1](https://user-images.githubusercontent.com/57038157/183460024-e2e3441b-f448-41e8-a6d2-6d3da40907dd.jpg)

## How to build 🏗️

### Automatic

*Linux :*  
``git clone --recursive https://github.com/PoloNX/AtmoPackUpdater``   
``cd AtmoPackUpdater``   
``sudo pacman -S switch-curl switch-zlib``  
``./make_nro.sh``

*Window :*  
``git clone --recursive https://github.com/PoloNX/AtmoPackUpdater``  
``cd AtmoPackUpdater``  
``pacman -S switch-curl switch-zlib``  
``./make_nro.bat``  
  
### Manual

``sudo pacman -S switch-curl switch-zlib``  
``git clone --recursive https://github.com/PoloNX/AtmoPackUpdater``  
``cd AtmoPackUpdater/amssu-rcm``  
``make``  
``cd ..``  
``cd amssu-forwarder``  
``make``  
``cd ..``  
``cp amssu-rcm/output/ams_rcm.bin resources/payload/ams_rcm.bin``  
``cp amssu-forwarder/amssu-forwarder.nro resources/forwarder/amssu-forwarder.nro``  
``make``  


## How to test beta 🛠️

To test the beta of this project you have to go on [actions](https://github.com/PoloNX/AtmoPackUpdater/actions) and click on the last commmit. Below you have the download link.
 
## Issues 🚩 

There is no issue :o (For how long?)

## Credits 📜 

- Thanks to [SciresM](https://github.com/SciresM) for [reboot_to_payload](https://github.com/Atmosphere-NX/Atmosphere/tree/master/troposphere/reboot_to_payload)
- Thanks to [THZoria](https://github.com/THZoria/THZoria) for his [pack](https://github.com/THZoria/AtmoPack-Vanilla)
- Thanks to [Team Neptune](https://github.com/Team-Neptune) for your [rcm payload](https://github.com/Team-Neptune/DeepSea-Updater/tree/master/rcm)
- Thanks to [HamletDuFromage](https://github.com/HamletDuFromage) for your [.nro](https://github.com/HamletDuFromage/aio-switch-updater) wich helped me.

## If you like my work you can star this repo 🌟 

