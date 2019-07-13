/**
 * slidestruct.c
 *
 * Contains implementation of slidestruct.h
 *
 * Joseph Yankel (jpyankel@gmail.com)
 */

#include <stdio.h> // File handling, fopen, getline, perror
#include <string.h> // strcat, strncpy, strchr
#include <stdlib.h> // malloc, NULL, strtof, strtoul
#include <ctype.h> // isspace
#include <errno.h> // Various error code consts.
#include <limits.h> // number type limits.
#include "slidestruct.h" // includes bool type
#include "slidestruct_defaults.h" 
// --- Helper Function Prototypes ---
slidestruct *construct_slidestruct (void);
imgstruct *construct_imgstruct (void);
bool is_whitespace_str (const char *str);
const char *first_non_whitespace_char (const char *str);
bool parse_float(const char *str, float *f);
bool parse_color(const char *str, Color *color);
bool parse_vector2 (const char *str, Vector2 *v);
bool parse_interp_type (const char *str, interp_type *type);
bool parse_interp_captype (const char *str, interp_captype *captype);

bool strtouc (unsigned char *c, const char *str, char **endptr, int base);
// --- ---

slidestruct *slidestruct_read_conf (const char *path)
{
  // Open file at path for reading:
  FILE *f;
  if ((f = fopen(path, "r")) == NULL) {
    perror("slidestruct read file open error");
    return NULL;
  }

  slidestruct *head_slidestruct = NULL; // The first slidestruct (returned)
  // The slidestruct that all further slide options will configure
  slidestruct *current_slidestruct = NULL;
  // The to-be head imagestruct (linked list) for the current_slidestruct.
  imgstruct *current_head_imgstruct = NULL; // Points to first of all imgstruct
  // The imgstruct that all further image options will configure
  imgstruct *current_imgstruct = NULL;

  char *linebuf = NULL; // Holds the line grabbed in each line read
  size_t buflen = 0; // Updated to hold the length of linebuf.
  ssize_t nchar; // Number of chars read by getline including '\n' but not NUL
  size_t lineno = 0; // Current line number in conf file. Used for error msg.

  char *prevlinebuf; // Holds the previous linebuf
  size_t prevlinebuflen; // Length of the previous linebuf.
  bool concat_next = false; // Whether to concat. next line to previous.

  // Read line by line, parsing each option and configuring slidestruct.
  while ((nchar = getline(&linebuf, &buflen, f)) != -1) {
    lineno++;
    // Ignore line if it is whitespace only or empty (unless it is appended via
    //   '\\'):
    if (!concat_next && is_whitespace_str(linebuf))
      continue;

    if (concat_next) {
      // Concatenate the current linebuf to the old prevlinebuf.
      char *newbuf;
      newbuf = malloc(prevlinebuflen + buflen - 1); // -1 for overwritten NUL
      strcpy(newbuf, prevlinebuf); // Copy old line to newbuf
      strcat(newbuf, linebuf); // Append current line to newbuf

      free(linebuf);
      linebuf = newbuf; // Point linebuf at our newly created buffer
      // We added a new string of prevlinebuf - 1 length (NUL not included),
      nchar += prevlinebuflen - 1; // so we add this many to nchar

      free(prevlinebuf);
      concat_next = false; // Reset flag
    }

    // Check to see if we should combine the current line and next line by '\\'
    if (nchar > 1 && linebuf[nchar - 2] == '\\'
        && linebuf[nchar - 1] == '\n') {
      // Store the previous line buf and next line buf.
      prevlinebuflen = nchar - 1; // -2 to remove '\\' and '\n', +1 for NUL

      prevlinebuf = malloc(prevlinebuflen);
      if (prevlinebuf == NULL) {
        perror("slidestruct read malloc error");
        return NULL;
      }

      // Copy all of the string up until '\\' and replace '\\' with NUL.
      strncpy(prevlinebuf, linebuf, prevlinebuflen);
      prevlinebuf[prevlinebuflen - 1] = '\0';
      
      // Set a flag to let the next loop know we should concat. the next
      //   line read onto the previous one.
      concat_next = true;

      continue; // Skip parsing and read next line.
    }

    /**
     * To isolate the option name, we search for the index of the first non
     *   whitespace char and the first space found after this char. The string
     *   created by the characters in between (including the first char but not
     *   including the ending space) is our option name.
     */

    const char *opt_start = first_non_whitespace_char(linebuf);
    // Check if the line is just whitespace.
    if (opt_start == NULL) // This can occur if multiple '\\' are misused.
      continue;

    const char *opt_end = strchr(opt_start, ' ');
    // If there are no spaces after opt_start, then there are no parameters.
    if (opt_end == NULL) {
      printf("slidestruct read error: option %s line %zu ended without"
          " settings\n", opt_start, lineno);

      return NULL; // We do not support options without parameters.
    }

    size_t opt_len = opt_end-opt_start;

    // Compare opt_len chars from opt_start to each of the possible options:
    if (!strncmp(opt_start, "title", opt_len)) {
      // Start a new slidestruct. This means that if a previous one exists, we
      //   must link its images array and set next ptr to the new slidestruct.
      slidestruct *new_ss = construct_slidestruct();
      if (new_ss == NULL)
        return NULL;

      if (current_slidestruct != NULL) {
        // Create links:
        current_slidestruct->images = current_head_imgstruct;
        current_slidestruct->next = new_ss;

        // When creating a new slidestruct, we must clear all stored imgstructs
        current_head_imgstruct = NULL;
        current_imgstruct = NULL;
      }
      else {
        // This is the first time we are constructing a slidestruct
        head_slidestruct = new_ss;
      }
      // The current slidestruct is the newly created one:
      current_slidestruct = new_ss;
      // Copy the setting (title) so that we can name the slidestruct:
      size_t title_len = strlen(opt_end+1); // +1 for NUL -1 for '\0'
      char *title = malloc(title_len);
      if (title == NULL) {
        perror("slidestruct read malloc error");
        return NULL;
      }
      strncpy(title, opt_end+1, title_len);
      title[title_len - 1] = '\0'; // Replace '\n' with '\0'
      current_slidestruct->title = title;
      
    }
    else if (!strncmp(opt_start, "title_duration", opt_len)) {
      if (current_slidestruct == NULL) {
        // The user has input a title-related option before the 'title' option.
        printf("slidestruct read error: option %s line %zu before a 'title'"
            " option\n", opt_start, lineno);
        return NULL;
      }
      
      // Parse setting as float and set the title_duration:
      float title_duration;
      if (!parse_float(opt_end+1, &title_duration)) {
        printf(" found option %s line %zu\n", opt_start, lineno);
        return NULL;
      }

      current_slidestruct->title_duration = title_duration;
    }
    else if (!strncmp(opt_start, "slide_duration", opt_len)) {
      if (current_slidestruct == NULL) {
        printf("slidestruct read error: option %s line %zu before a 'title'"
            " option\n", opt_start, lineno);
        return NULL;
      }

      float slide_duration;
      if (!parse_float(opt_end+1, &slide_duration)) {
        printf(" found option %s line %zu\n", opt_start, lineno);
        return NULL;
      }
      
      current_slidestruct->slide_duration = slide_duration;
    }
    else if (!strncmp(opt_start, "img_name", opt_len)) {
      if (current_slidestruct == NULL) {
        printf("slidestruct read error: option %s line %zu before a 'title'"
            " option\n", opt_start, lineno);
        return NULL;
      }

      // Start a new imgstruct. This means that if a previous one exists, we
      //   must set its next ptr to the new imgstruct.
      imgstruct *new_is = construct_imgstruct();
      if (new_is == NULL)
        return NULL;

      if (current_imgstruct != NULL) {
        // Create links:
        current_imgstruct->next = new_is;
      }
      else {
        // This is the first time we are constructing an imgstruct for this
        //   slidestruct
        current_head_imgstruct = new_is;
      }
      // The current imgstruct is the newly created one:
      current_imgstruct = new_is;
      // Copy the setting (img_name) so that we can set the resource path for
      //   the current image:
      size_t img_name_len = strlen(opt_end+1); // +1 for NUL -1 for '\n'
      char *img_name = malloc(img_name_len);
      if (img_name == NULL) {
        perror("slidestruct read malloc error");
        return NULL;
      }
      strncpy(img_name, opt_end+1, img_name_len);
      img_name[img_name_len - 1] = '\0'; // Replace '\n' with '\0'
      printf("%s\n", img_name);
      current_imgstruct->img_name = img_name;
    }
    else if (!strncmp(opt_start, "tint_i", opt_len)) {
      // Ensure that the user specifies an image before changing image
      //   properties
      if (current_imgstruct == NULL) {
        printf("slidestruct read error: option %s line %zu before an"
            " 'img_name' option\n", opt_start, lineno);
        return NULL;
      }

      Color color;
      if (!parse_color(opt_end+1, &color)) {
        printf(" found option %s line %zu\n", opt_start, lineno);
        return NULL;
      }

      current_imgstruct->tint_i = color;
    }
    else if (!strncmp(opt_start, "tint_f", opt_len)) {
      if (current_imgstruct == NULL) {
        printf("slidestruct read error: option %s line %zu before an"
               " 'img_name' option\n", opt_start, lineno);
        return NULL;
      }

      Color color;
      if (!parse_color(opt_end+1, &color)) {
        printf(" found option %s line %zu\n", opt_start, lineno);
        return NULL;
      }

      current_imgstruct->tint_f = color;
    }
    else if (!strncmp(opt_start, "tint_interp", opt_len)) {
      if (current_imgstruct == NULL) {
        printf("slidestruct read error: option %s line %zu before an"
            " 'img_name' option\n", opt_start, lineno);
        return NULL;
      }

      interp_type t;
      if (!parse_interp_type(opt_end+1, &t)) {
        printf(" option %s line %zu\n", opt_start, lineno);
        return NULL;
      }

      current_imgstruct->tint_interp = t;
    }
    else if (!strncmp(opt_start, "tint_interp_captype", opt_len)) {
      if (current_imgstruct == NULL) {
        printf("slidestruct read error: option %s line %zu before an"
            " 'img_name' option\n", opt_start, lineno);
        return NULL;
      }

      interp_captype t;
      if (!parse_interp_captype(opt_end+1, &t)) {
        printf(" option %s line %zu\n", opt_start, lineno);
        return NULL;
      }

      current_imgstruct->tint_interp_captype = t;
    }
    else if (!strncmp(opt_start, "tint_duration", opt_len)) {
      if (current_imgstruct == NULL) {
        printf("slidestruct read error: option %s line %zu before an"
               " 'img_name' option\n", opt_start, lineno);
        return NULL;
      }

      float tint_duration;
      if (!parse_float(opt_end+1, &tint_duration)) {
        printf(" found option %s line %zu\n", opt_start, lineno);
        return NULL;
      }
      
      current_imgstruct->tint_duration = tint_duration;
    }
    else if (!strncmp(opt_start, "pos_i", opt_len)) {
      if (current_imgstruct == NULL) {
        printf("slidestruct read error: option %s line %zu before an"
               " 'img_name' option\n", opt_start, lineno);
        return NULL;
      }

      Vector2 pos_i;
      if (!parse_vector2(opt_end+1, &pos_i)) {
        printf(" found option %s line %zu\n", opt_start, lineno);
        return NULL;
      }

      current_imgstruct->pos_i = pos_i;
    }
    else if (!strncmp(opt_start, "pos_f", opt_len)) {
      if (current_imgstruct == NULL) {
        printf("slidestruct read error: option %s line %zu before an"
               " 'img_name' option\n", opt_start, lineno);
        return NULL;
      }

      Vector2 pos_f;
      if (!parse_vector2(opt_end+1, &pos_f)) {
        printf(" found option %s line %zu\n", opt_start, lineno);
        return NULL;
      }

      current_imgstruct->pos_f = pos_f;
    }
    else if (!strncmp(opt_start, "pos_interp", opt_len)) {
      if (current_imgstruct == NULL) {
        printf("slidestruct read error: option %s line %zu before an"
            " 'img_name' option\n", opt_start, lineno);
        return NULL;
      }

      interp_type t;
      if (!parse_interp_type(opt_end+1, &t)) {
        printf(" option %s line %zu\n", opt_start, lineno);
        return NULL;
      }

      current_imgstruct->pos_interp = t;
    }
    else if (!strncmp(opt_start, "pos_interp_captype", opt_len)) {
      if (current_imgstruct == NULL) {
        printf("slidestruct read error: option %s line %zu before an"
            " 'img_name' option\n", opt_start, lineno);
        return NULL;
      }

      interp_captype t;
      if (!parse_interp_captype(opt_end+1, &t)) {
        printf(" option %s line %zu\n", opt_start, lineno);
        return NULL;
      }

      current_imgstruct->pos_interp_captype = t;
    }
    else if (!strncmp(opt_start, "pos_duration", opt_len)) {
      if (current_imgstruct == NULL) {
        printf("slidestruct read error: option %s line %zu before an"
               " 'img_name' option\n", opt_start, lineno);
        return NULL;
      }

      float pos_duration;
      if (!parse_float(opt_end+1, &pos_duration)) {
        printf(" found option %s line %zu\n", opt_start, lineno);
        return NULL;
      }
      
      current_imgstruct->pos_duration = pos_duration;
    }
    else if (!strncmp(opt_start, "size_i", opt_len)) {
      if (current_imgstruct == NULL) {
        printf("slidestruct read error: option %s line %zu before an"
               " 'img_name' option\n", opt_start, lineno);
        return NULL;
      }

      Vector2 size_i;
      if (!parse_vector2(opt_end+1, &size_i)) {
        printf(" found option %s line %zu\n", opt_start, lineno);
        return NULL;
      }

      current_imgstruct->size_i = size_i;
    }
    else if (!strncmp(opt_start, "size_f", opt_len)) {
      if (current_imgstruct == NULL) {
        printf("slidestruct read error: option %s line %zu before an"
               " 'img_name' option\n", opt_start, lineno);
        return NULL;
      }

      Vector2 size_f;
      if (!parse_vector2(opt_end+1, &size_f)) {
        printf(" found option %s line %zu\n", opt_start, lineno);
        return NULL;
      }

      current_imgstruct->size_f = size_f;
    }
    else if (!strncmp(opt_start, "size_interp", opt_len)) {
      if (current_imgstruct == NULL) {
        printf("slidestruct read error: option %s line %zu before an"
            " 'img_name' option\n", opt_start, lineno);
        return NULL;
      }

      interp_type t;
      if (!parse_interp_type(opt_end+1, &t)) {
        printf(" option %s line %zu\n", opt_start, lineno);
        return NULL;
      }

      current_imgstruct->size_interp = t;
    }
    else if (!strncmp(opt_start, "size_interp_captype", opt_len)) {
      if (current_imgstruct == NULL) {
        printf("slidestruct read error: option %s line %zu before an"
            " 'img_name' option\n", opt_start, lineno);
        return NULL;
      }

      interp_captype t;
      if (!parse_interp_captype(opt_end+1, &t)) {
        printf(" option %s line %zu\n", opt_start, lineno);
        return NULL;
      }

      current_imgstruct->size_interp_captype = t;
    }
    else if (!strncmp(opt_start, "size_duration", opt_len)) {
      if (current_imgstruct == NULL) {
        printf("slidestruct read error: option %s line %zu before an"
               " 'img_name' option\n", opt_start, lineno);
        return NULL;
      }

      float size_duration;
      if (!parse_float(opt_end+1, &size_duration)) {
        printf(" found option %s line %zu\n", opt_start, lineno);
        return NULL;
      }
      
      current_imgstruct->size_duration = size_duration;
    }
    else if (!strncmp(opt_start, "rot_i", opt_len)) {
      if (current_imgstruct == NULL) {
        printf("slidestruct read error: option %s line %zu before an"
               " 'img_name' option\n", opt_start, lineno);
        return NULL;
      }

      float rot_i;
      if (!parse_float(opt_end+1, &rot_i)) {
        printf(" found option %s line %zu\n", opt_start, lineno);
        return NULL;
      }
      
      current_imgstruct->rot_i = rot_i;
    }
    else if (!strncmp(opt_start, "rot_f", opt_len)) {
      if (current_imgstruct == NULL) {
        printf("slidestruct read error: option %s line %zu before an"
               " 'img_name' option\n", opt_start, lineno);
        return NULL;
      }

      float rot_f;
      if (!parse_float(opt_end+1, &rot_f)) {
        printf(" found option %s line %zu\n", opt_start, lineno);
        return NULL;
      }
      
      current_imgstruct->rot_f = rot_f;
    }
    else if (!strncmp(opt_start, "rot_interp", opt_len)) {
      if (current_imgstruct == NULL) {
        printf("slidestruct read error: option %s line %zu before an"
            " 'img_name' option\n", opt_start, lineno);
        return NULL;
      }

      interp_type t;
      if (!parse_interp_type(opt_end+1, &t)) {
        printf(" option %s line %zu\n", opt_start, lineno);
        return NULL;
      }

      current_imgstruct->rot_interp = t;
    }
    else if (!strncmp(opt_start, "rot_interp_captype", opt_len)) {
      if (current_imgstruct == NULL) {
        printf("slidestruct read error: option %s line %zu before an"
            " 'img_name' option\n", opt_start, lineno);
        return NULL;
      }

      interp_captype t;
      if (!parse_interp_captype(opt_end+1, &t)) {
        printf(" option %s line %zu\n", opt_start, lineno);
        return NULL;
      }

      current_imgstruct->rot_interp_captype = t;
    }
    else if (!strncmp(opt_start, "rot_duration", opt_len)) {
      if (current_imgstruct == NULL) {
        printf("slidestruct read error: option %s line %zu before an"
               " 'img_name' option\n", opt_start, lineno);
        return NULL;
      }

      float rot_duration;
      if (!parse_float(opt_end+1, &rot_duration)) {
        printf(" found option %s line %zu\n", opt_start, lineno);
        return NULL;
      }
      
      current_imgstruct->rot_duration = rot_duration;
    }
    else {
      // Not a supported option
      printf("slidestruct read error: option %.*s found line %zu is not a"
          " supported option\n", (int)opt_len, opt_start, lineno);
      return NULL;
    }

  }

  // If we exited with an imgstruct left over, we must link it:
  if (current_head_imgstruct != NULL) {
    current_slidestruct->images = current_head_imgstruct;
  }

  free(linebuf); // Needs to be freed regardless of error.
  
  if (fclose(f)) {
    perror("slidestruct read file close error");
    return NULL;
  }

  if (nchar == -1) { // We ended due to error or EOF
    if (errno == EINVAL || errno == ENOMEM) {
      // There was an error in reading: print error and return NULL
      perror("slidestruct read system error");
      return NULL;
    }
  }

  return head_slidestruct;
}

