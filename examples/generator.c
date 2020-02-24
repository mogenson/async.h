#include "../async.h"
#include <stdio.h>

/* define task named generator that takes an argument and returns a result */
ASYNC(generator, int max) {
  static int i; // returned results must be static
  BEGIN();

  for (i = 0; i < max; i++) {
    YIELD(&i); // return a pointer to i
  }

  END(); // END() return result will be NULL if not provided
}

int main() {
  void *item;
  int sum = 0;
  int max = 10;

  /* run generator repeatedly, as long as item != NULL */
  while (item = AWAIT(generator, max)->result, item) {
    sum += *(int *)item; // cast item as a pointer to an int, then dereference
  }

  printf("sum of 0 to %d is %d\n", max - 1, sum);
}
