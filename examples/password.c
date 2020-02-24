#include "../async.h"
#include <ncurses.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

static char *password = "1234";

ASYNC(lock) {
  static int index;
  static int c;

  BEGIN();

  printw("enter password: %s\n", password);

  for (index = 0; index < strlen(password); index++) {
    while (1) {
      YIELD_UNTIL((c = getch(), c != ERR)); // wait until char available
      addch('*');
      if (c == password[index]) {
        break; // leave while loop, goto next index
      } else {
        index = 0; // wrong char, reset index
      }
    }
  }

  printw("\npassword correct");

  END();
}

void ctrlc(int signal) {
  endwin();
  exit(0);
}

int main() {
  initscr();
  cbreak();
  noecho();

  signal(SIGINT, ctrlc);

  BLOCK(lock); // run the task until completion

  refresh();

  while (1)
    continue;

  return 0;
}
