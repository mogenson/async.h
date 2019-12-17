#include "async.h"
#include <stdio.h>

ASYNC(generator, int max) {
  static int i;
  TASK_BEGIN();

  for (i = 0; i < max; i++)
    YIELD(&i);

  TASK_END();
}

ASYNC(wait, int delay) {
  static int answer = 42;
  TASK_BEGIN();
  YIELD_FOR(delay);
  TASK_END(&answer);
}

int main() {

  void *item;
  int sum = 0;

  while (item = AWAIT(generator, 10)->result, item)
    sum += *(int *)item;
  printf("sum of 0 to 9 is %d\n", sum);

  int ans = *(int *)BLOCK(wait, 1000)->result;
  printf("answer is %d\n", ans);

  return 0;
}
