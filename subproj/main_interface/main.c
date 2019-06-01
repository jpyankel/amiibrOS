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
#include <math.h> // sin
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

// Determines sine wave frequency (Hz) for TI light pulse animation.
#define TI_PULSE_FREQ 0.5f
const float TI_PULSE_PERIOD = 1/TI_PULSE_FREQ;

/**
 * Performs a simplified integer modulus (x mod y) for the two given floats.
 * Useful for wrapping an indefinitely incrementing float value back to 0.
 * Preconditions:
 * * y must be a strictly positive float (for this to make sense).
 */
float fwrap (float x, float y)
{
  return x - y * floor(x / y);
}

/**
 * Updates the touch indicator's alpha value and cycles its color.
 * Must be called at the beginning of every frame.
 * Does not assume a constant 60 fps: This function utilizes a deltaTime for
 *   robustness and can be called with a variable FPS.
 * Pulse Frequency is determined via TI_PULSE_FREQ.
 */
void update_ti(float *ti_alpha, unsigned int *current_ti)
{
  // Keeps track of when we cycle between colors:
  static bool visible_switch = true; // true at start avoids skipping 1st color
  // Keeps track of the sine wave we use to create a pulse effect:
  static float theta = 0.0f;
  
  // Essentially adding deltaTime since last frame:
  theta = fwrap(theta + GetFrameTime(), TI_PULSE_PERIOD);

  if ( (*ti_alpha = sin(theta * 2 * PI * TI_PULSE_FREQ) ) < 0.0f) {
    *ti_alpha = 0.0f; // Clamp to 0 if the sine wave goes below 0.
    visible_switch = false; // We cannot see the pulse.
  }
  else if (visible_switch == false && *ti_alpha > 0.0f) {
    // We went from not seeing the pulse to seeing it. We update the current_ti
    *current_ti = (*current_ti + 1) % TI_TEX_CNT;
    visible_switch = true;
  }
}

int main(void)
{
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "amiibrOS"); // Init OpenGL context
  
  SetTargetFPS(60);
  // Load logo and other images into GPU memory (must do after OpenGL context)
  Texture2D logo = LoadTexture(LOGO_PATH);

  Texture2D tis[TI_TEX_CNT];
  unsigned int current_ti; // The current touch indicator texture in the cycle
  float ti_alpha = 0.0f; // Alpha value of ti [0.0f, 1.0f]
  
  // Pre-load all touch indicator textures:
  for (current_ti = 0; current_ti < TI_TEX_CNT; current_ti++) {
    char ti_path[TI_PATH_LEN]; // Calc'd once at compile-time.
    sprintf(ti_path, "%s%d.png", TI_PREF_DEF, current_ti);
    tis[current_ti] = LoadTexture(ti_path);
  }
  current_ti = 0; // Start the sequence from beginning.

  while (!WindowShouldClose()) { // while KEY_ESCAPE not yet pressed
    update_ti(&ti_alpha, &current_ti); // Calculate alpha value & current_ti
    Color ti_color = Fade(WHITE, ti_alpha);

    BeginDrawing();
    
    ClearBackground(RAYWHITE);

    DrawTexture(logo, LOGO_X, LOGO_Y, WHITE); // Draw logo centered, no tint
    // Draw amiibo touch indicator with the alpha value we calculated and the
    //   current texture in the cycle:
    DrawTexture(tis[current_ti], TI_X, TI_Y, ti_color);

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
