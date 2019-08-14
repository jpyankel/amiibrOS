/**
 * main.c
 *
 * Reads the config file specified by a constant living in the same directory
 *   to produce a slidestruct - a specification for how the slideshow will run.
 *
 * We then loop through the slidestruct, displaying the specified images and
 *   playing their animations via Raylib.
 *
 * Joseph Yankel (jpyankel@gmail.com)
 */

#include <stdlib.h>
#include <string.h>
#include "slidestruct.h"
#include "raylib.h"
#include "easings.h"

#define SCREEN_WIDTH 1440
#define SCREEN_HEIGHT 900

#define CONF_PATH "resources/config.txt"
#define RES_PATH "resources/"
#define RES_PATH_SIZE sizeof(RES_PATH)

// === Function Prototypes ===
Texture2D *load_slide_textures(slidestruct *current_slide,
                               size_t *textures_len);
void unload_slide_textures (Texture2D *textures, size_t textures_len);
void interp_pos (imgstruct *opts, Rectangle *destRec, float timeElapsed);
void interp_size (imgstruct *opts, Rectangle *destRec, float timeElapsed);
void interp_rot (imgstruct *opts, float *rot, float timeElapsed);
void interp_tint (imgstruct *opts, Color *color, float timeElapsed);
// ===========================

int main (void)
{
  // Read slidestruct 
  slidestruct *ss = slidestruct_read_conf(CONF_PATH);
  if (ss == NULL)
    return 1; // We failed to read slidestruct TODO throw error message???
  
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "slideshow"); // Init OpenGL context
  
  SetTargetFPS(60);
  
  slidestruct *current_slide = ss;
  size_t textures_len; // Size of the textures array
  // Load the necessary images for this first slide:
  Texture2D *textures = load_slide_textures(current_slide, &textures_len);
  double slide_start = GetTime();

  while (!WindowShouldClose()) {
    double timeElapsed = GetTime() - slide_start;
    
    BeginDrawing();
    
    ClearBackground(BLACK);
    
    size_t texture_idx = 0; // Current texture we are selecting in the array.
    // Loop through all images, update animatable properties, and draw.
    /**
     * Notice that the order of textures and imgstructs is sorted such that
     *   the texture at a given index corresponds to the imgstruct 'index away
     *   from' the head of the imgstruct linked-list.
     */
    for (imgstruct *opts = current_slide->images; opts != NULL;
         opts = opts->next) {

      Texture2D texture = textures[texture_idx];
      // Take the entire srcRec by default: (TODO Make this animatable)
      Rectangle srcRec = (Rectangle){0, 0, texture.width, texture.height};
      Rectangle destRec;
      float rot;
      Color tint;

      interp_pos(opts, &destRec, (float)timeElapsed); // Interpolate position
      interp_size(opts, &destRec, (float)timeElapsed); // Interpolate size
      interp_rot(opts, &rot, (float)timeElapsed); // Interpolate rotation
      interp_tint(opts, &tint, (float)timeElapsed); // Interpolate tint color

      // Draw:
      // Treat origin as centered: TODO Possible option per image!!!
      Vector2 origin = {destRec.width / 2, destRec.height / 2};
      DrawTexturePro(texture, srcRec, destRec, origin, rot, tint);

      if ( (++texture_idx) >= textures_len)
        texture_idx = 0; // Reset the texture count, we looped through all imgs
    }
    
    // TODO Draw title text and stuff if applicable

    EndDrawing();

    // Check if time has elasped for the slide:
    if (timeElapsed >= current_slide->slide_duration) {
      unload_slide_textures(textures, textures_len); // Unload old textures
      
      if ( (current_slide = current_slide->next) == NULL) {
        // Loops back to the first slide if we reach the end.
        current_slide = ss;
      }
      
      // Update to new images:
      textures = load_slide_textures(current_slide, &textures_len);

      slide_start = GetTime(); // Reset timer
    }
  }

  CloseWindow(); // Close OpenGL context
  
  slidestruct_free(ss); // Free slidestruct
  return 0;
}

/**
 * Creates an array of Texture2D structure based on the slidestruct's images
 *   linked-list.
 * Entries are ordered such that they match pairwise in index with the given
 *   slidestruct's images.
 * The argument textures_len will be populated (if non-null) with the length
 *   found during the loading.
 */
