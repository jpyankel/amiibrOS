/**
 * test.c
 *
 * Compile with test.c in place of main.c to run a simple slidestruct lifecycle
 *   test (construct and free).
 *
 * Joseph Yankel (jpyankel@gmail.com)
 */

#include "slidestruct.h"

int main (void) {
  slidestruct *ss = slidestruct_read_conf("conf.txt");
  
  slidestruct_print(ss);

  slidestruct_free(ss);

  return 0;
}
