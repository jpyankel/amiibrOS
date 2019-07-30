"""
  amiibo_scan.py

  This is the scanner program for amiibrOS. It utilizes CircuitPython's PN532
    library to read from the PN532 Adafruit PN532 NFC/RFID Module found here:
    https://www.adafruit.com/product/364

  This program communicates to amiibrOS via a pre-established pipe found at
    argv[1].

  Joseph Yankel (jpyankel@gmail.com)
"""

import os
import sys
import board
import busio
from digitalio import DigitalInOut
from adafruit_pn532.spi import PN532_SPI

# Helpful constants:
CHAR_ID_BLOCK = 0x15

def main():
  # Initialize SPI connection:
  spi = busio.SPI(board.SCK, board.MOSI, board.MISO)
  cs_pin = DigitalInOut(board.CE0)
  pn532 = PN532_SPI(spi, cs_pin, debug=False)

  # Check to make sure PN532 Exists:
  pn532.get_firmware_version() # This call will throw if it cannot find PN532

  # Configure PN532 to communicate with MiFare cards
  pn532.SAM_configuration()

  # Enter scanning loop:
  while True:
    # Check if a card is available to read
    try:
      uid = pn532.read_passive_target(timeout=0.5)
    except RuntimeError:
      # This occurs when more than one card or incompatible card is detected.
      # Tell amiibrOS that the scanned card could not be identified:
      pass #TODO Remove and do as the above comment says
    
    # Try again if no card is available
    if uid is None:
      continue

    try:
      charID = pn532.ntag2xx_read_block(CHAR_ID_BLOCK)
    except TypeError:
      # A bug in PN532_SPI will try to subscript a NoneType when the tag read
      #   becomes garbled (usually because an amiibo was lifted off of the
      #   scanner)
      continue # If this happens, we just try again.

    # Try again if no ID is available
    if charID is None:
      continue

    # Note sys.argv[1] has the pipe's file descriptor if this program is called
    #   from amiibrOS.
    # Tell amiibrOS the hex charID we found:
    os.write(int(sys.argv[1]), charID)

if __name__ == "__main__":
  main()
