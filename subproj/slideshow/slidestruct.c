/**
 * slidestruct.c
 *
 * Contains implementation of slidestruct.h
 *
 * Joseph Yankel (jpyankel@gmail.com)
 */

#include <stdio.h> // File handling, fopen, getline, perror
#include <string.h> // strcat, strncpy, strchr
#include <stdlib.h> // malloc, NULL
#include <ctype.h> // isspace
#include <errno.h> // Various error code consts.
#include "slidestruct.h" // includes bool type

// The length of the largest option is 14 (+1 for NUL).
//const size_t OPTION_BUF_LEN = 15

// --- Helper Function Prototypes ---
bool is_whitespace_str (const char *str);
const char *first_non_whitespace_char (const char *str);
// --- ---

slidestruct *slidestruct_read_conf (const char *path)
{
  // Open file at path for reading:
  FILE *f;
  if ((f = fopen(path, "r")) == NULL) {
    perror("slidestruct read file open error");
    return NULL;
  }
  
  char *linebuf = NULL;
  size_t buflen = 0; // Updated to hold the length of linebuf.
  ssize_t nchar; // Number of chars read by getline including '\n' but not NUL
  size_t lineno = 0; // Current line number in conf file. Used for error msg.

  char *prevlinebuf;
  size_t prevlinebuflen;
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

      continue; // Skip this option and move to the next
    }

    size_t opt_len = opt_end-opt_start;

    // Compare opt_len chars from opt_start to each of the possible options:
    //TODO

    printf("%s\n", linebuf); // TODO: Remove
    printf("option: %.*s\n", (int)opt_len, opt_start);
    printf("setting: %s\n", opt_end+1);

    // Check to make sure last char is \n to detect if there are any more lines
    //   to parse:
    /*if (linebuf[nchar-1] != '\n')
      break; // No more lines to parse*/
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

  return NULL; //TODO: REMOVE
  //return ss;
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
