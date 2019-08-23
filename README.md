# amiibrOS

This repository documents the features and building process of amiibrOS, the
software component for amiibrOS-display, a summer project started in 2019. Also
included are building materials for the software and a tutorial for the build
procedure.

For more information about the project as well as a wealth of tutorials on how
to build your own, please see my personal projects website:
https://jpyankel.github.io/projects/amiibrOS.html

## Features
* Raspberry Pi buildroot default configuration for building packages necessary
for amiibrOS hardware support.
* Automatically cross-compiles and packages all of the amiibrOS system
software.
* Customizable slideshow app that parses a configuration file to display images
and optionally animate them across the screen.
* Customizable overlay to include RetroArch games or amiibrOS slideshow apps as
well as WiFi configuration.
* Documented code, tutorials on customization, and instructions on how to build
amiibrOS system software to get a bootable Raspberry Pi amiibrOS display/video-
games console.

## Build Instructions
The following procedure will result in an amiibrOS SD card bootable on a
Raspberry Pi 3B+.

### Prerequisites:
* SD card with size > 4GB
* Ability to write to an SD card on your host PC.
* Linux with the mandatory packages denoted by
amiibrOS-buildroot/docs/manual/prerequisite.txt (a Docker container for this
build process may eventually be made).

### 1. Clone Repository with --recursive
Type in a command prompt and `cd` to wherever you want the root folder of this
project to be placed. Then run
`git clone --recursive https://github.com/jpyankel/amiibrOS.git`.

### 2. Setup amiibrOS-buildroot
First, cd into amiibrOS-buildroot by typing `cd amiibrOS/amiibrOS-buildroot`.

Type the following commands: `make amiibrOS_defconfig`

### 2a. Precongifure Wifi Settings (Optional)
You may want to utilize the wifi connection capability of the Raspberry Pi.
This has many benefits, including being able to ssh into the Raspberry Pi and
add/change programs. If so, you should create your own
`<project root>/amiibrOS/amiibrOS-overlay/etc/wpa_supplicant.conf`. See
`wpa_supplicant.conf.example` in the same directory for an example of how to
create your wpa_supplicant.conf.

### 3. Build amiibrOS-buildroot
Ensure you are back in the `amiibrOS/amiibrOS-buildroot` directory. To start
the building process, simply type `make`.

Note that this will take a very long time (3+ Hours). It has to build the
entire cross-compilation toolchain for the Raspberry Pi and use that toolchain
to build the various sub-projects that make up amiibrOS. After this, any
customizations made via `make menuconfig` (not necessary to run) will build
faster when you re-type `make`.

### 4. Install/Configure Programs per Amiibo
We will be making changes in `amiibrOS/amiibrOS-overlay/usr/bin/amiibrOS/app`.
Please `cd` to this directory and read the README.md there for instructions on
how to add your own slideshows, RetroArch games, or other programs.

### 5. Copy to SD Card
Type `sudo dd if=output/images/sdcard.img of=/dev/mmcblk0 bs=4M`. This command
will also take a while, but not nearly as long as the amiibrOS-buildroot build
process.

After the copy finishes, you can slot the SD card back into the Raspberry Pi
and boot it up. Enjoy!

## License
[MIT](https://github.com/jpyankel/amiibrOS/blob/master/LICENSE.md)

## TODO/Future Ideas
This project is mostly complete and will probably not be returned to any time
soon (other than for small software fixes), but there are some polishing
details that were originally intended for the project that didn't end up making
it into the build before the summer ended.

These include:
* Hiding of the command prompt by switching default tty to a tty other than
tty1. The console would be shown again by reverting to tty1 after manually
exiting amiibrOS via the escape key from the USB keyboard.
* Testing (and likely fixing) all slideshow configurations.
