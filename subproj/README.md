# amiibrOS System Software
In each of the folders in this directory (excluding include, which contains
Raylib development header files). This README contains a brief description of
each one.

## Software
### amiibo_scan
Python script which uses Adafruit's CircuitPython PN532 library to
interface with the PN532 NFC Module. It communicates with amiibrOS by sending
the Amiibo figure's character ID code so that amiibrOS can launch an app
related to that character.

### amiibrOS
The main software which spawns the amiibo_scan script and listens to it via
a pipe. This software acts sort of like a shell, but with the input being
Amiibo figure codes; it handles launching and reaping app processes. It also
comes with an interface and an optional method to escape to the command line.

### powerswitch
Software for the Raspberry Pi's halt and wake functionality. It is started by
init.d/S32powerswitch.sh (which is included in the amiibrOS-overlay) and
waits for a GPIO pin to drop low signaling that the hardware power switch was
closed. The wake functionality is builtin to the Raspberry Pi. You can read
more about it here:
https://howchoo.com/g/mwnlytk3zmm/how-to-add-a-power-button-to-your-raspberry-pi

### slideshow
Custom slideshow app to be compiled separately and included manually in
amiibrOS-overlay/usr/bin/amiibrOS/app/<charID>/. It also serves as an example
app which can be added to the amiibrOS display/console. This app is optional
and the documentation/tutorials contains more information on how to use it.
