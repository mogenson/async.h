#include "../async.h"
#include <stdio.h>

/* define task to return factorial of num at end */
ASYNC(factorial, int num) {
  static int i, result = 1;
  BEGIN();

  for (i = 1; i <= num; i++) {
    result *= i;
    YIELD(&result); // return a pointer to result
  }

  END(&result); // return a pointer to result and signal task is done
}

int main() {

  int ans = *(int *)BLOCK(factorial, 5)->result;
  printf("factorial of 5 is %d\n", ans);
}
