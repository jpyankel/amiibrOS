/**
 * slidestruct.c
 *
 * Contains implementation of slidestruct.h
 *
 * Joseph Yankel (jpyankel@gmail.com)
 */

#include <stdio.h> // File handling, fopen, getline, perror
#include <string.h> // strcat, strncpy, strchr
#include <stdlib.h> // malloc, NULL, atof
#include <ctype.h> // isspace
#include <errno.h> // Various error code consts.
#include "slidestruct.h" // includes bool type
#include "slidestruct_defaults.h"

// --- Helper Function Prototypes ---
slidestruct *construct_slidestruct (void);
imgstruct *construct_imgstruct (void);
bool is_whitespace_str (const char *str);
const char *first_non_whitespace_char (const char *str);
bool parse_color(const char *str, Color *color);
interp_type parse_interp_type (const char *str);
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

    // To isolate the option name, we search for the index of the first non
    //   whitespace char and the first space found after this char. The string
    //   created by the characters in between (including the first char but not
    //   including the ending space) is our option name.
    const char *opt_start = first_non_whitespace_char(linebuf);
    if (opt_start == NULL) { // This can occur if multiple '\\' are 
      printf("slidestruct read error: no option found line %zu\n", lineno);
      continue;
    }
    const char *opt_end = strchr(opt_start, ' ');
    
    // If there are no spaces after opt_start, then there are no parameters.
    if (opt_end == NULL) {
      printf("slidestruct read error: option %s line %zu ended without"
          " settings\n", opt_start, lineno);

      continue; // We do not support options without parameters: skip this opt.
    }

    // Create copy of option for easy use:
    size_t opt_len = opt_end-opt_start;

    printf("%s\n", linebuf); // TODO: Remove
    printf("option: %.*s\n", (int)opt_len, opt_start);
    printf("setting: %s\n", opt_end+1);
    
    // Compare opt_len chars from opt_start to each of the possible options:
    if (!strncmp(opt_start, "title", opt_len)) {
      printf("'title' option matched!");
      
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
    }
    else if (!strncmp(opt_start, "title_duration", opt_len)) {
      printf("'title_duration' option matched!");

      if (current_slidestruct == NULL) {
        // The user has input a title-related option before the 'title' option.
        printf("slidestruct read error: option %s line %zu before a 'title'"
            " option\n", opt_start, lineno);
        continue; // Ignore this option as it doesn't apply to anything.
      }

      // Parse setting as float and set the title_duration:
      float title_duration = atof(opt_end+1);
      current_slidestruct->title_duration = title_duration;
    }
    else if (!strncmp(opt_start, "slide_duration", opt_len)) {
      printf("'slide_duration' option matched!");

      if (current_slidestruct == NULL) {
        printf("slidestruct read error: option %s line %zu before a 'title'"
            " option\n", opt_start, lineno);
        continue;
      }

      float slide_duration = atof(opt_end+1);
      current_slidestruct->slide_duration = slide_duration;
    }
    else if (!strncmp(opt_start, "img_name", opt_len)) {
      printf("'img_name' option matched!");

      if (current_slidestruct == NULL) {
        printf("slidestruct read error: option %s line %zu before a 'title'"
            " option\n", opt_start, lineno);
        continue;
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
    }
    else if (!strncmp(opt_start, "tint_i", opt_len)) {
      printf("'tint_i' option matched!");

      // Ensure that the user specifies an image before changing image
      //   properties
      if (current_imgstruct == NULL) {
        printf("slidestruct read error: option %s line %zu before an"
            " 'img_name' option\n", opt_start, lineno);
        continue;
      }

      Color color;
      if (!parse_color(opt_end+1, &color))
        continue; // Could not parse. Move on.

      current_imgstruct->tint_i = color;
    }
    else if (!strncmp(opt_start, "tint_f", opt_len)) {
      printf("'tint_f' option matched!");

      // Ensure that the user specifies an image before changing image
      //   properties
      if (current_imgstruct == NULL) {
        printf("slidestruct read error: option %s line %zu before an"
            " 'img_name' option\n", opt_start, lineno);
        continue;
      }

      Color color;
      if (!parse_color(opt_end+1, &color))
        continue; // Could not parse. Move on.

      current_imgstruct->tint_f = color;
    }
    else if (!strncmp(opt_start, "tint_interp", opt_len)) {
      printf("'tint_interp' option matched!");
      if (current_imgstruct == NULL) {
        printf("slidestruct read error: option %s line %zu before an"
            " 'img_name' option\n", opt_start, lineno);
        continue;
      }

      interp_type t = parse_interp_type(opt_end+1);
      if (t == ERROR) {
        printf(" option %s line %zu\n", opt_start, lineno);
      }

      current_imgstruct->tint_interp = t;
    }
    else if (!strncmp(opt_start, "tint_duration", opt_len)) {
      // TODO
    }
    else if (!strncmp(opt_start, "pos_i", opt_len)) {
      // TODO
    }
    else if (!strncmp(opt_start, "pos_f", opt_len)) {
      // TODO
    }
    else if (!strncmp(opt_start, "pos_interp", opt_len)) {
      // TODO
    }
    else if (!strncmp(opt_start, "pos_duration", opt_len)) {
      // TODO
    }
    else if (!strncmp(opt_start, "size_i", opt_len)) {
      // TODO
    }
    else if (!strncmp(opt_start, "size_f", opt_len)) {
      // TODO
    }
    else if (!strncmp(opt_start, "size_interp", opt_len)) {
      // TODO
    }
    else if (!strncmp(opt_start, "size_duration", opt_len)) {
      // TODO
    }
    else if (!strncmp(opt_start, "rot_i", opt_len)) {
      // TODO
    }
    else if (!strncmp(opt_start, "rot_f", opt_len)) {
      // TODO
    }
    else if (!strncmp(opt_start, "rot_interp", opt_len)) {
      // TODO
    }
    else if (!strncmp(opt_start, "rot_duration", opt_len)) {
      // TODO
    }
    else {
      // Not a supported option
      printf("slidestruct read error: option %.*s found line %zu is not a"
          " supported option\n", (int)opt_len, opt_start, lineno);
      continue;
    }

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
  new_is->tint_duration = TINT_DURATION_DEFAULT;
  new_is->pos_i = POS_I_DEFAULT;
  new_is->pos_f = POS_F_DEFAULT;
  new_is->pos_interp = POS_INTERP_DEFAULT;
  new_is->pos_duration = POS_DURATION_DEFAULT;
  new_is->size_i = SIZE_I_DEFAULT;
  new_is->size_f = SIZE_F_DEFAULT;
  new_is->size_interp = SIZE_INTERP_DEFAULT;
  new_is->size_duration = SIZE_DURATION_DEFAULT;
  new_is->rot_i = ROT_I_DEFAULT;
  new_is->rot_f = ROT_F_DEFAULT;
  new_is->rot_interp = ROT_INTERP_DEFAULT;
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
  // TODO
  // Take all chars up to the first ','
  // TODO
  // Take all chars after the first ',' up to the second ','
  // TODO
  // Take all chars after the second ',' up to the third ','
  // TODO
  // Take all chars after the third ',' up to the ending ')'
  // TODO
}

/**
 * Returns the interp_type parsed from string str. If an error would occur,
 *   this returns interp_type.ERROR
 * 
 * If the parsing would fail, this function prints an error message without a
 *   newline.
 */
interp_type parse_interp_type (const char *str)
{
  int interp = atoi(str);
  if (interp < INTERP_TYPE_MIN || interp > INTERP_TYPE_MAX) {
    printf("slidestruct read error: parsed value %d out of range", interp);
    return ERROR;
  }
  return (interp_type)interp;
}
