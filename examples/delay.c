#include "../async.h"
#include <stdio.h>

ASYNC(count) {
  TASK_BEGIN();

  printf("1\n");
  YIELD_FOR(1000);
  printf("2\n");
  YIELD_FOR(1000);
  printf("3\n");

  TASK_END();
}

ASYNC(delay) {
  TASK_BEGIN();

  printf("waiting for 3 seconds\n");
  YIELD_FOR(3000);
  printf("done waiting\n");

  TASK_END();
}

int main() {

  while (AWAIT(delay)->status)
    AWAIT(count);

  return 0;
}
