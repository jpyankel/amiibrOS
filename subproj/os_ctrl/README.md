# os_ctrl
os_ctrl is the parent process started just after booting which starts, stops,
and controls all of amiibrOS's features.

It is responsible for:
* Spawning amiibo_scan.py, reading its detections, and acting on those
  detections.
* Spawning and signaling game/UI subprocesses.
* (TODO) Handling power switches and other GPIO/Bluetooth stuff.
