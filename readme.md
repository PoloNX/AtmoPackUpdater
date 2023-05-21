<div align="center">
    <h1>AtmoPackUpdater</h1>
    <p>An Nintendo Switch homebrew which update your cfw.. and more!</p>
</div>

<p align="center">
    <a rel="LICENSE" href="https://github.com/PoloNX/AtmoPackUpdater/blob/master/LICENSE">
        <img src="https://img.shields.io/static/v1?label=license&message=GPLV3&labelColor=111111&color=0057da&style=for-the-badge&logo=data%3Aimage/png%3Bbase64%2CiVBORw0KGgoAAAANSUhEUgAAABQAAAATCAYAAACQjC21AAAACXBIWXMAAAsTAAALEwEAmpwYAAAAIGNIUk0AAHpFAACAgwAA/FcAAIDoAAB5FgAA8QEAADtfAAAcheDStWoAAAFGSURBVHjarJK9LgRhFIafWUuiEH/rJwrJClEq3IELUKgo3IrETWh0FC7BNVih0AoKBQoEydq11qMwm5yMsbPEm3yZd55zvnfO92VQKVhLak09UZeL%2BrsVZ9Qdv2tXnf1NYEndUushZFGthvemuq32FwWuq%2BeZid5DvZGpXambeYGr6qnd9dGldqaudQL3QuFWvVbbmaC6%2BprDr9WbwA4SdQW4BwaABb50CTykfjjwC%2BAx9SPAfOANYDxRCXpOnxNAM4ePA63Ul8NHR4E2QClsGgGG0jUR%2BFjglcAn8/pj4HTwUz/42FPJ68lOSDhCkR/O46XM0Qh3VcRH83jph%2BZefKUosBr8XA%2B%2BmufLAR4Dh6k/CrzWA691YOc/3Ejv6iNM3k59Xw%2B8D3gC9hN1ErjjfzSbqHVg8J8CG2XgBXgL4/9VCdD6HACaHdcHGCRMgQAAAABJRU5ErkJggg%3D%3D" alt=License>
    </a>
    <a rel="VERSION" href="https://github.com/PoloNX/AtmoPackUpdater">
        <img src="https://img.shields.io/static/v1?label=version&message=1.9.0&labelColor=111111&color=06f&style=for-the-badge" alt="Version">
    </a>
    <a rel="BUILD" href="https://github.com/PoloNX/AtmoPackUpdater/actions">
        <img src="https://img.shields.io/github/actions/workflow/status/PoloNX/AtmoPackUpdater/c-cpp.yml?branch=master &labelColor=111111&color=06f&style=for-the-badge" alt=Build>
    </a>
</p>

---
  
     

- [Features](#features)
- [Screenshot](#screenshot)
- [How to build](#how-to-build)
  - [Automatic](#automatic)
    - [Linux](#linux)
    - [Windows](#windows)
  - [Manual](#manual)
    - [Linux](#linux-1)
    - [Windows](#windows-1)
- [How to test the beta](#how-to-test-the-beta)
- [Credits](#credits)

## Features

- Download the latest update of [AtmosphereVanillaPack](https://github.com/THZoria/AtmoPack-Vanilla)
- Download the latest update of this application
- Download the latest [sigpatches](https://github.com/ITotalJustice/patches)
- Download the [firmware](https://github.com/THZoria/NX_Firmware) of your choice

## Screenshot

![screenshot](https://user-images.githubusercontent.com/57038157/183460024-e2e3441b-f448-41e8-a6d2-6d3da40907dd.jpg)

## How to build

### Automatic

#### *Linux :*  
```console
git clone --recursive https://github.com/PoloNX/AtmoPackUpdater  
cd AtmoPackUpdater  
sudo pacman -S switch-curl switch-zlib switch-glfw switch-mesa switch-glm  
./make_nro.sh
```  

#### *Window :* 
```console 
git clone --recursive https://github.com/PoloNX/AtmoPackUpdater  
cd AtmoPackUpdater  
pacman -S switch-curl switch-zlib switch-glfw switch-mesa switch-glm
./make_nro.bat
```
  
### Manual

#### *Linux :*  
```console
sudo pacman -S switch-curl switch-zlib switch-glfw switch-mesa switch-glm  
git clone --recursive https://github.com/PoloNX/AtmoPackUpdater  
cd AtmoPackUpdater/amssu-rcm  
make  
cd ..  
cd amssu-forwarder  
make  
cd ..  
cp amssu-rcm/output/ams_rcm.bin resources/payload/ams_rcm.bin  
cp amssu-forwarder/amssu-forwarder.nro resources/forwarder/amssu-forwarder.nro  
make
```  

#### *Window :* 
```console
pacman -S switch-curl switch-zlib switch-glfw switch-mesa switch-glm  
git clone --recursive https://github.com/PoloNX/AtmoPackUpdater  
cd AtmoPackUpdater/amssu-rcm
make
cd ..
cd amssu-forwarder
make
cd ..
copy amssu-rcm/output/ams_rcm.bin resources/payload/ams_rcm.bin  
copy amssu-forwarder/amssu-forwarder.nro resources/forwarder/amssu-forwarder.nro  
make
```

## How to test the beta

To test the beta of this project you have to go on [actions](https://github.com/PoloNX/AtmoPackUpdater/actions) and click on the last commmit. Below you have the download link.

## Credits 

- Thanks to [SciresM](https://github.com/SciresM) for [reboot_to_payload](https://github.com/Atmosphere-NX/Atmosphere/tree/master/troposphere/reboot_to_payload)
- Thanks to [THZoria](https://github.com/THZoria/THZoria) for his [pack](https://github.com/THZoria/AtmoPack-Vanilla)
- Thanks to [Team Neptune](https://github.com/Team-Neptune) for your [rcm payload](https://github.com/Team-Neptune/DeepSea-Updater/tree/master/rcm)
- Thanks to [HamletDuFromage](https://github.com/HamletDuFromage) for your [.nro](https://github.com/HamletDuFromage/aio-switch-updater) wich helped me.

