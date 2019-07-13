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
#include "slidestruct.h"
#include "raylib.h"
#include "easings.h"

#define SCREEN_WIDTH 1440
#define SCREEN_HEIGHT 900

// === Function Prototypes ===
Texture2D *load_slide_textures(slidestruct *current_slide,
                               size_t *textures_len);
void unload_slide_textures (Texture2D *textures, size_t textures_len);
// ===========================

int main (void) {
  // Read slidestruct 
  slidestruct *ss = slidestruct_read_conf("conf.txt");
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

      // Interpolate position:
      switch (opts->pos_interp) {
        case NONE:
          destRec.x = opts->pos_i.x;
          destRec.y = opts->pos_i.y;
          break;
        case LINEAR:
          destRec.x = EaseLinearInOut(timeElapsed, opts->pos_i.x,
                                      opts->pos_f.x, opts->pos_duration);
          destRec.y = EaseLinearInOut(timeElapsed, opts->pos_i.y,
                                      opts->pos_f.y, opts->pos_duration);
          break;
        // TODO Other cases
        default: break;
      }
      // Interpolate size:
      switch (opts->size_interp) {
        case NONE:
          destRec.width = opts->size_i.x;
          destRec.height = opts->size_i.y;
          break;
        case LINEAR:
          destRec.width = EaseLinearInOut(timeElapsed, opts->size_i.x,
                                          opts->size_f.x, opts->size_duration);
          destRec.height = EaseLinearInOut(timeElapsed, opts->size_i.y,
                                           opts->size_f.y,
                                           opts->size_duration);
          break;
        // TODO Other cases
        default: break;
      }
      // Interpolate rotation:
      rot = 0; // TODO Implement for real
      // Interpolate tint:
      // TODO Also avoid calculating anything not useful!
      tint = WHITE; // TODO Implement for real

      // Draw:
      // Treat origin as centered: TODO Possible option per image!!!
      Vector2 origin = {destRec.width / 2, destRec.height / 2};
      DrawTexturePro(texture, srcRec, destRec, origin, rot, tint);
    }
    
    // TODO Draw title text and stuff if applicable

    EndDrawing();

    // Check if time has elasped for the slide:
    if (timeElapsed >= current_slide->slide_duration) {
      unload_slide_textures(textures, textures_len); // Unload old textures
      texture_idx++;
      
      if ( (current_slide = current_slide->next) == NULL) {
        // Loops back to the first slide if we reach the end.
        current_slide = ss;
        texture_idx = 0; // Reset the texture count
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
    textures[cnt] = LoadTexture(opts->img_name);
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
