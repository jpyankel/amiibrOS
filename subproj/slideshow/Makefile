# NOTE: Building requires amiibrOS-buildroot to have already built its output/
#   host/bin/arm-linux-gcc compiler as well as its output/target/usr/lib
#   directory.
# It must be run either:
#   1. Automatically, via amiibrOS-buildroot's make.
#   2. Manually after amiibrOS-buildroot's make. In this option, the compiled
#      executable has to be copied over to the Pi via ssh or other means.

.RECIPEPREFIX += 
.PHONY: all

# Define compiler flags:
#  -O1                  defines optimization level
#  -g                   include debug information on compilation
#  -s                   strip unnecessary data from build
#  -Wall                turns on most, but not all, compiler warnings
#  -std=c99             defines C language mode (standard C from 1999 revision)
#  -std=gnu99           defines C language mode (GNU C from 1999 revision)
#  -Wno-missing-braces  ignore invalid warning (GCC bug 53119)
#  -D_DEFAULT_SOURCE    use with -std=c99 on Linux and PLATFORM_WEB, required
#                       for timespec
BASE_CFLAGS = -O1 -s -Wall -std=c99 -D_DEFAULT_SOURCE -Wno-missing-braces \
  -Wextra -Wstrict-prototypes

BUILD_DIR = build

# === Linux ===a
CC_DEV = gcc

# For Linux, set library location to our installed packages (where raylib.so
#   lives):
DEV_CFLAGS = $(BASE_CFLAGS) -L/lib

# These are the libraries needed for linux version:
DEV_LIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -lc
# === ===

# === RPI ===
CC = ../../amiibrOS-buildroot/output/host/bin/arm-linux-gcc

CFLAGS = $(BASE_CFLAGS) -std=gnu99
CFLAGS += -L../../amiibrOS-buildroot/output/target/usr/lib
CFLAGS += -I../include
LIBS = -lraylib -lbrcmGLESv2 -lbrcmEGL -lpthread -lrt -lm -lbcm_host -ldl
# === ===

all: slideshow_DEV slideshow

# amiibrOS_DEV compiled for development and testing on Linux.
slideshow_DEV: slidestruct.h slidestruct.c main.c
  mkdir -p $(BUILD_DIR)
  cp -r resources $(BUILD_DIR) | true
  $(CC_DEV) $(DEV_CFLAGS) $(DEV_LIBS) -o $(BUILD_DIR)/slideshow_DEV\
		slidestruct.h slidestruct.c main.c

slideshow: slidestruct.h slidestruct.c main.c
  mkdir -p $(BUILD_DIR)
  cp -r resources $(BUILD_DIR) | true
  $(CC) $(CFLAGS) $(LIBS) -o $(BUILD_DIR)/slideshow slidestruct.h \
		slidestruct.c main.c
