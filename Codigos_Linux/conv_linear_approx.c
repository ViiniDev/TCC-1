#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef enum { UNIFORME, NORMAL, EXPONENCIAL } dist_t;
typedef enum { APPROX_FLOAT, APPROX_SKIP_KERNEL } approx_t;

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

    if (strcmp(text, "skip_kernel") == 0) {
        *approx = APPROX_SKIP_KERNEL;
        return 1;
    }

    return 0;
}

static int valida_argumentos(int N, int dist, int K) {
    if (N <= 0) {
        fprintf(stderr, "Erro: N deve ser maior que zero.\n");
        return 0;
    }

    if (dist < UNIFORME || dist > EXPONENCIAL) {
        fprintf(stderr, "Erro: dist deve ser 0=uniforme, 1=normal ou 2=exponencial.\n");
        return 0;
    }

    if (K <= 0 || K % 2 == 0 || K > N) {
        fprintf(stderr, "Erro: K deve ser positivo, impar e menor ou igual a N.\n");
        return 0;
    }

    return 1;
}

static double rand_uniform(void) {
    return (double)rand() / (double)RAND_MAX;
}

static double rand_normal(void) {
    double u1 = rand_uniform();
    double u2 = rand_uniform();

    if (u1 == 0.0) {
        u1 = 1.0 / (double)RAND_MAX;
    }

    return sqrt(-2.0 * log(u1)) * cos(2.0 * M_PI * u2);
}

static double rand_exponencial(double lambda) {
    double u = rand_uniform();
    return -log(1.0 - u) / lambda;
}

static double *gera_matriz(int N, dist_t tipo) {
    long long size = (long long)N * N;
    double *mat = malloc((size_t)size * sizeof(double));

    if (!mat) {
        fprintf(stderr, "Erro: falha ao alocar matriz.\n");
        return NULL;
    }

    for (long long i = 0; i < size; i++) {
        switch (tipo) {
            case UNIFORME:
                mat[i] = rand_uniform();
                break;
            case NORMAL:
                mat[i] = rand_normal();
                break;
            case EXPONENCIAL:
                mat[i] = rand_exponencial(1.0);
                break;
        }
    }

    return mat;
}

static double *gera_kernel(int K) {
    long long size = (long long)K * K;
    double *ker = malloc((size_t)size * sizeof(double));
    double soma = 0.0;

    if (!ker) {
        fprintf(stderr, "Erro: falha ao alocar kernel.\n");
        return NULL;
    }

    for (long long i = 0; i < size; i++) {
        ker[i] = rand_uniform();
        soma += ker[i];
    }

    if (soma == 0.0) {
        free(ker);
        return NULL;
    }

    for (long long i = 0; i < size; i++) {
        ker[i] /= soma;
    }

    return ker;
}

static double *convolucao_exata(const double *mat, int N, const double *ker, int K) {
    int offset = K / 2;
    long long size = (long long)N * N;
    double *saida = calloc((size_t)size, sizeof(double));

    if (!saida) {
        return NULL;
    }

    for (int i = offset; i < N - offset; i++) {
        for (int j = offset; j < N - offset; j++) {
            double soma = 0.0;

            for (int ki = 0; ki < K; ki++) {
                for (int kj = 0; kj < K; kj++) {
                    soma += mat[(i - offset + ki) * N + (j - offset + kj)] * ker[ki * K + kj];
                }
            }

            saida[i * N + j] = soma;
        }
    }

    return saida;
}

static double *convolucao_float(const double *mat, int N, const double *ker, int K) {
    long long nsize = (long long)N * N;
    long long ksize = (long long)K * K;
    float *mat_f = malloc((size_t)nsize * sizeof(float));
    float *ker_f = malloc((size_t)ksize * sizeof(float));
    double *saida = calloc((size_t)nsize, sizeof(double));
    int offset = K / 2;

    if (!mat_f || !ker_f || !saida) {
        free(mat_f);
        free(ker_f);
        free(saida);
        return NULL;
    }

    for (long long i = 0; i < nsize; i++) {
        mat_f[i] = (float)mat[i];
    }

    for (long long i = 0; i < ksize; i++) {
        ker_f[i] = (float)ker[i];
    }

    for (int i = offset; i < N - offset; i++) {
        for (int j = offset; j < N - offset; j++) {
            float soma = 0.0f;

            for (int ki = 0; ki < K; ki++) {
                for (int kj = 0; kj < K; kj++) {
                    soma += mat_f[(i - offset + ki) * N + (j - offset + kj)] * ker_f[ki * K + kj];
                }
            }

            saida[i * N + j] = (double)soma;
        }
    }

    free(mat_f);
    free(ker_f);
    return saida;
}

