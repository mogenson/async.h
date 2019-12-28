#include "../async.h"
#include <stdio.h>

/* define an async task named echo */
ASYNC(echo) {
  static int c = 0; // local variables should be declared static

  TASK_BEGIN(); // must be included at beginning of task

  while (1) {
    c = getchar();

    if (c == EOF) {
      break; // break out of while loop and call TASK_END()
    } else {
      printf("%c", c); // print c as a char
    }

    YIELD(); // yield back to main()
  }

  TASK_END(); // must be included at end of task
}

int main() {

  /* run echo task once, should print one char */
  AWAIT(echo);

  /* run echo task until completion, press ctrl-d to send EOF and end task */
  BLOCK(echo);

  return 0;
}
