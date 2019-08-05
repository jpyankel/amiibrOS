/**
 * fbscreenshot.h
 *
 * Various screenshot utilities for raylib directly using the framebuffer.
 *
 * Joseph Yankel (jpyankel@gmail.com)
 */

#include "raylib.h"

/**
 * Takes a screenshot by directly reading pixels from the framebuffer; this
 *   allows for screenshots without the OpenGL context to be active.
 * The screenshot is copied to memory and an image is generated from it.
 *
 * The image must be freed after via UnloadImage.
 */
Image takeFBScreenshot (void);