void slidestruct_print(slidestruct *ss)
{
  for (slidestruct *s = ss; s != NULL; s = s->next) {
    // Print slide info:
    printf("title: %s\n", s->title);
    printf("title_duration: %f\n", s->title_duration);
    printf("slide_duration: %f\n", s->slide_duration);

    for (imgstruct *i = s->images; i != NULL; i = i->next) {
      // Print image info:
      printf("img_name: %s\n", i->img_name);

      Color color = i->tint_i;
      printf("tint_i: (%d, %d, %d, %d)\n", color.r, color.g, color.b, color.a);
      color = i->tint_f;
      printf("tint_f: (%d, %d, %d, %d)\n", color.r, color.g, color.b, color.a);
      printf("tint_interp: %u\n", i->tint_interp);
      printf("tint_interp_captype: %u\n", i->tint_interp_captype);
      printf("tint_duration: %f\n", i->tint_duration);

      Vector2 vec2 = i->pos_i;
      printf("pos_i: (%f, %f)\n", vec2.x, vec2.y);
      vec2 = i->pos_f;
      printf("pos_f: (%f, %f)\n", vec2.x, vec2.y);
      printf("pos_interp: %d\n", i->pos_interp);
      printf("pos_interp_captype: %u\n", i->pos_interp_captype);
      printf("pos_duration: %f\n", i->pos_duration);

      vec2 = i->size_i;
      printf("size_i: (%f, %f)\n", vec2.x, vec2.y);
      vec2 = i->size_f;
      printf("size_f: (%f, %f)\n", vec2.x, vec2.y);
      printf("size_interp: %d\n", i->size_interp);
      printf("size_interp_captype: %u\n", i->size_interp_captype);
      printf("size_duration: %f\n", i->size_duration);

      printf("rot_i: %f\n", i->rot_i);
      printf("rot_f: %f\n", i->rot_f);
      printf("rot_interp: %d\n", i->rot_interp);
      printf("rot_interp_captype: %u\n", i->rot_interp_captype);
      printf("rot_duration: %f\n", i->rot_duration);
    }
  }
}

