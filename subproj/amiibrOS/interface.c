/**
 * interface.c
 *
 * Contains implementation of amiibrOS's main interface. Features a cool logo
 *   and an indicator for amiibo NFC.
 * The indicator (referred to as touch indicator or TI), pulses, fading in and
 *   out and switching colors.
 * 
 * Joseph Yankel (jpyankel@gmail.com)
 */

#include <stdio.h> // sprintf
#include <math.h> // sin fmod
#include <pthread.h> // pthread_cond_wait, ... etc.
#include <signal.h> // sigset_t, etc.
#include "easings.h"
#include "interface.h"

// === Logo Constants ===
#define SCREEN_WIDTH 1440
#define SCREEN_HEIGHT 900
static const char *LOGO_PATH = "resources/logo.png";
// In order to center the logo we perform the calculation:
// left margin = (SCREEN_WIDTH - IMAGE_WIDTH)/2 = (1440-1276)/2 = 164/2 = 82.
#define LOGO_X 82
#define LOGO_Y 100 // This choice is arbitrary: It just looks good.
// ======================

// === Touch Indicator Constants ===
// Beginning of the path to each touch indicator we will cycle between:
#define TI_PREF_DEF "resources/touch_indicator"
// Length of touch indicator path (prefix len + id + ".png" including NUL):
#define TI_PATH_LEN sizeof(TI_PREF_DEF) + 6
// Number of touch indicators to cycle between. Note that this means TI_TEX_CNT
//   indicator .png files must exist with the above prefix.
#define TI_TEX_CNT 6
// TODO: These will need to be reconfigured when the physical build is
//   constructed.
#define TI_X 960
#define TI_Y 700
#define TI_SIZE 378
// Determines sine wave frequency (Hz) for TI light pulse animation.
#define TI_PULSE_FREQ 0.5f
const float TI_PULSE_PERIOD = 1/TI_PULSE_FREQ;
// =================================

// === Success Indicator Constants ===
// Success indicator texture path:
#define SI_PATH "resources/success_indicator.png"
// Success indicator tint colors:
#define SI_TINT (Color){0, 255, 0, 255}
// Length (in seconds) of success animation:
#define SI_ANIM_LEN 1
#define SI_ANIM_SIZE_START TI_SIZE
#define SI_ANIM_SIZE_END 2512
// ===================================

// === Failure Indicator Constants ===
// Failure indicator texture path:
#define FI_PATH "resources/failure_indicator.png"
// Failure indicator flashing animation duration (in seconds):
#define FI_ANIM_LEN 1
// Failure indicator tint color
#define FI_TINT (Color){255, 0, 0, 255}
// # of times the indicator flashes in the duration FI_ANIM_LEN
#define FI_ANIM_FLSH_CNT 2
const double FI_ANIM_PERIOD = (double)FI_ANIM_LEN / (double)FI_ANIM_FLSH_CNT;
const double FI_ANIM_FREQ = 1.0/((double)FI_ANIM_LEN/(double)FI_ANIM_FLSH_CNT);
// ===================================

// === Fade Out Constants ===
#define FADEOUT_ANIM_LEN 1
// ==========================

// === Instructions Text Constants ===
#define INSTR_TEXT "Place amiibo stand against glow"
#define INSTR_X 50
#define INSTR_Y 675
#define INSTR_FONTSIZE 45
#define INSTR_COLOR DARKGRAY
// ===================================

// === Runtime Flags ===
// These flags are controllable by the host program via helper functions.
// They signal to the mainUI_thread, telling it to perform animations, etc..
volatile bool flag_stop = false; // Stops the mainUI_thread
volatile bool flag_scan_success_anim = false; // Starts scan_success animation
volatile bool flag_scan_fail_anim = false; // Starts scan failed animation
// =====================
// === Runtime Variables ===
double anim_start; // Current animation start time.
pthread_mutex_t flag_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t flag_cond = PTHREAD_COND_INITIALIZER;
pthread_t mainUI_thread; // Thread running the amiibrOS interface
bool mainUI_thread_active = false; // Whether or not mainUI_thread is running
// =========================

// === Function Prototypes ===
bool start_interface (void);
bool stop_interface (void);
bool play_scan_success_anim (void);
bool play_scan_fail_anim (void);
bool is_interface_active (void);

void *start_mainUI_thread (void* arg);

void anim_success_indicator (Texture2D *texture);
void anim_fail_indicator (Texture2D *texture);
void anim_fadeout (bool *flag_fade_anim);

