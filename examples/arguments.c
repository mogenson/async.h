#include "../async.h"
#include <stdio.h>

/* define an async task named print that takes two arguments */
ASYNC(print, char c, int i) {
  BEGIN();

  while (1) {
    printf("c is '%c' and i is %d\n", c, i); // print arguments
    YIELD();
  }

  END();
}

int main() {
  char c = 'a';
  int i = 0;

  while (i < 10) {
    AWAIT(print, c, i); // run print task with arguments

    c++;
    i++;
  }

  return 0;
}