void slidestruct_free (slidestruct *ss)
{
  // Free all slides starting from the head slide.
  slidestruct *s = ss;
  while (s != NULL) {
    // Free all images starting from the head image.
    imgstruct *i = s->images;
    while (i != NULL) {
      imgstruct *old_i = i;
      i = i->next;
      free(old_i->img_name);
      free(old_i);
    }

    slidestruct *old_s = s;
    s = s->next;
    free(old_s->title);
    free(old_s);
  }
}

/**
 * Returns a new instance of slidestruct with defaults from
 *   slidestruct_defaults.h
 * If a slidestruct could not be created (due to malloc error), then NULL is
 *   returned instead and an error message is printed.
 */
slidestruct *construct_slidestruct (void)
{
  slidestruct *new_ss = malloc(sizeof(slidestruct));
  if (new_ss == NULL) {
    perror("slidestruct read malloc error");
    return NULL;
  }

  // Set defaults:
  new_ss->title = NULL;
  new_ss->title_duration = TITLE_DURATION_DEFAULT;
  new_ss->slide_duration = SLIDE_DURATION_DEFAULT;
  new_ss->images = NULL;
  new_ss->next = NULL;
  return new_ss;
}

/**
 * Returns a new instance of imgstruct with defaults from
 *   slidestruct_defaults.h
 * If an imgstruct could not be created (due to malloc error), then NULL is
 *   returned instead and an error message is printed.
 */
