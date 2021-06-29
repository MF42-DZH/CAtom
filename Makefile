CC      = gcc
CFLAGS  = -g3 -Og -D_POSIX_SOURCE -D_DEFAULT_SOURCE -std=c99 -Wextra -Werror -pedantic
LIB     = libtestsuite.a
LIBOBJS = testsuite.o
BUILD   = $(LIB)

.SUFFIXES: .c .o

.PHONY: all docs clean clean_docs rebuild remake_docs

all: $(BUILD)

docs:
	+$(MAKE) -C doc

rebuild: clean all

remake_docs:
	+$(MAKE) remake -C doc

clean:
	rm -f $(BUILD) *.o

clean_docs:
	+$(MAKE) clean -C doc

$(LIB): $(LIBOBJS)
	ar rcs $(LIB) $(LIBOBJS)

testsuite.o: testsuite.h