float fwrap (float x, float y);
bool threadsafe_read_flag (volatile bool *flag, bool *state);
bool threadsafe_write_flag (volatile bool *flag, bool new_state);
bool threadsafe_read_mainUI_flags (bool *stop_val,
    bool *scan_success_val, bool *scan_fail_val);

void update_ti(float *ti_alpha, unsigned int *current_ti);
void draw_touch_indicator (Texture2D *texture, Color *tint);
// ===========================

// === interface.h Implementation ===
bool start_interface (void)
{
  // Temporarily block all signals so that child thread inherits a mask with
  //   all blockable signals blocked:
  sigset_t set;
  sigset_t oldset;
  sigfillset(&set);
  if (pthread_sigmask(SIG_SETMASK, &set, &oldset))
    return false;

  // Start a new thread for our main interface:
  if (pthread_create(&mainUI_thread, NULL, start_mainUI_thread, NULL))
    return false;
  mainUI_thread_active = true; // Keep track of state for later use

  // Unblock the signals:
  if (pthread_sigmask(SIG_SETMASK, &oldset, NULL))
    return false;

  return true; // Thread creation and signal mask handling was successful
}

bool stop_interface (void)
{
  printf("STOPPING INTERFACE\n");
  // Setting this flag will begin the unloading process on the next draw cycle.
  if (!threadsafe_write_flag(&flag_stop, true)) // Tell main ui thread to stop
    return false;
  
  // Wait for the mainUI_thread to finish cleanup before returning:
  if (pthread_join(mainUI_thread, NULL))
    return false;
  mainUI_thread_active = false; // Keep track of state for later use

  printf("STOPPING COMPLETE\n");
  return true; // Animation complete and thread rejoined.
}

bool play_scan_success_anim (void)
{
  printf("PLAYING SUCCESS ANIM\n");
  if (pthread_mutex_lock(&flag_mutex))
    return false;

  // Signal animation to start by setting animation flag:
  flag_scan_success_anim = true;

  // Wait for the flag change to signal us that animation completed:
  while (flag_scan_success_anim) {
    // Sleep main thread until pthread_cond_signal is called on UI thread.
    if (pthread_cond_wait (&flag_cond, &flag_mutex))
      return false;
  }

  if (pthread_mutex_unlock(&flag_mutex))
    return false;

  printf("SUCCESS ANIM COMPLETE\n");
  return true; // Animation finished successfully
}

bool play_scan_fail_anim (void)
{
  printf("PLAYING FAIL ANIM\n");
  if (pthread_mutex_lock(&flag_mutex))
    return false;

  // Signal animation to start by setting animation flag:
  flag_scan_fail_anim = true;

  // Wait for the flag change to signal us that animation completed:
  while (flag_scan_fail_anim) {
    // Sleep main thread until pthread_cond_signal is called on UI thread.
    if (pthread_cond_wait (&flag_cond, &flag_mutex))
      return false;
  }

  if (pthread_mutex_unlock(&flag_mutex))
    return false;

  printf("FAIL ANIM COMPLETE\n");
  return true; // Animation finished successfully
}

bool is_interface_active (void)
{
  return mainUI_thread_active;
}
// ==================================

// === Threading Functions ===
/**
 * Starts the UI window and runtime loop for the UI drawing and starts UI state
 *   from the main screen.
 *
 * Note, this function will block and must be started from a thread other than
 *   the main in order to prevent blocking of the system.
 *
 * Also, this function takes a void* argument to satisfy pthreads. You can
 *   safely ignore it by passing NULL.
 *
 * Warning, all signals must be blocked before calling this function. After the
 *   function call returns you may revert back to the previous settings.
 */