imgstruct *construct_imgstruct (void)
{
  imgstruct *new_is = malloc(sizeof(imgstruct));
  if (new_is == NULL) {
    perror("slidestruct read malloc error");
    return NULL;
  }

  // Set defaults:
  new_is->img_name = NULL;
  new_is->tint_i = TINT_I_DEFAULT;
  new_is->tint_f = TINT_F_DEFAULT;
  new_is->tint_interp = TINT_INTERP_DEFAULT;
  new_is->tint_interp_captype = TINT_INTERP_CAPTYPE_DEFAULT;
  new_is->tint_duration = TINT_DURATION_DEFAULT;
  new_is->pos_i = POS_I_DEFAULT;
  new_is->pos_f = POS_F_DEFAULT;
  new_is->pos_interp = POS_INTERP_DEFAULT;
  new_is->pos_interp_captype = POS_INTERP_CAPTYPE_DEFAULT;
  new_is->pos_duration = POS_DURATION_DEFAULT;
  new_is->size_i = SIZE_I_DEFAULT;
  new_is->size_f = SIZE_F_DEFAULT;
  new_is->size_interp = SIZE_INTERP_DEFAULT;
  new_is->size_interp_captype = SIZE_INTERP_CAPTYPE_DEFAULT;
  new_is->size_duration = SIZE_DURATION_DEFAULT;
  new_is->rot_i = ROT_I_DEFAULT;
  new_is->rot_f = ROT_F_DEFAULT;
  new_is->rot_interp = ROT_INTERP_DEFAULT;
  new_is->rot_interp_captype = ROT_INTERP_CAPTYPE_DEFAULT;
  new_is->rot_duration = ROT_DURATION_DEFAULT;
  new_is->next = NULL;
  return new_is;
}

