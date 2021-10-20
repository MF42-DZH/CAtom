CC      = gcc
CFLAGS  = -g3 -Og -D_POSIX_SOURCE -D_DEFAULT_SOURCE -std=c99 -Wextra -Werror -pedantic
LIB     = libcatom.a
LIBOBJS = catom.o memalloc.o vbprint.o tprinterr.o genarrays.o hashing.o arrcmp.o
BUILD   = $(LIB)

REMOVE =
ifeq ($(OS), Windows_NT)
	REMOVE += DEL /S /F /Q
else
	REMOVE += rm -rf
endif

.SUFFIXES: .c .o

.PHONY: all docs clean clean_docs rebuild remake_docs

all: $(BUILD)

rebuild: clean all

clean:
	$(REMOVE) $(BUILD) *.o

docs:
	+$(MAKE) -C doc

remake_docs:
	+$(MAKE) remake -C doc

clean_docs:
	+$(MAKE) clean -C doc

$(LIB): $(LIBOBJS)
	ar rcs $(LIB) $(LIBOBJS)

memalloc.o: memalloc.h vbprint.h vbprint.o

catom.o: catom.h salloc.h memalloc.h memalloc.o vbprint.h vbprint.o tprinterr.h tprinterr.o genarrays.h genarrays.o hashing.h hashing.o arrcmp.h arrcmp.o whatos.h
