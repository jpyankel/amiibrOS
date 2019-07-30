# amiibrOS

This repository documents the features and building process of amiibrOS, the
software component for amiibrOS-display, a summer project started in 2019. Also
included are building materials for the software and a tutorial for the build
procedure.

For more information about the project as well as a wealth of tutorials on how
to build your own, please see my personal projects website:
https://github.com/jpyankel/jpyankel.github.io/amiibrOS

## Features
(TODO)

## Build Instructions
The following procedure will result in an amiibrOS SD card bootable on a
Raspberry Pi 3B+.

### Prerequisites:
* SD Card (TODO SIZE)
* Ability to write to an SD card on your host PC.
* (TODO OS and libraries) (maybe make a Docker container for this)

### 1. Clone Repository with --recursive
Type in a command prompt and `cd` to wherever you want the root folder of this
project to be placed. Then run
`git clone --recursive https://github.com/jpyankel/amiibrOS.git`.

### 2. Setup amiibrOS-buildroot
First, cd into amiibrOS-buildroot by typing `cd amiibrOS/amiibrOS-buildroot`.

Now, depending on whether you want to build amiibrOS for display or console
mode, type one of the following commands:
* Display Mode: `make amiibrOS_display_defconfig`
* Console Mode: `make amiibrOS_console_defconfig`

### 2a. Precongifure Wifi Settings (Optional)
If Console Mode was chosen, you may want to utilize the wifi connection
capability of the Raspberry Pi. This has many benefits, including being able to
ssh into the Raspberry Pi and add/change programs. If so, you should create
your own `<project root>/amiibrOS/amiibrOS-overlay/etc/wpa_supplicant.conf`.
See `wpa_supplicant.conf.example` in the same directory for an example of how
to create your wpa_supplicant.conf.

### 3. Build amiibrOS-buildroot
Ensure you are back in the `amiibrOS/amiibrOS-buildroot` directory. To start
the building process, simply type `make`.

Note that this will take a very long time (3+ Hours). It has to build the
entire cross-compilation toolchain for the Raspberry Pi and use that toolchain
to build the various sub-projects that make up amiibrOS.

(TODO include timing stats)

### 4. Install/Configure Programs per Amiibo
We will be making changes in `amiibrOS/amiibrOS-overlay/usr/bin/amiibrOS/app`.
Please `cd` to this directory and read the README.md there for instructions on
how to add your own slideshows, RetroArch games, or other programs.

### 5. Copy to SD Card
Type `sudo dd (TODO)`. This command will also take a while, but not nearly as
long as the amiibrOS-buildroot build process.

(TODO include timing stats)

After the copy finishes, you can slot the SD card back into the Raspberry Pi
and boot it up. Enjoy!

## License
[MIT](https://github.com/jpyankel/amiibrOS/blob/master/LICENSE.md)
