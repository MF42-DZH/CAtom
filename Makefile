CC      = gcc
CFLAGS  = -g3 -Og -D_POSIX_SOURCE -D_DEFAULT_SOURCE -std=c99 -Wextra -Werror -pedantic
LIB     = libcatom.a
LIBOBJS = catom.o
BUILD   = $(LIB)

.SUFFIXES: .c .o

.PHONY: all docs clean clean_docs rebuild remake_docs

all: $(BUILD)

rebuild: clean all

clean:
	rm -f $(BUILD) *.o

docs:
	+$(MAKE) -C doc

remake_docs:
	+$(MAKE) remake -C doc

clean_docs:
	+$(MAKE) clean -C doc

$(LIB): $(LIBOBJS)
	ar rcs $(LIB) $(LIBOBJS)

catom.o: catom.h salloc.h
