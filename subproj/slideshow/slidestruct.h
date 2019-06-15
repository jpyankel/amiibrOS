/**
 * slidestruct.h
 *
 * Contains prototypes for the slidestruct object.
 *
 * The slidestruct is a singly-linked-list entry representing one slide in the
 *   slideshow. 
 *
 * Joseph Yankel (jpyankel@gmail.com)
 */

#include <stdbool.h>
#include "raylib.h"

// Type of interpolation
enum interp_type
{
  NONE = 0,
  LINEAR = 1,
  SINE = 2,
  CIRCULAR = 3,
  CUBIC = 4,
  QUADRATIC = 5,
  EXPONENTIAL = 6,
  BACK = 7,
  BOUNCE = 8,
  ELASTIC = 9,
};

/**
 * Singly-linked-list entry representing one slide in the slideshow.
 */
typedef struct slidestruct
{
  char *title; // Changes text displayed when this slidestruct is chosen
  char *img_name; // Path to the image to display
  float img_duration; // Image display time

  Color bgcolor_i; // Initial background color to display behind the image
  Color bgcolor_f; // Final background color to display behind the image
  Color tint_i; // initial tint color
  Color tint_f; // final tint color
  Vector2 pos_i; // initial position
  Vector2 pos_f; // final position
  Vector2 size_i; // initial size
  Vector2 size_f; // final size
  float rot_i; // initial rotation
  float rot_f; // final rotation

  interp_type bgcolor_interp; // bgcolor interpolation type
  interp_type tint_interp; // tint interpolation type
  interp_type pos_interp; // position interpolation type
  interp_type size_interp; // size interpolation type
  interp_type rot_interp; // rotation interpolation type

  float bgcolor_duration; // Duration of the background color transition
  float tint_duration; // duration of the tint color transition
  float pos_duration; // duration of the position transform
  float size_duration; // duration of the size change
  float rot_duration; // duration of the rotation
  
  struct slidestruct *next; // Next slidestruct in the list (can be NULL)
} slidestruct;

/**
 * Populates a newly created slidestruct with configuration info parsed from
 *   the text file found at path. Returns newly created slidestruct, or NULL on
 *   error.
 * 
 * Error Conditions:
 * * path is invalid
 * * the text data at path is malformed
 * * any malloc or open errors would occur
 */
slidestruct *slidestruct_read_conf (char *path);