// Returns true if the given NUL-terminated string str is whitespace only.
bool is_whitespace_str (const char *str)
{
  size_t idx = 0;

  // Search every char to see if it is not whitespace until we run into NUL:
  char c;
  while ( (c = str[idx]) != '\0') {
    if (!isspace(c))
      return false;
    
    idx++;
  }

  // We searched every char and didn't find anything except whitespace: i.e.
  //   the whole string is whitespace.
  return true;
}

/**
 * Returns a pointer to the first non whitespace char in the given string.
 * If no such char exists(i.e. is_whitespace_str(str) == true), NULL is
 *   returned.
 */
const char *first_non_whitespace_char (const char *str)
{
  size_t idx = 0;

  char c;
  while ( (c = str[idx]) != '\0') {
    if (!isspace(c))
      return (str+idx);
    
    idx++;
  }

  return NULL;
}

/**
 * Populates the float argument f with the float extracted from string str.
 * Returns true if successful, false if not.
 */
bool parse_float(const char *str, float *f)
{
  char *endptr;
  *f = strtof(str, &endptr);
  if (str == endptr || endptr == NULL
      || (*endptr != '\0' && !isspace(*endptr))) {
    // An error occured in parsing.
    printf("slidestruct read error: float could not be parsed from string %s",
        str);
    return false;
  }
  return true;
}

