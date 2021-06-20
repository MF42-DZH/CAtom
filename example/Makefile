CC      = gcc
CFLAGS  = -g3 -D_POSIX_SOURCE -D_DEFAULT_SOURCE -std=c99 -Wextra -Werror -pedantic
LDFLAGS = -L.. -ltestsuite
TARGET  = testexample
OBJS    = example.o testexample.o
BUILD   = $(TARGET)

.SUFFIXES: .c .o

.PHONY: all clean mod rebuild test testsuite

all: mod testsuite $(BUILD) test

rebuild: clean all

clean:
	rm -f $(BUILD) *.o

$(TARGET): $(OBJS)
	gcc $(OBJS) -o $@ $(LDFLAGS)

mod:
	touch $(TARGET).c

test:
	./$(TARGET)

testsuite:
	+$(MAKE) -C ..

example.o: example.h
