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

#define INTERP_TYPE_MIN 0
#define INTERP_TYPE_MAX 10

// Type of interpolation
typedef enum interp_type
{
  ERROR = 0,
  NONE = 1,
  LINEAR = 2,
  SINE = 3,
  CIRCULAR = 4,
  CUBIC = 5,
  QUADRATIC = 6,
  EXPONENTIAL = 7,
  BACK = 8,
  BOUNCE = 9,
  ELASTIC = 10,
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

  struct imgstruct *next;
} imgstruct;

/**
 * Singly-linked-list entry representing one slide in the slideshow.
 */
typedef struct slidestruct
{
  char *title; // Changes corner text displayed when this slidestruct is chosen
  float title_duration; // How long before the title fades out in seconds
  float slide_duration; // slide display time in seconds
  imgstruct *images; // imagestruct linked list to be displayed on this slide

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

/**
 * Frees a given slidestruct, releasing its used memory.
 */
void slidestruct_free (slidestruct *ss);
