/**
 * main.c
 *
 * Contains implementation of amiibrOS_interface. Features a cool logo and an
 *   indicator for amiibo NFC.
 * The indicator (referred to as touch indicator or TI), pulses, fading in and
 *   out and switches colors.
 * 
 * Joseph Yankel (jpyankel@gmail.com)
 */

#include <stdio.h> // sprintf
#include "raylib.h"

#define SCREEN_WIDTH 1440
#define SCREEN_HEIGHT 900

static const char *LOGO_PATH = "resources/logo.png";
// In order to center the logo we perform the calculation:
// left margin = (SCREEN_WIDTH - IMAGE_WIDTH)/2 = (1440-1276)/2 = 164/2 = 82.
#define LOGO_X 82
#define LOGO_Y 100 // This choice is arbitrary: It just looks good.

// Beginning of the path to each touch indicator we will cycle between:
#define TI_PREF_DEF "resources/touch_indicator"
// Length of touch indicator path (prefix len + id + ".png" including NUL):
#define TI_PATH_LEN sizeof(TI_PREF_DEF) + 6
// Number of touch indicators to cycle between. Note that this means TI_TEX_CNT
//   indicator .png files must exist with the above prefix.
#define TI_TEX_CNT 6
// TODO: These will need to be reconfigured when the physical build is
//   constructed.
#define TI_X 891
#define TI_Y 500

/**
 * Updates the touch indicator parameters given.
 * Assumes a constant 60 fps: This function is called around 60Hz
 * TODO: More documentation, create a constant that determines how long a pulse
 * takes
 */
/*void update_ti(void)
{

}*/

int main(void)
{
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "amiibrOS"); // Init OpenGL context
  
  SetTargetFPS(60);
  // Load logo and other images into GPU memory (must do after OpenGL context)
  Texture2D logo = LoadTexture(LOGO_PATH);
  Texture2D tis[TI_TEX_CNT];
  unsigned int current_ti; // The current touch indicator texture in the cycle
  // Pre-load all touch indicator textures:
  for (current_ti = 0; current_ti < TI_TEX_CNT; current_ti++) {
    char ti_path[TI_PATH_LEN]; // Calc'd once at compile-time.
    sprintf(ti_path, "%s%d.png", TI_PREF_DEF, current_ti);
    tis[current_ti] = LoadTexture(ti_path);
  }
  current_ti = 0; // Start the sequence from beginning.

  while (!WindowShouldClose()) { // while KEY_ESCAPE not yet pressed
    //update_ti(); // Calculate pulsing of alpha value and incr current_ti.

    BeginDrawing();
    
    ClearBackground(RAYWHITE);

    DrawTexture(logo, LOGO_X, LOGO_Y, WHITE); // Draw logo centered, no tint
    // Draw amiibo touch indicator with the alpha value we calculated and the
    //   current texture in the cycle:
    DrawTexture(tis[current_ti], TI_X, TI_Y, WHITE);//ti_color);

    EndDrawing();
  }
  
  // Unload all touch indicator textures:
  for (current_ti = 0; current_ti < TI_TEX_CNT; current_ti++) {
    UnloadTexture(tis[current_ti]);
  }

  UnloadTexture(logo);

  CloseWindow(); // Close OpenGL context

  return 0;
}
