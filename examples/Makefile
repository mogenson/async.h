CFLAGS = -Wall -ggdb
CC = gcc
SRCS = arguments generator factorial nested delay spsc password

all: $(SRCS)
arguments: arguments.c
generator: generator.c
factorial: factorial.c
nested: nested.c
delay: delay.c
spsc: spsc.c
password: password.c -lncurses

clean:
	$(RM) $(SRCS)
