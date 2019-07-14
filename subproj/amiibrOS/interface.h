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

/**
 * Starts the UI window and runtime loop for the UI drawing and starts UI state
 *   from the main screen.
 *
 * Note, this function will block and must be started from a thread other than
 *   the main in order to prevent blocking of the system.
 * 
 * Also, this function takes a void* argument to satisfy pthreads. You can
 *   safely ignore it by passing NULL.
 */
void *start_interface (void*);

/**
 * Stops the UI drawing process and closes out the main window.
 * Call this function from the main thread to stop the interface on the next
 *   draw cycle.
 */
void stop_interface (void);