Texture2D *load_slide_textures(slidestruct *current_slide,
                               size_t *textures_len)
{
  // Determine size of needed array:
  size_t cnt = 0;
  for (imgstruct *opts = current_slide->images; opts != NULL;
       opts = opts->next) {
    cnt++;
  }
  if (textures_len != NULL) *textures_len = cnt;
  
  // Allocate an array with the determined size:
  Texture2D *textures = malloc(sizeof(Texture2D)*cnt);
  if (textures == NULL)
    return NULL; // TODO Not sure what to change this to yet...
  
  // Fill this array with loaded textures:
  cnt = 0;
  for (imgstruct *opts = current_slide->images; opts != NULL;
       opts = opts->next) {
    // Find out the size of the texture's path:
    size_t name_size = strlen(opts->img_name);
    size_t path_size = RES_PATH_SIZE + name_size; // RES_PATH_SIZE includes NUL
    char img_path[path_size]; // Create a buffer of path_size in length
    // Construct the image path:
    strcpy(img_path, RES_PATH);
    strcat(img_path, opts->img_name);
    // Load the image and move on to the next:
    textures[cnt] = LoadTexture(img_path);
    cnt++;
  }
  
  return textures;
}

// Unloads all textures in array 'textures' and frees the array.
void unload_slide_textures (Texture2D *textures, size_t textures_len)
{
  for (size_t idx = 0; idx < textures_len; idx++) {
    UnloadTexture(textures[idx]);
  }
  free(textures);
}

/**
 * Interpolates the position variables of the given destRec rectangle according
 *   to the method specified by the interp_type and interp_captype in the given
 *   imgstruct 'opts'.
 */
void interp_pos (imgstruct *opts, Rectangle *destRec, float timeElapsed)
{
  // First, search for the interp_func based on user preferences (opt)
  float (*interp_func)(float, float, float, float) = NULL;
  switch (opts->pos_interp) {
    case NONE:
      destRec->x = opts->pos_i.x;
      destRec->y = opts->pos_i.y;
      break;
    case LINEAR:
      switch (opts->pos_interp_captype) {
        case IN: interp_func = &EaseLinearIn; break;
        case OUT: interp_func = &EaseLinearOut; break;
        case INOUT: interp_func = &EaseLinearInOut; break;
      }
      break;
    case SINE:
      switch (opts->pos_interp_captype) {
        case IN: interp_func = &EaseSineIn; break;
        case OUT: interp_func = &EaseSineOut; break;
        case INOUT: interp_func = &EaseSineInOut; break;
      }
      break;
    case CIRCULAR:
      switch (opts->pos_interp_captype) {
        case IN: interp_func = &EaseCircIn; break;
        case OUT: interp_func = &EaseCircOut; break;
        case INOUT: interp_func = &EaseCircInOut; break;
      }
      break;
    case CUBIC:
      switch (opts->pos_interp_captype) {
        case IN: interp_func = &EaseCubicIn; break;
        case OUT: interp_func = &EaseCubicOut; break;
        case INOUT: interp_func = &EaseCubicInOut; break;
      }
      break;
    case QUADRATIC:
      switch (opts->pos_interp_captype) {
        case IN: interp_func = &EaseQuadIn; break;
        case OUT: interp_func = &EaseQuadOut; break;
        case INOUT: interp_func = &EaseQuadInOut; break;
      }
      break;
    case EXPONENTIAL:
      switch (opts->pos_interp_captype) {
        case IN: interp_func = &EaseExpoIn; break;
        case OUT: interp_func = &EaseExpoOut; break;
        case INOUT: interp_func = &EaseExpoInOut; break;
      }
      break;
    case BACK:
      switch (opts->pos_interp_captype) {
        case IN: interp_func = &EaseBackIn; break;
        case OUT: interp_func = &EaseBackOut; break;
        case INOUT: interp_func = &EaseBackInOut; break;
      }
      break;
    case BOUNCE:
      switch (opts->pos_interp_captype) {
        case IN: interp_func = &EaseBounceIn; break;
        case OUT: interp_func = &EaseBounceOut; break;
        case INOUT: interp_func = &EaseBounceInOut; break;
      }
      break;
    case ELASTIC:
      switch (opts->pos_interp_captype) {
        case IN: interp_func = &EaseElasticIn; break;
        case OUT: interp_func = &EaseElasticOut; break;
        case INOUT: interp_func = &EaseElasticInOut; break;
      }
      break;
  }
  if (interp_func != NULL) { // If we found an interp function...
    // ...use the function to interpolate the target values:
    destRec->x = (*interp_func)(timeElapsed, opts->pos_i.x, opts->pos_f.x,
                                opts->pos_duration);
    destRec->y = (*interp_func)(timeElapsed, opts->pos_i.y,
                                opts->pos_f.y, opts->pos_duration);
  } // Else, we leave it alone (because the user picked NONE).
}

/**
 * Interpolates the size variables of the given destRec rectangle according to
 *   the method specified by the interp_type and interp_captype given.
 */
