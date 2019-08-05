/**
 * interface.h
 *
 * Contains prototypes for amiibrOS's main interface. Most constants are
 *   implementation defined and are found instead in interface.c.
 *
 * This header is mostly for convenience in abstracting away the interface
 *   implementation details and providing handling functions for the main
 *   control program (main.c).
 * 
 * Joseph Yankel (jpyankel@gmail.com)
 */

#include <stdbool.h>
#include "raylib.h"

/**
 * Creates the main UI thread and begins the runtime loop for the UI drawing.
 * This thread will not respond to any blockable signals. These should be
 *   instead handled in the calling thread.
 *
 * Warning, all blockable signals will be temporarily blocked on the calling
 *   thread during this function call. Before the call returns the calling
 *   thread will have its blocked signals reverted to their original settings.
 *
 * Returns true if successful; false if an error occured and errno is set.
 */
bool start_interface (void);

/**
 * Joins the main UI thread after a short cleanup.
 *
 * Must be called only after a successful start_interface.
 */
bool stop_interface (void);

/**
 * Plays a scan success animation and fades out the UI.
 * It uses a background image of the previous screen behind the fade as a hack
 *   to get around using another framebuffer and RenderTextures to create the
 *   same effect.
 *
 * bg: bg must != NULL. bg is a screencapture of the state right before closing
 *   the last running program (or mainUI_thread).
 *
 * Warning, all blockable signals will be temporarily blocked on
 *   the calling thread during this function call. Before the call returns the
 *   calling thread will have its blocked signals reverted to their original
 *   settings.
 *
 * Note, this function will block until the animation is completed.
 * Note, this function will not free bg on its own. This must be done after the
 *   function call returns.
 * 
 * Returns true if successful; false if there were errors.
 */
bool play_scan_success_anim (Image *bg);

/**
 * Plays a scan failed animation.
 * It uses a background image of the previous screen behind the animation as a
 *   hack to get around using another framebuffer and RenderTextures to create
 *   the same effect.
 *
 * bg: bg must != NULL. bg is a screencapture of the state right before closing
 *   the last running program (or mainUI_thread).
 *
 * Warning, all blockable signals will be temporarily blocked on
 *   the calling thread during this function call. Before the call returns the
 *   calling thread will have its blocked signals reverted to their original
 *   settings.
 *
 * Note, this function will block until the animation is completed.
 * Note, this function will not free bg on its own. This must be done after the
 *   function call returns.
 * 
 * Returns true if successful; false if there were errors.
 */
bool play_scan_failed_anim (Image *bg);

// Returns whether or not the interface (mainUI_thread's interface) is active.
bool is_interface_active (void);
