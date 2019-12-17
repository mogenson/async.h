#include "tasks.h"
#include <stdio.h>

TASK(f, int i);
TASK(g);

TASK(f, int i) {
  TASK_BEGIN();

  while (i < 5) {
    printf("in f(): %d\n", i);
    TASK_YIELD();
  }

  TASK_YIELD_UNTIL(i == 8);

  printf("task done\n");
  TASK_END();
}

TASK(g) {
  TASK_BEGIN();
  printf("in g() at time %d\n", millis());
  TASK_YIELD_FOR(1000);
  printf("in g() at time %d\n", millis());
  TASK_END();
}

int main() {

  AWAIT(g);
  for (int j = 0; j < 10; j++)
    RUN(f, j);

  printf("main done\n");

  return 0;
}
