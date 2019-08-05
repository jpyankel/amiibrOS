/**
 * fbscreenshot.c
 *
 * Implementation file for fbscreenshot.h
 *
 * Joseph Yankel (jpyankel@gmail.com)
 */

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include "fbscreenshot.h"

#define FBPATH "/dev/fb0"
// BITS_PER_BYTE = 8
#define BITS_PER_BYTE_SHIFT 3

// Prints error via perror then exits.
void fatal_error (const char *msg)
{
  perror(msg);
  exit(1);
}

Image takeFBScreenshot (void)
{
  int fbfd; // Frame buffer file descriptor
  struct fb_fix_screeninfo finfo; // Fixed frame buffer info
	struct fb_var_screeninfo vinfo; // Variable frame buffer info

  // Populate info structs:
  if ( (fbfd = open(FBPATH, O_RDONLY)) == -1) // Open frame buffer as read-only
    fatal_error("takeFBScreenshot failed to open frame buffer");

  if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo) == -1) // Get fixed info
    fatal_error("takeFBScreenshot failed to populate finfo");

  if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo) == -1) // Get variable info
    fatal_error("takeFBScreenshot failed to populate vinfo");

  // Retrieve info that we need to copy all pixel values:
  unsigned int width = vinfo.xres; // Visible width
  unsigned int height = vinfo.yres; // Visible height
  unsigned int Bpp = vinfo.bits_per_pixel >> BITS_PER_BYTE_SHIFT;// Pixel bytes

  if (Bpp != 4) {
    printf("takeFBScreenshot failed: Frame buffer pixel format unsupported\n");
    exit(1);
  }

  unsigned int fb_len = finfo.smem_len; // Frame buffer total size (bytes)
  unsigned int fb_linelen = finfo.line_length; // Frame buffer bytes per line

  char *fb_data = (char*)mmap(NULL, fb_len, PROT_READ, MAP_SHARED, fbfd, 0);
  if (fb_data == MAP_FAILED)
    fatal_error("takeFBScreenshot failed to map frame buffer");

  // Copy image data from frame buffer:
  unsigned int *data_cpy;
  if ( (data_cpy = malloc(width * height * Bpp)) == NULL)
    fatal_error("takeFBScreenshot failed to allocate memory for screenshot");
  size_t data_cpy_idx = 0;
  for (unsigned int h = 0; h < height; h++) { // Loop through visible pixels
    for (unsigned int w = 0; w < width; w++) {
      char *color_ptr = fb_data + h * fb_linelen + w * Bpp; // Ptr to color
      unsigned int color = *((unsigned int*)color_ptr);
      color |= 0xFF000000; // Set alpha to 1
      data_cpy[data_cpy_idx] = color;
      data_cpy_idx++;
    }
  }
  
  // Close frame buffer and cleanup:
  if (close(fbfd) == -1)
    fatal_error("takeFBScreenshot failed to close frame buffer");
  
  if (munmap(fb_data, width * height * Bpp) == -1)
    fatal_error("takeFBScreenshot failed to unmap frame buffer");

  // Create raylib Image from copied image:
  Image image;
  image.data = data_cpy;
  image.width = width;
  image.height = height;
  image.mipmaps = 1;
  image.format = UNCOMPRESSED_R8G8B8A8;

  return image;
}
