.PHONY: all clean remake

all: html man

clean:
	rm -rf ./html ./man

html:
	doxygen

man:
	doxygen

remake: clean all
