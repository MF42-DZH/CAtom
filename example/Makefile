CC      = cc
CFLAGS  = -g3 -Og -D_POSIX_SOURCE -D_DEFAULT_SOURCE -std=c99 -Wextra -Werror -pedantic
LDFLAGS = -L.. -lcatom
NAME    = testexample
OBJS    = example.o testexample.o
BUILD   = $(TARGET)

TARGET  =
REMOVE  =
ifeq ($(OS), Windows_NT)
	TARGET += testexample.exe
	REMOVE += DEL /S /F /Q
else
	TARGET += testexample
	REMOVE += rm -rf
endif

.SUFFIXES: .c .o

.PHONY: all clean mod rebuild test testsuite

all: mod testsuite $(BUILD) test

rebuild: clean all

clean:
	$(REMOVE) $(BUILD) *.o

$(TARGET): $(OBJS)
	gcc $(OBJS) -o $@ $(LDFLAGS)

mod:
	touch $(NAME).c

test:
	./$(TARGET)

testsuite:
	+$(MAKE) -C ..

example.o: example.h
