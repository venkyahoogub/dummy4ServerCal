# DMD Driver

The DMD Driver is used and a hardware abstraction layer for the DMD micromirror array, which is a USB connected device.

This repository is intended to be build as a library that can be linked into a Linux parent application.

## Installing the build environment

Make sure you have the following tools installed.

## Cmake

sudo apt-get install cmake

Recommended version: 3.22.1

## Build essentials including g++

sudo apt-get install build-essentials

Recommended g++ version: 9.4.0

## Vialux Alp43 library

This driver depends on the Vialux Alp43 Linux library for the V4395 hardware set.
The current version of the library is a beta release.

### Installation on Ubuntu

```
sudo dpkg -i libalp43_0.0.2-beta_amd64.deb
```

ALP devices belong to the vialux_alp group by default (see /etc/udev/rules.d/80-alp43.rules).
To access ALP devices, add your user to the vialux_alp group.

```
sudo usermod -aG vialux_alp $(id -un).
```

## Building the code

The code is built using Cmake.

```
mkdir build && cd build && cmake .. && make
```

