CC = gcc
CFLAGS = -O3 -march=native -Wall -Wextra
LDFLAGS = -lm

TARGETS = conv_linear conv_malloc dgemm conv_linear_approx conv_malloc_approx dgemm_approx

all: $(TARGETS)

conv_linear: main_linear.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

conv_malloc: main_malloc.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

dgemm: dgemm_blocked.c
	$(CC) $(CFLAGS) -o $@ $<

conv_linear_approx: conv_linear_approx.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

conv_malloc_approx: conv_malloc_approx.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

dgemm_approx: dgemm_approx.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(TARGETS)
