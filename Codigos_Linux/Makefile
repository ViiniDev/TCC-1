CC = gcc
CFLAGS = -O3 -march=native
LDFLAGS = -lm

TARGETS = conv_linear conv_malloc dgemm

all: $(TARGETS)

conv_linear: main_linear.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

conv_malloc: main_malloc.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

dgemm: dgemm_naive.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS)
