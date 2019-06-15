/**
 * slidestruct.c
 *
 * Contains implementation of slidestruct.h
 *
 * Joseph Yankel (jpyankel@gmail.com)
 */

#include <stdio.h> // File handling, fopen
#include "slidestruct.h"

slidestruct *slidestruct_read_conf (char *path)
{
  // Open file at path for reading:
  FILE *f;
  if ((f = fopen(path, "r")) == NULL)
    return NULL;
  
  // Read line by line, parsing each option and configuring ss.
  //TODO


  //return ss;
}
