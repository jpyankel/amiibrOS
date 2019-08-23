# Adding Apps to amiibrOS
The amiibrOS-overlay is copied post-build to the sdcard.img, overwriting any
files contained within having the same name (hence the name "overlay"). We can
use this feature of Buildroot to add our own custom apps to the filesystem so
that amiibrOS can execute them.

## How it Works
The way amiibrOS will launch an app upon scanning an Amiibo figure is the
following:
* A 4-byte code will be returned from the scanner. These four bytes translate
to an 8 character hex string which we use to identify a character (though some
character variations will share the same code). This hex string will appear as
something like "01000000" and can be found using an online source such as
https://docs.google.com/spreadsheets/d/19E7pMhKN6x583uB6bWVBeaTMyBPtEAC-Bk59Y6cfgxA/htmlview#
by reading characters 1-8 (Link is listed as "01000000" for example).
* amiibrOS will look for an executable shell script (.sh) at
/usr/bin/amiibrOS/app/########/########.sh and will execute this if it has
permission (more on this later).

For example, if we scan Link, amiibrOS translates the 4-byte code to "01000000"
and will attempt to launch /usr/bin/amiibrOS/app/0100000/01000000.sh.

## How to Add Apps
So for each Amiibo figure you own you need to to first find the character code
associated with it via an online source such as the one mentiond earlier.

Once you get a code, you must create a folder in
amiibrOS-overlay/usr/bin/amiibrOS/app/ with the name being the code retrieved.

Then, create a .sh script also with the name of the code that will launch your
custom app.

Before running make in amiibrOS-buildroot, be sure to set the executable flag
on any .sh scripts you create via `chmod +x <your-script-name>`.

## Example
Say the goal is to run a [RetroArch](https://github.com/libretro/RetroArch)
game for when a Mario figure is scanned. I will use the libretro snes9x2010
core found here: https://github.com/libretro/snes9x2010

First find the code for Mario via Internet. It happens to be `00000000`.

Next, `mkdir 00000000` in the directory this README.md is found (in
<project root>/amiibrOS-overlay/usr/bin/amiibrOS/app/).

Before the next step, `cd 00000000` to move into the directory. It is
important that the next file lies in this directory.

Now create a file with a text editor and name it 00000000.sh. In the file write
`retroarch -L /usr/lib/libretro/snes9x2010_libretro.so <path to game>`, where
`<path to game>` is a path to wherever you have chosen to place a compatible
ROM.

Now save and exit from the editor and type `chmod +x 00000000.sh`.

Congratulations. You can repeat similarly for your other Amiibo figures.
