#include "../async.h"
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static char *password = "1234";

ASYNC(lock) {
  static int index;
  static char c;

  BEGIN();

  printf("enter password: %s\n", password);

  for (index = 0; index < strlen(password); index++) {
    while (1) {
      YIELD_UNTIL(read(STDIN_FILENO, &c, 1) > 0); // wait until char available
      if (c == password[index]) {
        break; // leave while loop, goto next index
      } else {
        index = 0; // wrong char, reset index
      }
    }
  }

  printf("password correct\n");

  END();
}

int main() {
  /* make reads from stdin non-blocking */
  fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL) | O_NONBLOCK);

  BLOCK(lock); // run the task until completion

  return 0;
}
