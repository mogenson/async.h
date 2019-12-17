#!/usr/bin/sh

gcc -E main.c | /usr/bin/kak -e "set buffer filetype c"