void *start_mainUI_thread (void* arg)
{
  // arg exists only to satisfy pthreads.
  (void)arg; // We tell compiler to ignore the fact that we never use arg.

  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "amiibrOS"); // Init OpenGL context
  
  SetTargetFPS(60);
  // Load logo and other images into GPU memory (must do after OpenGL context)
  Texture2D logo = LoadTexture(LOGO_PATH);
  Texture2D success_indicator = LoadTexture(SI_PATH);
  Texture2D fail_indicator = LoadTexture(FI_PATH);

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

  bool stop_val;
  bool scan_success_val;
  bool scan_fail_val;
  // Read in thread-shared values:
  // TODO Error check:
  threadsafe_read_mainUI_flags(&stop_val, &scan_success_val, &scan_fail_val);

  while (stop_val != true) {
    BeginDrawing();

    ClearBackground(WHITE);
    DrawTexture(logo, LOGO_X, LOGO_Y, WHITE); // Draw logo centered, no tint
    DrawText(INSTR_TEXT, INSTR_X, INSTR_Y, INSTR_FONTSIZE, INSTR_COLOR);
    update_ti(&ti_alpha, &current_ti); // Calculate alpha value & current_ti
    Color color = Fade(WHITE, ti_alpha);
    Texture2D texture = tis[current_ti];
    draw_touch_indicator(&texture, &color);

    if (scan_success_val) {
      if (anim_start == 0) // If the animation hasn't been started yet...
        anim_start = GetTime(); // ... start it from beginning!
      anim_success_indicator(&success_indicator);
    }
    else if (scan_fail_val) {
      if (anim_start == 0)
        anim_start = GetTime();
      anim_fail_indicator(&fail_indicator);
    }

    EndDrawing();

    // Read in thread-shared values so that loop guards gets updated values:
    // TODO Error check:
    threadsafe_read_mainUI_flags(&stop_val, &scan_success_val, &scan_fail_val);
  }

  printf("PLAYING FADE ANIM\n");
  // We received a signal to stop the interface: Next, play fade out animation:
  anim_start = GetTime();
  bool flag_fade_anim = true;
  while (flag_fade_anim) {
    BeginDrawing();
    ClearBackground(WHITE);

    anim_fadeout(&flag_fade_anim);

    EndDrawing();
  }
  printf("FADE ANIM COMPLETE\n");
  
  // Unload all touch indicator textures:
  for (current_ti = 0; current_ti < TI_TEX_CNT; current_ti++) {
    UnloadTexture(tis[current_ti]);
  }
  UnloadTexture(fail_indicator);
  UnloadTexture(success_indicator);
  UnloadTexture(logo);

  CloseWindow(); // Close OpenGL context

  threadsafe_write_flag(&flag_stop, false); // Reset flag: We have handled it.

  return NULL; // You can safely ignore this. This is just to satisfy pthreads.
}
// ===========================

// === Drawing Functions ===
/**
 * Updates the animatable values of the success indicator and draws it.
 * This function must be called between BeginDrawing/EndDrawing calls.
 *
 * Once the animation time runs out, the animation flag is reset.
 */
void anim_success_indicator (Texture2D *texture)
{
  // Update time:
  double time_elapsed = GetTime() - anim_start;
  if (time_elapsed > SI_ANIM_LEN) {
    // This is our last draw cycle:
    time_elapsed = SI_ANIM_LEN;
  }

  // Calculate updated values:
  double size = EaseLinearInOut(time_elapsed, SI_ANIM_SIZE_START,
                                SI_ANIM_SIZE_END, SI_ANIM_LEN);
  Rectangle srcRec = (Rectangle){0, 0, texture->width, texture->height};
  Rectangle destRec = (Rectangle){TI_X, TI_Y, size, size};
  Vector2 origin = {destRec.width / 2, destRec.height / 2};
  float rot = 0;
  Color tint = SI_TINT;

  // Draw the indicator
  DrawTexturePro(*texture, srcRec, destRec, origin, rot, tint);

  // Check to see if time ran out:
  if (time_elapsed == SI_ANIM_LEN) {
    anim_start = 0; // Reset animation start time to indicate no animation
    pthread_mutex_lock(&flag_mutex); // TODO Error checking
    
    flag_scan_success_anim = false; // Disable animation

    pthread_cond_signal(&flag_cond); // Wake main thread
    pthread_mutex_unlock(&flag_mutex); // TODO Error checking
  }
}

/**
 * Updates the animatable values of the failure indicator and draws it. This
 *   function must be called between BeginDrawing/EndDrawing calls.
 *
 * Once the animation time runs out, the animation flag is reset.
 */