/**
 * Populates the color argument with the color extracted from string str using
 *   the following rules:
 * The str should appear like so: "(r,g,b,a)" where r, g, b, a are to be parsed
 *   as unsigned chars and can have whitespace prepended to them.
 *
 * If parsing is successful, then true is returned.
 * If any parsing fails, then returns false.
 * If the parsing would fail, this function prints an error message.
 */
bool parse_color(const char *str, Color *color)
{
  // Find the '('.
  const char *sett_start = first_non_whitespace_char(str);
  if (*sett_start != '(') {
    printf("slidestruct read error: malformed color. Found '%c' before '('",
        *sett_start);
    return false;
  }

  unsigned char rgba[4];

  // Take all chars up to the first ','
  // Take all chars after the first ',' up to the second ','
  // Take all chars after the second ',' up to the third ','
  // Take all chars after the third ',' up to the ending ')'
  char *endptr;
  for (size_t idx = 0; idx < 4; idx++) {
    unsigned char convert;
    bool no_overflow = true;
    if (idx == 0)
      no_overflow = strtouc(&convert, sett_start+1, &endptr, 10);
    else {
      const char *new_str = endptr+1;
      no_overflow = strtouc(&convert, new_str, &endptr, 10);
    }

    // Error checking
    if (!no_overflow) {
      // The number was too large.
      printf("slidestruct read error: malformed color. Entry %zu too "
          "large - must be in range [0, 255]. Error", idx);
      return false;
    }
    else if (str == endptr) {
      // The string didn't start with a number.
      printf("slidestruct read error: malformed color. Entry %zu did not start"
          " with a number. Error", idx);
      return false;
    }
    else if (idx != 3 && *endptr != ',') {
      // The string is malformed. The number should have an ',' after it.
      printf("slidestruct read error: malformed color. Character after entry "
          "%zu must be a ','. Error", idx);
      return false;
    }
    else if (idx == 3 && *endptr != ')') {
      // The string is malformed. The string should end in a ')'.
      printf("slidestruct read error: malformed color. Character after entry "
          "%zu must be a ')'. Error", idx);
      return false;
    }

    // Save the converted value
    rgba[idx] = convert;
  }

  *color = (Color){rgba[0], rgba[1], rgba[2], rgba[3]};
  return true;
}

