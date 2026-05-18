#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef enum { APPROX_FLOAT, APPROX_SKIP_K } approx_t;

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

static int parse_approx(const char *text, approx_t *approx) {
    if (strcmp(text, "float") == 0) {
        *approx = APPROX_FLOAT;
        return 1;
    }

    if (strcmp(text, "skip_k") == 0) {
        *approx = APPROX_SKIP_K;
        return 1;
    }

    return 0;
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

static void dgemm_blocked_exact(int N, int BS, const double *A, const double *B, double *C) {
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

static int dgemm_float(int N, int BS, const double *A, const double *B, double *C) {
    long long size = (long long)N * N;
    float *Af = malloc((size_t)size * sizeof(float));
    float *Bf = malloc((size_t)size * sizeof(float));
    float *Cf = calloc((size_t)size, sizeof(float));

    if (!Af || !Bf || !Cf) {
        free(Af);
        free(Bf);
        free(Cf);
        return 0;
    }

    for (long long i = 0; i < size; i++) {
        Af[i] = (float)A[i];
        Bf[i] = (float)B[i];
    }

    for (int ii = 0; ii < N; ii += BS) {
        for (int jj = 0; jj < N; jj += BS) {
            for (int kk = 0; kk < N; kk += BS) {
                for (int i = ii; i < ii + BS && i < N; i++) {
                    for (int j = jj; j < jj + BS && j < N; j++) {
                        float sum = Cf[i * N + j];

                        for (int k = kk; k < kk + BS && k < N; k++) {
                            sum += Af[i * N + k] * Bf[k * N + j];
                        }

                        Cf[i * N + j] = sum;
                    }
                }
            }
        }
    }

    for (long long i = 0; i < size; i++) {
        C[i] = (double)Cf[i];
    }

    free(Af);
    free(Bf);
    free(Cf);
    return 1;
}

static void dgemm_skip_k(int N, int BS, const double *A, const double *B, double *C) {
    double scale = (double)N / (double)((N + 1) / 2);

    for (int ii = 0; ii < N; ii += BS) {
        for (int jj = 0; jj < N; jj += BS) {
            for (int kk = 0; kk < N; kk += BS) {
                for (int i = ii; i < ii + BS && i < N; i++) {
                    for (int j = jj; j < jj + BS && j < N; j++) {
                        double sum = C[i * N + j];

                        for (int k = kk; k < kk + BS && k < N; k += 2) {
                            sum += A[i * N + k] * B[k * N + j] * scale;
                        }

                        C[i * N + j] = sum;
                    }
                }
            }
        }
    }
}

static void calcula_metricas(const double *exato, const double *aprox, long long size,
                             double *checksum, double *erro_abs, double *erro_rel,
                             double *rmse, double *erro_max) {
    double soma_abs = 0.0;
    double soma_rel = 0.0;
    double soma_quad = 0.0;
    double max_abs = 0.0;

    *checksum = 0.0;

    for (long long i = 0; i < size; i++) {
        double diff = fabs(aprox[i] - exato[i]);
        double denom = fabs(exato[i]) > 1e-12 ? fabs(exato[i]) : 1e-12;

        *checksum += aprox[i];
        soma_abs += diff;
        soma_rel += diff / denom;
        soma_quad += diff * diff;

        if (diff > max_abs) {
            max_abs = diff;
        }
    }

    *erro_abs = soma_abs / (double)size;
    *erro_rel = soma_rel / (double)size;
    *rmse = sqrt(soma_quad / (double)size);
    *erro_max = max_abs;
}

int main(int argc, char *argv[]) {
    int N;
    int BS;
    int seed;
    int compare_errors = 1;
    approx_t approx;

    if (argc != 5 && argc != 6) {
        fprintf(stderr, "Uso: %s <N> <BS> <approx:float|skip_k> <seed> [measure|compare]\n", argv[0]);
        return 1;
    }

    if (!parse_int(argv[1], &N) || !parse_int(argv[2], &BS) || !parse_int(argv[4], &seed)) {
        fprintf(stderr, "Erro: argumentos numericos invalidos.\n");
        return 1;
    }

    if (!parse_approx(argv[3], &approx)) {
        fprintf(stderr, "Erro: aproximacao deve ser float ou skip_k.\n");
        return 1;
    }

    if (!valida_argumentos(N, BS)) {
        return 1;
    }

    if (argc == 6) {
        if (strcmp(argv[5], "measure") == 0) {
            compare_errors = 0;
        } else if (strcmp(argv[5], "compare") == 0) {
            compare_errors = 1;
        } else {
            fprintf(stderr, "Erro: modo deve ser measure ou compare.\n");
            return 1;
        }
    }

    srand((unsigned int)seed);

    long long size = (long long)N * N;
    double *A = malloc((size_t)size * sizeof(double));
    double *B = malloc((size_t)size * sizeof(double));
    double *C_exact = compare_errors ? calloc((size_t)size, sizeof(double)) : NULL;
    double *C_approx = calloc((size_t)size, sizeof(double));

    if (!A || !B || (compare_errors && !C_exact) || !C_approx) {
        fprintf(stderr, "Erro: falha de alocacao.\n");
        free(A);
        free(B);
        free(C_exact);
        free(C_approx);
        return 1;
    }

    for (long long i = 0; i < size; i++) {
        A[i] = (double)rand() / (double)RAND_MAX;
        B[i] = (double)rand() / (double)RAND_MAX;
    }

    if (compare_errors) {
        dgemm_blocked_exact(N, BS, A, B, C_exact);
    }

    struct timespec start, end;

    clock_gettime(CLOCK_MONOTONIC, &start);

    int ok = 1;

    if (approx == APPROX_FLOAT) {
        ok = dgemm_float(N, BS, A, B, C_approx);
    } else {
        dgemm_skip_k(N, BS, A, B, C_approx);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    if (!ok) {
        fprintf(stderr, "Erro: falha de alocacao durante aproximacao.\n");
        free(A);
        free(B);
        free(C_exact);
        free(C_approx);
        return 1;
    }

    double tempo = (end.tv_sec - start.tv_sec) +
                   (end.tv_nsec - start.tv_nsec) / 1e9;
    double checksum;
    double erro_abs;
    double erro_rel;
    double rmse;
    double erro_max;

    if (compare_errors) {
        calcula_metricas(C_exact, C_approx, size, &checksum, &erro_abs, &erro_rel, &rmse, &erro_max);
    } else {
        checksum = 0.0;
        for (long long i = 0; i < size; i++) {
            checksum += C_approx[i];
        }
        erro_abs = 0.0;
        erro_rel = 0.0;
        rmse = 0.0;
        erro_max = 0.0;
    }

    printf("N=%d BS=%d approx=%s seed=%d Tempo=%.6f s\n", N, BS, argv[3], seed, tempo);
    printf("Checksum %.6f\n", checksum);
    printf("ErrorAbsMean %.12f\n", erro_abs);
    printf("ErrorRelMean %.12f\n", erro_rel);
    printf("RMSE %.12f\n", rmse);
    printf("ErrorMax %.12f\n", erro_max);

    free(A);
    free(B);
    free(C_exact);
    free(C_approx);

    return 0;
}
