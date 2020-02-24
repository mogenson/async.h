#include "../async.h"
#include <stdio.h>

ASYNC(count) {
  static int i;
  BEGIN();

  for (i = 1; i < 5; i++) {
    YIELD_FOR(1);
    printf("%d\n", i);
  }

  END();
}

ASYNC(delay) {
  BEGIN();

  printf("waiting for 5 seconds\n");
  YIELD_FOR(5);
  printf("done waiting\n");

  END();
}

int main() {

  while (AWAIT(delay)->status)
    AWAIT(count);

  return 0;
}
