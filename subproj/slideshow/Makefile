#
# amiibrOS-raylib Sub-Project Makefile
#
# NOTE: Building requires amiibrOS-buildroot to have already built its output/
#   host/bin/arm-linux-gcc compiler as well as its output/target/usr/lib
#   directory.
# It must be run either:
#   1. Automatically, via amiibrOS-buildroot's make.
#   2. Manually after amiibrOS-buildroot's make. In this option, the compiled
#      executable has to be copied over to the Pi via ssh or other means.
# To customize this makefile for another program, modify the following:
#   * NAME_<PLATFORM> - Modify to change the name of the output program for the
#     specified platform.
#   * SRC_<PLATFORM> - Modify to add or remove source files for the target
#     platform.
#
# Joseph Yankel (jpyankel@gmail.com)
#

.RECIPEPREFIX += 
.PHONY: all dev test clean

# Raylib compiler flags (taken from Raylib Examples):
#  -O1                  defines optimization level
#  -Wall                turns on most, but not all, compiler warnings
#  -std=c99             defines C language mode (standard C from 1999 revision)
#  -Wno-missing-braces  ignore invalid warning (GCC bug 53119)
#  -D_DEFAULT_SOURCE    use with -std=c99 on Linux and PLATFORM_WEB, required
#                       for timespec
BASE_CFLAGS = -O1 -Wall -std=c99 -D_DEFAULT_SOURCE -Wno-missing-braces \
  -Wextra -Wstrict-prototypes
BASE_CFLAGS += -I../include

BUILD_DIR = build
TEST_DIR = test

# === Linux ===a
CC_LINUX = gcc

# For Linux, set library location to our installed packages (where raylib.so
#   lives):
# -g (include debug information on compilation)
CFLAGS_LINUX = $(BASE_CFLAGS) -g
CFLAGS_LINUX += -L/lib

# These are the libraries needed for linux version:
LIBS_LINUX = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -lc

# Files included in compilation (order matters)
SRC_LINUX = slidestruct.h slidestruct_defaults.h slidestruct.c main.c
SRC_LINUX_TEST = slidestruct.h slidestruct_defaults.h slidestruct.c test.c

# Output file name
NAME_LINUX = slideshow_dev
NAME_LINUX_TEST = slideshow_test
# === ===

# === RPI ===
CC_RPI = ../../amiibrOS-buildroot/output/host/bin/arm-linux-gcc

# -s (strip unnecessary data from build)
# -std=gnu99 (defines C language mode (GNU C from 1999 revision))
CFLAGS_RPI = $(BASE_CFLAGS) -std=gnu99 -s
CFLAGS_RPI += -L../../amiibrOS-buildroot/output/target/usr/lib
LIBS_RPI = -lraylib -lbrcmGLESv2 -lbrcmEGL -lpthread -lrt -lm -lbcm_host -ldl

SRC_RPI = slidestruct.h slidestruct_defaults.h slidestruct.c main.c

NAME_RPI = slideshow
# === ===

all: $(NAME_LINUX) $(NAME_LINUX_TEST) $(NAME_RPI)

linux: $(NAME_LINUX)

rpi: $(NAME_RPI)

test: $(NAME_LINUX_TEST)

$(NAME_LINUX): $(SRC_LINUX)
  mkdir -p $(BUILD_DIR)
	# "| true" continues even if resources does not exist.
  cp -r resources $(BUILD_DIR) | true
  $(CC_LINUX) $(CFLAGS_LINUX) $(LIBS_LINUX) -o $(BUILD_DIR)/$(NAME_LINUX)\
		$(SRC_LINUX)

$(NAME_LINUX_TEST): $(SRC_LINUX_TEST)
  mkdir -p $(TEST_DIR)
  cp -r resources $(TEST_DIR) | true
  $(CC_LINUX) $(CFLAGS_LINUX) $(LIBS_LINUX) -o $(TEST_DIR)/$(NAME_LINUX_TEST)\
		$(SRC_LINUX_TEST)

$(NAME_RPI): $(SRC_RPI)
  mkdir -p $(BUILD_DIR)
  cp -r resources $(BUILD_DIR) | true
  $(CC_RPI) $(CFLAGS_RPI) $(LIBS_RPI) -o $(BUILD_DIR)/$(NAME_RPI) $(SRC_RPI)

clean: 
  rm -rf $(BUILD_DIR) | true # Clean build dir before starting
