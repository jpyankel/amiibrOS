# amiibrOS
This README document will give an overview of the scripts contained within this
folder as well as technical details on what they accomplish.

Note that amiibrOS should not be compiled manually (unless for testing). The
amiibrOS-buildroot bundled with this project will automatically cross-compile
this for the Raspberry Pi 3B+.

## Purpose
The amiibrOS system software in this folder serves three purposes:
1. To provide a main interface for the user. This main interface will provide
simple, non-technical instruction on how to use amiibrOS.
2. To launch various sub-processes which handle various hardware components or
run applications.
3. To handle errors safely and bring the user back to the main interface if
something were to occur.

## Technical Details Overview
Upon startup, amiibrOS's main.c will set up a single pipe for communication
with the scanner (amiibo_scan.py). amiibrOS will use only the read-end of the
pipe while amiibo_scan.py will use the write end.

Signals are blocked shortly after to allow for safe execution of the following:
* Installation of SIGCHLD handler (for handling unexpected sub-process
termination, such as when an app dies, we should reboot the main interface)
* Installation of SIGTERM and SIGINT handler (which a sub-thread uses to tell
amiibrOS to exit).
* Fork the amiibo_scan.py subprocess.

The main process will then unblock signals, start the interface, and listen to
the scanner pipe.

Starting the interface spawns a new thread; stopping the interface (when
launching an app, for example) will join that thread to the main one.

Also, if the scanner app were to die prematurely, the SIGCHLD handler will know
and will tell amiibrOS to exit with an error. This is for debug reasons, as
amiibrOS's scanner app should never terminate while amiibrOS is running.

When a 4-byte character ID is read from the scanner pipe, the main process will
send a SIGTERM to any previous app sub-process. It will then wait to reap
before forking a new sub-process and executing the .sh file at
/usr/bin/amiibrOS/app/########/########.sh, where the #s denote an 8
character hex string corresponding to the first 4 bytes of the amiibo's
character ID (found at start of 15th block of the Amiibo's ntag213). Ex:
/usr/bin/amiibrOS/app/12345678/12345678

Because it is a .sh file, we allow the user to more easily write or download
their own programs and launch them with custom arguments (see
<project-root>/amiibrOS-overlay/usr/bin/amiibrOS/app/README.md for more info).
