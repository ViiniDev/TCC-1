#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static int parse_int(const char *text, int *value) {
    char *end = NULL;
    long parsed;

    errno = 0;
    parsed = strtol(text, &end, 10);

    if (errno != 0 || end == text || *end != '\0' || parsed < INT_MIN || parsed > INT_MAX) {
        return 0;
    }

    *value = (int)parsed;
    return 1;
}

static int valida_argumentos(int N, int BS) {
    if (N <= 0) {
        fprintf(stderr, "Erro: N deve ser maior que zero.\n");
        return 0;
    }

    if (BS <= 0) {
        fprintf(stderr, "Erro: BS deve ser maior que zero.\n");
        return 0;
    }

    return 1;
}

static void dgemm_blocked(int N, int BS, double *A, double *B, double *C) {
    for (int ii = 0; ii < N; ii += BS) {
        for (int jj = 0; jj < N; jj += BS) {
            for (int kk = 0; kk < N; kk += BS) {
                for (int i = ii; i < ii + BS && i < N; i++) {
                    for (int j = jj; j < jj + BS && j < N; j++) {
                        double sum = C[i * N + j];

                        for (int k = kk; k < kk + BS && k < N; k++) {
                            sum += A[i * N + k] * B[k * N + j];
                        }

                        C[i * N + j] = sum;
                    }
                }
            }
        }
    }
}

int main(int argc, char *argv[]) {
    int N;
    int BS;
    int seed;

    if (argc != 3 && argc != 4) {
        fprintf(stderr, "Uso: %s <N> <BS> [seed]\n", argv[0]);
        return 1;
    }

    if (!parse_int(argv[1], &N) || !parse_int(argv[2], &BS)) {
        fprintf(stderr, "Erro: argumentos devem ser numeros inteiros validos.\n");
        return 1;
    }

    if (!valida_argumentos(N, BS)) {
        return 1;
    }

    if (argc == 4) {
        if (!parse_int(argv[3], &seed)) {
            fprintf(stderr, "Erro: seed deve ser um numero inteiro valido.\n");
            return 1;
        }
    } else {
        seed = (int)time(NULL);
    }

    srand((unsigned int)seed);

    long long size = (long long)N * N;
    double *A = malloc((size_t)size * sizeof(double));
    double *B = malloc((size_t)size * sizeof(double));
    double *C = calloc((size_t)size, sizeof(double));

    if (!A || !B || !C) {
        fprintf(stderr, "Erro: falha de alocacao.\n");
        free(A);
        free(B);
        free(C);
        return 1;
    }

    for (long long i = 0; i < size; i++) {
        A[i] = (double)rand() / (double)RAND_MAX;
        B[i] = (double)rand() / (double)RAND_MAX;
    }

    struct timespec start, end;

    clock_gettime(CLOCK_MONOTONIC, &start);

    dgemm_blocked(N, BS, A, B, C);

    clock_gettime(CLOCK_MONOTONIC, &end);

    double tempo = (end.tv_sec - start.tv_sec) +
                   (end.tv_nsec - start.tv_nsec) / 1e9;
    double flops = 2.0 * N * N * N;
    double gflops = flops / (tempo * 1e9);

    double checksum = 0.0;

    for (long long i = 0; i < size; i++) {
        checksum += C[i];
    }

    printf("N=%d BS=%d seed=%d Tempo=%.6f s GFLOPS=%.2f\n", N, BS, seed, tempo, gflops);
    printf("Checksum %.6f\n", checksum);

    free(A);
    free(B);
    free(C);

    return 0;
}
