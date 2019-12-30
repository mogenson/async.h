#include "../async.h"
#include <stdio.h>

int *shared_item = NULL;

ASYNC(producer) {
  static int item;

  TASK_BEGIN();

  for (item = 0; item < 10; item++) {
    printf("produce item: %d\n", item);
    shared_item = &item;       // set to allow consumer to run
    YIELD_UNTIL(!shared_item); // wait for item == NULL
  }

  TASK_END();
}

ASYNC(consumer) {
  TASK_BEGIN();

  while (1) {
    YIELD_UNTIL(shared_item); // wait for item != NULL
    printf("consume item: %d\n", *shared_item);
    shared_item = NULL; // reset to allow producer to run
  }

  TASK_END();
}

int main() {

  while (AWAIT(producer)->status)
    AWAIT(consumer);

  return 0;
}