/**
 * Populates the color argument with the color extracted from string str using
 *   the following rules:
 * The str should appear like so: "(x,y)" where x and y are string
 *   representations of floats.
 *
 * If parsing is successful, then true is returned.
 * If any parsing fails, then returns false.
 * If the parsing would fail, this function prints an error message.
 */
bool parse_vector2 (const char *str, Vector2 *v)
{
  // Find the '('.
  const char *sett_start = first_non_whitespace_char(str);
  if (*sett_start != '(') {
    printf("slidestruct read error: malformed Vector2. Found '%c' before '('",
        *sett_start);
    return false;
  }

  float xy[2];

  // Take all chars up to the first ','
  // Take all chars after the first ',' up to the ')'
  char *endptr;
  for (size_t idx = 0; idx < 2; idx++) {
    float convert;
    if (idx == 0)
      convert = strtof(sett_start+1, &endptr);
    else {
      const char *new_str = endptr+1;
      convert = strtof(new_str, &endptr);
    }

    // Error checking
    if (str == endptr) {
      // The string didn't start with a number.
      printf("slidestruct read error: malformed color. Entry %zu did not start"
          " with a number. Error", idx);
      return false;
    }
    else if (idx == 0 && *endptr != ',') {
      // The string is malformed. The number should have an ',' after it.
      printf("slidestruct read error: malformed color. Character after entry "
          "%zu must be a ','. Error", idx);
      return false;
    }
    else if (idx == 1 && *endptr != ')') {
      // The string is malformed. The string should end in a ')'.
      printf("slidestruct read error: malformed color. Character after entry "
          "%zu must be a ')'. Error", idx);
      return false;
    }

    // Save the converted value
    xy[idx] = convert;
  }

  *v = (Vector2){xy[0], xy[1]};
  return true;
}

