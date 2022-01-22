export CC     = gcc
export CFLAGS = -g3 -Og -D_POSIX_SOURCE -D_DEFAULT_SOURCE -std=c99 -Wextra -Werror -pedantic
export LIB    = libcatom.a
export LIBOBJ = catom.o
export BUILD  = $(LIB)

export REMOVE =
ifeq ($(OS), Windows_NT)
	REMOVE += DEL /S /F /Q
else
	REMOVE += rm -rf
endif

.SUFFIXES: .c .o

.PHONY: all libs docs clean clean_docs rebuild remake_docs

all: $(BUILD)

rebuild: clean all

clean:
	$(REMOVE) $(BUILD) *.o
	+$(MAKE) -C libs clean

docs:
	+$(MAKE) -C doc

remake_docs:
	+$(MAKE) remake -C doc

clean_docs:
	+$(MAKE) clean -C doc

$(LIB): $(LIBOBJ)
	ar rcs $(LIB) $(LIBOBJ) libs/*.o

libs:
	+$(MAKE) -C libs

catom.o: catom.h libs
