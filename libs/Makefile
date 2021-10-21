# -- This Makefile should only be called recursively. --
OBJS = memalloc.o vbprint.o tprinterr.o genarrays.o hashing.o arrcmp.o

.SUFFIXES: .c .o

.PHONY: all docs clean clean_docs rebuild remake_docs

all: $(OBJS)

rebuild: clean all

clean:
	$(REMOVE) *.o

$(LIB): $(LIBOBJS)
	ar rcs $(LIB) $(LIBOBJS)

memalloc.o: memalloc.h vbprint.h vbprint.o

