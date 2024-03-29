CFLAGS  += -g -Wall -std=c11 -pedantic
prefix  ?= $(HOME)
bindir   = $(prefix)/bin
PROGS    = hhhconf-t2 hhhconf-obtheme
SCRIPTS  = hhhconf

all: $(PROGS)

install: $(PROGS)
	install -m755 $(PROGS) $(SCRIPTS) $(DESTDIR)$(bindir)

hhhconf-t2: hhhconf-t2.o util.o
	$(CC) $(CFLAGS) -o $@ $^

hhhconf-obtheme: hhhconf-obtheme.o util.o
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f *.o $(PROGS)

test:
	@splint -weak -redef $(wildcard *.c) 2>/dev/null
	@shellcheck $(SCRIPTS)