/**
 * Returns if successfully parsed the interp_type from string str. If an error
 *   would occur, this returns false. The parsed interp_type is stored in the
 *   given interp_type pointer 'type'
 * 
 * If the parsing would fail, this function prints an error message without a
 *   newline.
 */
bool parse_interp_type (const char *str, interp_type *type)
{
  char *endptr;
  unsigned char t;
  if (!strtouc(&t, str, &endptr, 10)) {
    // The number was too large.
    printf("slidestruct read error: malformed interp type. Specified type too "
        "large - must be in range [0, %u]. Error", UCHAR_MAX);
    return false;
  }
  else if (str == endptr) {
    // The string didn't start with a number.
    printf("slidestruct read error: malformed interp type. Specified type did "
        "not start with a number. Error");
    return false;
  }
  else if (*endptr != '\n') {
    printf("slidestruct read error: malformed interp type. Specified type did "
        "not contain only a number. Error");
    return false;
  }
  else if (t > INTERP_TYPE_MAX) {
    printf("slidestruct read error: parsed value %u out of range", t);
    return false;
  }

  *type = (interp_type)t;
  return true;
}

/**
 * Returns true if the interp_captype parsed from string str is valid.
 * If an error would occur, this returns false.
 * 
 * If the parsing would fail, this function prints an error message without a
 *   newline.
 */
bool parse_interp_captype (const char *str, interp_captype *captype)
{
  char *endptr;
  unsigned char t;
  if (!strtouc(&t, str, &endptr, 10)) {
    // The number was too large.
    printf("slidestruct read error: malformed captype. Specified captype too "
        "large - must be in range [0, %u]. Error", UCHAR_MAX);
    return false;
  }
  else if (str == endptr) {
    // The string didn't start with a number.
    printf("slidestruct read error: malformed captype. Specified captype did "
        "not start with a number. Error");
    return false;
  }
  else if (*endptr != '\n') {
    printf("slidestruct read error: malformed captype. Specified captype did "
        "not contain only a number. Error");
    return false;
  }
  else if (t > INTERP_CAPTYPE_MAX) {
    printf("slidestruct read error: parsed value %u out of range", t);
    return false;
  }

  *captype = (interp_captype)t;
  return true;
}

/**
 * Parses str via strtoul and returns true if successful in converting its
 *   result to a unsigned char.
 * Returns false if the converted value would overflow when converted to an
 *   unsigned char.
 */
bool strtouc (unsigned char *c, const char *str, char **endptr, int base)
{
  unsigned long l = strtoul(str, endptr, base);
  if (l > UCHAR_MAX) {
    return false;
  }
  *c = (unsigned char)l;
  return true;
}
