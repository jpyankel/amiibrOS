/**
 * slidestruct.h
 *
 * Contains prototypes for the slidestruct object.
 *
 * The slidestruct is a singly-linked-list entry representing one slide in the
 *   slideshow. One slidestruct contains multiple imgstructs. Each imgstruct
 *   represents an image in that slide.
 *
 * Joseph Yankel (jpyankel@gmail.com)
 */

#include <stdbool.h>
#include "raylib.h"

// Type of interpolation
typedef enum interp_type
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
} interp_type;

/**
 * Container for image information. A slidestruct contains an array of these so
 *   that there can be multiple images with different animations on a single
 *   slide.
 */
typedef struct imgstruct
{
  char *img_name; // Path (relative to resources) to the image

  Color tint_i; // initial image tint color
  Color tint_f; // final image tint color
  interp_type tint_interp; // image tint interpolation type
  float tint_duration; // duration of the tint color transition in seconds
  
  Vector2 pos_i; // initial image position
  Vector2 pos_f; // final image position
  interp_type pos_interp; // image position interpolation type
  float pos_duration; // duration of the position transform in seconds
  
  Vector2 size_i; // initial image size
  Vector2 size_f; // final image size
  interp_type size_interp; // size interpolation type
  float size_duration; // duration of the size change in seconds
  
  float rot_i; // initial image rotation
  float rot_f; // final image rotation
  interp_type rot_interp; // rotation interpolation type
  float rot_duration; // duration of the rotation in seconds
} imgstruct;

/**
 * Singly-linked-list entry representing one slide in the slideshow.
 */
typedef struct slidestruct
{
  char *title; // Changes corner text displayed when this slidestruct is chosen
  float title_duration; // How long before the title fades out in seconds
  float slide_duration; // slide display time in seconds
  imgstruct **images; // All imagestructs to be displayed on this slide

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
slidestruct *slidestruct_read_conf (const char *path);