void interp_size (imgstruct *opts, Rectangle *destRec, float timeElapsed)
{
  // First, search for the interp_func based on user preferences (opt)
  float (*interp_func)(float, float, float, float) = NULL;
  switch (opts->size_interp) {
    case NONE:
      destRec->width = opts->size_i.x;
      destRec->height = opts->size_i.y;
      break;
    case LINEAR:
      switch (opts->size_interp_captype) {
        case IN: interp_func = &EaseLinearIn; break;
        case OUT: interp_func = &EaseLinearOut; break;
        case INOUT: interp_func = &EaseLinearInOut; break;
      }
      break;
    case SINE:
      switch (opts->size_interp_captype) {
        case IN: interp_func = &EaseSineIn; break;
        case OUT: interp_func = &EaseSineOut; break;
        case INOUT: interp_func = &EaseSineInOut; break;
      }
      break;
    case CIRCULAR:
      switch (opts->size_interp_captype) {
        case IN: interp_func = &EaseCircIn; break;
        case OUT: interp_func = &EaseCircOut; break;
        case INOUT: interp_func = &EaseCircInOut; break;
      }
      break;
    case CUBIC:
      switch (opts->size_interp_captype) {
        case IN: interp_func = &EaseCubicIn; break;
        case OUT: interp_func = &EaseCubicOut; break;
        case INOUT: interp_func = &EaseCubicInOut; break;
      }
      break;
    case QUADRATIC:
      switch (opts->size_interp_captype) {
        case IN: interp_func = &EaseQuadIn; break;
        case OUT: interp_func = &EaseQuadOut; break;
        case INOUT: interp_func = &EaseQuadInOut; break;
      }
      break;
    case EXPONENTIAL:
      switch (opts->size_interp_captype) {
        case IN: interp_func = &EaseExpoIn; break;
        case OUT: interp_func = &EaseExpoOut; break;
        case INOUT: interp_func = &EaseExpoInOut; break;
      }
      break;
    case BACK:
      switch (opts->size_interp_captype) {
        case IN: interp_func = &EaseBackIn; break;
        case OUT: interp_func = &EaseBackOut; break;
        case INOUT: interp_func = &EaseBackInOut; break;
      }
      break;
    case BOUNCE:
      switch (opts->size_interp_captype) {
        case IN: interp_func = &EaseBounceIn; break;
        case OUT: interp_func = &EaseBounceOut; break;
        case INOUT: interp_func = &EaseBounceInOut; break;
      }
      break;
    case ELASTIC:
      switch (opts->size_interp_captype) {
        case IN: interp_func = &EaseElasticIn; break;
        case OUT: interp_func = &EaseElasticOut; break;
        case INOUT: interp_func = &EaseElasticInOut; break;
      }
      break;
  }
  if (interp_func != NULL) { // If we found an interp function...
    // ...use the function to interpolate the target values:
    destRec->width = (*interp_func)(timeElapsed, opts->size_i.x,
                                    opts->size_f.x, opts->size_duration);
    destRec->height = (*interp_func)(timeElapsed, opts->size_i.y,
                                     opts->size_f.y, opts->size_duration);
  } // Else, we leave it alone (because the user picked NONE).
}

/**
 * Interpolates the rotation variable given according to the method specified
 *   by opts.
 */
void interp_rot (imgstruct *opts, float *rot, float timeElapsed)
{
  // First, search for the interp_func based on user preferences (opt)
  float (*interp_func)(float, float, float, float) = NULL;
  switch (opts->rot_interp) {
    case NONE:
      *rot = opts->rot_i;
      break;
    case LINEAR:
      switch (opts->rot_interp_captype) {
        case IN: interp_func = &EaseLinearIn; break;
        case OUT: interp_func = &EaseLinearOut; break;
        case INOUT: interp_func = &EaseLinearInOut; break;
      }
      break;
    case SINE:
      switch (opts->rot_interp_captype) {
        case IN: interp_func = &EaseSineIn; break;
        case OUT: interp_func = &EaseSineOut; break;
        case INOUT: interp_func = &EaseSineInOut; break;
      }
      break;
    case CIRCULAR:
      switch (opts->rot_interp_captype) {
        case IN: interp_func = &EaseCircIn; break;
        case OUT: interp_func = &EaseCircOut; break;
        case INOUT: interp_func = &EaseCircInOut; break;
      }
      break;
    case CUBIC:
      switch (opts->rot_interp_captype) {
        case IN: interp_func = &EaseCubicIn; break;
        case OUT: interp_func = &EaseCubicOut; break;
        case INOUT: interp_func = &EaseCubicInOut; break;
      }
      break;
    case QUADRATIC:
      switch (opts->rot_interp_captype) {
        case IN: interp_func = &EaseQuadIn; break;
        case OUT: interp_func = &EaseQuadOut; break;
        case INOUT: interp_func = &EaseQuadInOut; break;
      }
      break;
    case EXPONENTIAL:
      switch (opts->rot_interp_captype) {
        case IN: interp_func = &EaseExpoIn; break;
        case OUT: interp_func = &EaseExpoOut; break;
        case INOUT: interp_func = &EaseExpoInOut; break;
      }
      break;
    case BACK:
      switch (opts->rot_interp_captype) {
        case IN: interp_func = &EaseBackIn; break;
        case OUT: interp_func = &EaseBackOut; break;
        case INOUT: interp_func = &EaseBackInOut; break;
      }
      break;
    case BOUNCE:
      switch (opts->rot_interp_captype) {
        case IN: interp_func = &EaseBounceIn; break;
        case OUT: interp_func = &EaseBounceOut; break;
        case INOUT: interp_func = &EaseBounceInOut; break;
      }
      break;
    case ELASTIC:
      switch (opts->rot_interp_captype) {
        case IN: interp_func = &EaseElasticIn; break;
        case OUT: interp_func = &EaseElasticOut; break;
        case INOUT: interp_func = &EaseElasticInOut; break;
      }
      break;
  }
  if (interp_func != NULL) { // If we found an interp function...
    // ...use the function to interpolate the target values:
    *rot = (*interp_func)(timeElapsed, opts->rot_i, opts->rot_f,
                          opts->rot_duration);
  } // Else, we leave it alone (because the user picked NONE).
}

