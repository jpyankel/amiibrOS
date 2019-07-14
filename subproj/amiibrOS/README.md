# os_ctrl
os_ctrl is the parent process started just after booting which starts, stops,
and controls all of amiibrOS's features.

It is responsible for:
* Spawning amiibo_scan.py, reading its detections, and acting on those
  detections.
* Spawning and signaling game/UI subprocesses.
* (TODO) Handling power switches and other GPIO/Bluetooth stuff.

os_ctrl launches apps (games / image or gif displays), assuming the following
structure: /usr/bin/amiibrOS/app/########/########, where the #s denote an 8
character hex string corresponding to the first 4 bytes of the amiibo's
character ID (found at start of 15th block of ntag213). Ex:
/usr/bin/amiibrOS/app/12345678/12345678