static double *convolucao_skip_kernel(const double *mat, int N, const double *ker, int K) {
    int offset = K / 2;
    long long size = (long long)N * N;
    double *saida = calloc((size_t)size, sizeof(double));
    double selected_sum = 0.0;

    if (!saida) {
        return NULL;
    }

    for (int ki = 0; ki < K; ki += 2) {
        for (int kj = 0; kj < K; kj += 2) {
            selected_sum += ker[ki * K + kj];
        }
    }

    if (selected_sum == 0.0) {
        free(saida);
        return NULL;
    }

    for (int i = offset; i < N - offset; i++) {
        for (int j = offset; j < N - offset; j++) {
            double soma = 0.0;

            for (int ki = 0; ki < K; ki += 2) {
                for (int kj = 0; kj < K; kj += 2) {
                    double normalized_weight = ker[ki * K + kj] / selected_sum;
                    soma += mat[(i - offset + ki) * N + (j - offset + kj)] * normalized_weight;
                }
            }

            saida[i * N + j] = soma;
        }
    }

    return saida;
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
    int dist;
    int K;
    int seed;
    int compare_errors = 1;
    approx_t approx;

    if (argc != 6 && argc != 7) {
        fprintf(stderr, "Uso: %s <N> <dist:0,1,2> <K> <approx:float|skip_kernel> <seed> [measure|compare]\n", argv[0]);
        return 1;
    }

    if (!parse_int(argv[1], &N) || !parse_int(argv[2], &dist) ||
        !parse_int(argv[3], &K) || !parse_int(argv[5], &seed)) {
        fprintf(stderr, "Erro: argumentos numericos invalidos.\n");
        return 1;
    }

    if (!parse_approx(argv[4], &approx)) {
        fprintf(stderr, "Erro: aproximacao deve ser float ou skip_kernel.\n");
        return 1;
    }

    if (!valida_argumentos(N, dist, K)) {
        return 1;
    }

    if (argc == 7) {
        if (strcmp(argv[6], "measure") == 0) {
            compare_errors = 0;
        } else if (strcmp(argv[6], "compare") == 0) {
            compare_errors = 1;
        } else {
            fprintf(stderr, "Erro: modo deve ser measure ou compare.\n");
            return 1;
        }
    }

    srand((unsigned int)seed);

    double *mat = gera_matriz(N, (dist_t)dist);
    double *ker = gera_kernel(K);

    if (!mat || !ker) {
        free(mat);
        free(ker);
        return 1;
    }

    double *exato = compare_errors ? convolucao_exata(mat, N, ker, K) : NULL;

    if (compare_errors && !exato) {
        free(mat);
        free(ker);
        return 1;
    }

    struct timespec start, end;

    clock_gettime(CLOCK_MONOTONIC, &start);

    double *aprox = approx == APPROX_FLOAT
                        ? convolucao_float(mat, N, ker, K)
                        : convolucao_skip_kernel(mat, N, ker, K);

    clock_gettime(CLOCK_MONOTONIC, &end);

    if (!aprox) {
        free(mat);
        free(ker);
        free(exato);
        return 1;
    }

    double tempo = (end.tv_sec - start.tv_sec) +
                   (end.tv_nsec - start.tv_nsec) / 1e9;
    double checksum;
    double erro_abs;
    double erro_rel;
    double rmse;
    double erro_max;
    long long size = (long long)N * N;

    if (compare_errors) {
        calcula_metricas(exato, aprox, size, &checksum, &erro_abs, &erro_rel, &rmse, &erro_max);
    } else {
        checksum = 0.0;
        for (long long i = 0; i < size; i++) {
            checksum += aprox[i];
        }
        erro_abs = 0.0;
        erro_rel = 0.0;
        rmse = 0.0;
        erro_max = 0.0;
    }

    printf("N=%d dist=%d K=%d approx=%s seed=%d Tempo=%.6f s\n", N, dist, K, argv[4], seed, tempo);
    printf("Checksum %.6f\n", checksum);
    printf("ErrorAbsMean %.12f\n", erro_abs);
    printf("ErrorRelMean %.12f\n", erro_rel);
    printf("RMSE %.12f\n", rmse);
    printf("ErrorMax %.12f\n", erro_max);

    free(mat);
    free(ker);
    free(exato);
    free(aprox);

    return 0;
}
