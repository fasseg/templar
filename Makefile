CC=gcc
CFLAGS=-I. -g
DEPS=templar.h

%.o: %.c $(DEPS)
	$(CC) -g -c -o $@ $< $(CLFLAGS)

all: templar.o
	$(CC) ${CFLAGS} -o templar templar.o

clean:
	rm templar.o
	rm templar
