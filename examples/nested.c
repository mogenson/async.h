#include "../async.h"
#include <stdio.h>

/* task declarations */
ASYNC(upper);
ASYNC(lower);
ASYNC(numeric);

/* task definitions */
ASYNC(upper) {
  static char c;
  TASK_BEGIN();

  for (c = 'A'; c <= 'F'; c++) {
    printf("%c", c);
    AWAIT(lower);
    YIELD();
  }

  TASK_END();
}

ASYNC(lower) {
  static char c;
  TASK_BEGIN();

  for (c = 'a'; c <= 'f'; c++) {
    printf("%c", c);
    YIELD();
  }

  TASK_END();
}

ASYNC(numeric) {
  static int i;
  TASK_BEGIN();

  for (i = 0; i <= 9; i++) {
    printf("%d", i);
    YIELD();
  }

  TASK_END();
}

int main() {

  /* numeric task will not run until after upper task
   * due to short circuit evaluation
   * while (AWAIT(upper)->status || AWAIT(numeric)->status) continue;
   */

  task_status_t upper_status = TASK_RUNNING, numeric_status = TASK_RUNNING;
  while (upper_status || numeric_status) {
    if (upper_status)
      upper_status = AWAIT(upper)->status;
    if (numeric_status)
      numeric_status = AWAIT(numeric)->status;
  }

  putchar('\n');

  return 0;
}
