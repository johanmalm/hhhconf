CFLAGS  += -g -Wall -std=c11 -pedantic

PROGS = hhhconf-t2

all: $(PROGS)

hhhconf-t2: hhhconf-t2.o
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f *.o $(PROGS)

