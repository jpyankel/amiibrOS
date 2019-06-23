/**
 * slidestruct.c
 *
 * Contains implementation of slidestruct.h
 *
 * Joseph Yankel (jpyankel@gmail.com)
 */

#include <stdio.h> // File handling, fopen, getline, perror
#include <string.h> // strcat, strncpy
#include <stdlib.h> // malloc, NULL
#include <ctype.h> // isspace
#include <errno.h> // Various error code consts.
#include "slidestruct.h" // includes bool type

// --- Helper Function Prototypes ---
bool is_whitespace_str (const char *str);
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

  char *prevlinebuf;
  size_t prevlinebuflen;
  bool concat_next = false; // Whether to concat. next line to previous.

  // Read line by line, parsing each option and configuring slidestruct.
  while ((nchar = getline(&linebuf, &buflen, f)) != -1) {
    // Ignore line if it is whitespace only or empty (unless it is appended via
    //   '\'):
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

    // Check to see if we should combine the current line and next line by '\'
    if (nchar > 1 && linebuf[nchar - 2] == '\\'
        && linebuf[nchar - 1] == '\n') {
      // Store the previous line buf and next line buf.
      prevlinebuflen = nchar - 1; // -2 to remove '\' and '\n', +1 for NUL

      prevlinebuf = malloc(prevlinebuflen);
      if (prevlinebuf == NULL) {
        perror("slidestruct read malloc error");
        return NULL;
      }

      // Copy all of the string up until '\' and replace '\' with NUL.
      strncpy(prevlinebuf, linebuf, prevlinebuflen);
      prevlinebuf[prevlinebuflen - 1] = '\0';
      
      // Set a flag to let the next loop know we should concat. the next
      //   line read onto the previous one.
      concat_next = true;

      continue; // Skip parsing and read next line.
    }

    // TODO: Do parsing on linebuf.
    printf("%s\n", linebuf); // TODO: Remove

    // Check to make sure last char is \n to detect if there are any more lines
    //   to parse:
    if (linebuf[nchar-1] != '\n')
      break; // No more lines to parse
  }

  free(linebuf); // Needs to be freed regardless of error.
  
  if (fclose(f)) {
    perror("slidestruct read file close error");
    return NULL;
  }

  if (nchar == -1) { // We ended due to error or EOF
    if (errno == EINVAL || errno == ENOMEM) {
      // There was an error in reading: print error and return NULL
      perror("slidestruct read configuration error");
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