void anim_fail_indicator (Texture2D *texture)
{
  // Update time:
  double time_elapsed = GetTime() - anim_start;
  if (time_elapsed > FI_ANIM_LEN) {
    // This is our last draw cycle:
    time_elapsed = FI_ANIM_LEN;
  }
  
  // Recalculate time-based variables:
  Rectangle srcRec = (Rectangle){0, 0, texture->width, texture->height};
  Rectangle destRec = (Rectangle){TI_X, TI_Y, SI_ANIM_SIZE_START,
                                  SI_ANIM_SIZE_START};
  Vector2 origin = {destRec.width / 2, destRec.height / 2};
  float rot = 0;
  Color tint = FI_TINT;
  int new_alpha = trunc(
    255 * sin(fwrap(time_elapsed, FI_ANIM_PERIOD) * 2 * PI * FI_ANIM_FREQ)
  );
  if (new_alpha < 0) { // Clamp to 0 (invisible) when sine goes negative.
    new_alpha = 0;
  }
  tint.a = (unsigned char)new_alpha;

  // Draw the indicator
  DrawTexturePro(*texture, srcRec, destRec, origin, rot, tint);
  
  // Check to see if time ran out:
  if (time_elapsed == FI_ANIM_LEN) {
    anim_start = 0; // Reset animation start time to indicate no animation
    pthread_mutex_lock(&flag_mutex); // TODO Error checking
    
    flag_scan_fail_anim = false; // Disable animation

    pthread_cond_signal(&flag_cond); // Wake main thread
    pthread_mutex_unlock(&flag_mutex); // TODO Error checking
  }
}

/**
 * Animates a screen fade out configurable by constants at the top of this
 *   file.
 *
 * This function must be called between BeginDrawing/EndDrawing calls.
 *
 * Resets the local flag (not shared between threads) flag_fade_anim on
 *   completion.
 */
void anim_fadeout (bool *flag_fade_anim)
{
  // Update time:
  double time_elapsed = GetTime() - anim_start;
  if (time_elapsed > FADEOUT_ANIM_LEN) {
    // This is our last draw cycle:
    time_elapsed = FADEOUT_ANIM_LEN;
  }

  // Calculate time-updated values:
  double alpha = EaseLinearInOut(time_elapsed, 0, 255, FADEOUT_ANIM_LEN);
  Color color = (Color){0, 0, 0, alpha};
  Rectangle destRec = (Rectangle){0, 0, GetScreenWidth(), GetScreenHeight()};

  // Draw the fadeout rectangle:
  DrawRectangleRec(destRec, color);

  // Check to see if time ran out:
  if (time_elapsed == FADEOUT_ANIM_LEN) {
    anim_start = 0; // Reset animation start time to indicate no animation
    *flag_fade_anim = false; // Animation is completed
  }
}
// =========================

// === Helpers ===
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
 * Thread safe function using mutexes to ensure only one thread is setting or
 *   reading flags at a time.
 * Sets the given bool to the state read from the given flag.
 *
 * Returns true if successful, false if an error occured.
 */
bool threadsafe_read_flag (volatile bool *flag, bool *state)
{
  // Ensure this thread is the only thread accessing the flags:
  if (pthread_mutex_lock(&flag_mutex))
    return false;

  *state = *flag;

  if (pthread_mutex_unlock(&flag_mutex)) // Let other threads access flags
    return false;

  return true;
}

/**
 * Thread safe function using mutexes to ensure only one thread is setting or
 *   reading flags at a time.
 * Sets the given flag to the given state.
 *
 * Returns true if successful, false if an error occured.
 */
bool threadsafe_write_flag (volatile bool *flag, bool new_state)
{
  // Ensure this thread is the only thread accessing the flags:
  if (pthread_mutex_lock(&flag_mutex))
    return false;

  *flag = new_state;

  if (pthread_mutex_unlock(&flag_mutex)) // Let other threads access flags
    return false;

  return true;
}

bool threadsafe_read_mainUI_flags (bool *stop_val,
    bool *scan_success_val, bool *scan_fail_val)
{
  // Ensure this thread is the only thread accessing the flags:
  if (pthread_mutex_lock(&flag_mutex))
    return false;

  *stop_val = flag_stop;
  *scan_success_val = flag_scan_success_anim;
  *scan_fail_val = flag_scan_fail_anim;

  if (pthread_mutex_unlock(&flag_mutex)) // Let other threads access flags
    return false;

  return true;
}
// ===========================

// TODO: Refactor these:
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

/**
 * Draws the touch indicator given its texture and a tint determining opacity.
 *
 * This function must be called between BeginDrawing/EndDrawing calls.
 */
void draw_touch_indicator (Texture2D *texture, Color *tint)
{
  // Draw amiibo touch indicator with the alpha value we calculated and the
  //   current texture in the cycle:
  Rectangle srcRec = (Rectangle){0, 0, texture->width, texture->height };
  Rectangle destRec = (Rectangle){TI_X, TI_Y, TI_SIZE, TI_SIZE};
  Vector2 origin = {destRec.width / 2, destRec.height / 2};
  float rot = 0;

  DrawTexturePro(*texture, srcRec, destRec, origin, rot, *tint);
}
