"""
  amiibo_scan.py

  A small program that tests amiibrOS's features by feeding it fake,
    pre-generated scans.

  Joseph Yankel (jpyankel@gmail.com)
"""

import os
import sys
import time

if __name__ == "__main__":
  testbytes1 = bytes.fromhex("01000000")
  testbytes2 = bytes.fromhex("00000000")
  testbytes3 = bytes.fromhex("01030000")
  print("PYTHON STARTED")
  print("ARGUMENTS: ", str(sys.argv), ". SLEEPING...")
  time.sleep(5)

  print("SLEEP COMPLETE. SCANNER WRITING 4 BYTES...")
  os.write(int(sys.argv[1]), testbytes1)
  print("WRITE COMPLETE. SCANNER SLEEPING...")
  time.sleep(5)
  
  print("SLEEP COMPLETE. SCANNER WRITING 4 BYTES...")
  os.write(int(sys.argv[1]), testbytes2)
  print("WRITE COMPLETE. SCANNER SLEEPING...")
  time.sleep(5)

  print("SLEEP COMPLETE. SCANNER WRITING 4 BYTES...")
  os.write(int(sys.argv[1]), testbytes3)
  print("WRITE COMPLETE. SCANNER SLEEPING...")
  time.sleep(10)
  
  #print("SLEEP COMPLETE. CHILD EXITING.")
  print("LOOPING FOREVER.")
  while True:
    pass
