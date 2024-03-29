/**
 * slidestruct.h
 *
 * Contains prototypes for the slidestruct object.
 *
 * The slidestruct is a singly-linked-list entry representing one slide in the
 *   slideshow. A slidestruct contains a linked list of imgstructs. Each
 *   imgstruct represents an image in that slide.
 *
 * Joseph Yankel (jpyankel@gmail.com)
 */

#include <stdbool.h>
#include "raylib.h"

#define INTERP_TYPE_MAX 9

#define INTERP_CAPTYPE_MAX 2

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

typedef enum interp_captype
{
  IN = 0,
  OUT = 1,
  INOUT = 2,
} interp_captype;

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
  interp_captype tint_interp_captype; // interp beginning/ending behaviour
  float tint_duration; // duration of the tint color transition in seconds
  
  Vector2 pos_i; // initial image position
  Vector2 pos_f; // final image position
  interp_type pos_interp; // image position interpolation type
  interp_captype pos_interp_captype;
  float pos_duration; // duration of the position transform in seconds
  
  Vector2 size_i; // initial image size
  Vector2 size_f; // final image size
  interp_type size_interp; // size interpolation type
  interp_captype size_interp_captype;
  float size_duration; // duration of the size change in seconds
  
  float rot_i; // initial image rotation
  float rot_f; // final image rotation
  interp_type rot_interp; // rotation interpolation type
  interp_captype rot_interp_captype;
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
 *
 * If an error occurs, info is printed to stdout. Otherwise, this function runs
 *   silently.
 */
slidestruct *slidestruct_read_conf (const char *path);

// Prints every parameter of every image in every slide from given slidestruct
void slidestruct_print(slidestruct *ss);

/**
 * Frees a given slidestruct, releasing its used memory.
 */
void slidestruct_free (slidestruct *ss);
