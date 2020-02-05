#include "../async.h"
#include <stdio.h>

ASYNC(count) {
  static int i;
  TASK_BEGIN();

  for (i = 1; i < 5; i++) {
    YIELD_FOR(1);
    printf("%d\n", i);
  }

  TASK_END();
}

ASYNC(delay) {
  TASK_BEGIN();

  printf("waiting for 5 seconds\n");
  YIELD_FOR(5);
  printf("done waiting\n");

  TASK_END();
}

int main() {

  while (AWAIT(delay)->status)
    AWAIT(count);

  return 0;
}
