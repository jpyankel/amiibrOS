#!/usr/bin/python

"""
  Spawned on boot via init.d to monitor monitor power switch GPIO in order to
    shutdown the RPI.

  Hold the power switch for SHUTDOWN_HOLD_TIME to turn RPI off. Press for less
    than that duration to reboot.

  Joseph Yankel (jpyankel@gmail.com)
"""

import threading, subprocess
import RPi.GPIO as GPIO

SHUTDOWN_HOLDTIME = 3.0 # Number of seconds we need to hold to shutdown
WAKE_PIN = 5 # GPIO Pin which we use for wake/shutdown functionality

# Mutex used to prevent both threads from executing shutdown/reboot at once:
shutdown_lock = threading.Lock()

"""
  Waits for GPIO WAKE_PIN to go LOW (shorted to GND) before timing this state
    and beginning the shutdown or reboot process.
"""
def main():
  # Use board pin numberings:
  GPIO.setmode(GPIO.BOARD)
	
  # Set wake pin to be an input pin pulled to high (3.3V):
  GPIO.setup(WAKE_PIN, GPIO.IN)
	
  # Wait for wake pin to be brought low:
  GPIO.wait_for_edge(WAKE_PIN, GPIO.FALLING)

  # Also start timer to perform shutdown after SHUTDOWN_HOLDTIME seconds:
  shutdown_timer = threading.Timer(SHUTDOWN_HOLDTIME, shutdown)
  shutdown_timer.start()

  # Wait until user releases switch:
  GPIO.wait_for_edge(WAKE_PIN, GPIO.RISING)

  # Button has been released, attempt to reboot (note that the timer thread
  #   will already have called shutdown if SHUTDOWN_HOLDTIME has passed):
  reboot()

def reboot():
  # Make sure this thread is the only one performing reboot/shutdown:
  shutdown_lock.acquire()
  # Best practice to perform cleanup before program termination:
  GPIO.cleanup()
  # Reboot:
  subprocess.call('reboot', shell=False)

def shutdown():
  # Make sure this thread is the only one performing reboot/shutdown:
  shutdown_lock.acquire()
  # Best practice to perform cleanup before program termination:
  GPIO.cleanup()
  # Shutdown:
  subprocess.call('poweroff', shell=False)

if __name__ == '__main__':
  main()