/**
 * Interpolates the tint color variable given according to the method specified
 *   by opts.
 */
void interp_tint (imgstruct *opts, Color *color, float timeElapsed)
{
  // First, search for the interp_func based on user preferences (opt)
  float (*interp_func)(float, float, float, float) = NULL;
  switch (opts->tint_interp) {
    case NONE:
      *color = opts->tint_i;
      break;
    case LINEAR:
      switch (opts->tint_interp_captype) {
        case IN: interp_func = &EaseLinearIn; break;
        case OUT: interp_func = &EaseLinearOut; break;
        case INOUT: interp_func = &EaseLinearInOut; break;
      }
      break;
    case SINE:
      switch (opts->tint_interp_captype) {
        case IN: interp_func = &EaseSineIn; break;
        case OUT: interp_func = &EaseSineOut; break;
        case INOUT: interp_func = &EaseSineInOut; break;
      }
      break;
    case CIRCULAR:
      switch (opts->tint_interp_captype) {
        case IN: interp_func = &EaseCircIn; break;
        case OUT: interp_func = &EaseCircOut; break;
        case INOUT: interp_func = &EaseCircInOut; break;
      }
      break;
    case CUBIC:
      switch (opts->tint_interp_captype) {
        case IN: interp_func = &EaseCubicIn; break;
        case OUT: interp_func = &EaseCubicOut; break;
        case INOUT: interp_func = &EaseCubicInOut; break;
      }
      break;
    case QUADRATIC:
      switch (opts->tint_interp_captype) {
        case IN: interp_func = &EaseQuadIn; break;
        case OUT: interp_func = &EaseQuadOut; break;
        case INOUT: interp_func = &EaseQuadInOut; break;
      }
      break;
    case EXPONENTIAL:
      switch (opts->tint_interp_captype) {
        case IN: interp_func = &EaseExpoIn; break;
        case OUT: interp_func = &EaseExpoOut; break;
        case INOUT: interp_func = &EaseExpoInOut; break;
      }
      break;
    case BACK:
      switch (opts->tint_interp_captype) {
        case IN: interp_func = &EaseBackIn; break;
        case OUT: interp_func = &EaseBackOut; break;
        case INOUT: interp_func = &EaseBackInOut; break;
      }
      break;
    case BOUNCE:
      switch (opts->tint_interp_captype) {
        case IN: interp_func = &EaseBounceIn; break;
        case OUT: interp_func = &EaseBounceOut; break;
        case INOUT: interp_func = &EaseBounceInOut; break;
      }
      break;
    case ELASTIC:
      switch (opts->tint_interp_captype) {
        case IN: interp_func = &EaseElasticIn; break;
        case OUT: interp_func = &EaseElasticOut; break;
        case INOUT: interp_func = &EaseElasticInOut; break;
      }
      break;
  }
  if (interp_func != NULL) { // If we found an interp function...
    // ...use the function to interpolate the target values:
    color->r = (*interp_func)(timeElapsed, opts->tint_i.r, opts->tint_f.r,
                              opts->tint_duration);
    color->g = (*interp_func)(timeElapsed, opts->tint_i.g, opts->tint_f.g,
                              opts->tint_duration);
    color->b = (*interp_func)(timeElapsed, opts->tint_i.b, opts->tint_f.b,
                              opts->tint_duration);
    color->a = (*interp_func)(timeElapsed, opts->tint_i.a, opts->tint_f.a,
                              opts->tint_duration);
  } // Else, we leave it alone (because the user picked NONE).
}
