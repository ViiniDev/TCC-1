CC = gcc
CFLAGS = -O3 -march=native -Wall
LDFLAGS = -lm

TARGETS = conv_malloc dgemm

all: $(TARGETS)

conv_malloc: main_malloc.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

dgemm: dgemm.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS)