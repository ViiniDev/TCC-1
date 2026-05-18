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

static void libera_matriz_double(double **mat, int rows) {
    if (!mat) {
        return;
    }

    for (int i = 0; i < rows; i++) {
        free(mat[i]);
    }

    free(mat);
}

static void libera_matriz_float(float **mat, int rows) {
    if (!mat) {
        return;
    }

    for (int i = 0; i < rows; i++) {
        free(mat[i]);
    }

    free(mat);
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

static double **aloca_matriz_double(int rows, int cols, int zero) {
    double **mat = calloc((size_t)rows, sizeof(double *));

    if (!mat) {
        return NULL;
    }

    for (int i = 0; i < rows; i++) {
        mat[i] = zero ? calloc((size_t)cols, sizeof(double)) : malloc((size_t)cols * sizeof(double));

        if (!mat[i]) {
            libera_matriz_double(mat, rows);
            return NULL;
        }
    }

    return mat;
}

static float **aloca_matriz_float(int rows, int cols, int zero) {
    float **mat = calloc((size_t)rows, sizeof(float *));

    if (!mat) {
        return NULL;
    }

    for (int i = 0; i < rows; i++) {
        mat[i] = zero ? calloc((size_t)cols, sizeof(float)) : malloc((size_t)cols * sizeof(float));

        if (!mat[i]) {
            libera_matriz_float(mat, rows);
            return NULL;
        }
    }

    return mat;
}

static double **gera_matriz(int N, dist_t tipo) {
    double **mat = aloca_matriz_double(N, N, 0);

    if (!mat) {
        return NULL;
    }

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            switch (tipo) {
                case UNIFORME:
                    mat[i][j] = rand_uniform();
                    break;
                case NORMAL:
                    mat[i][j] = rand_normal();
                    break;
                case EXPONENCIAL:
                    mat[i][j] = rand_exponencial(1.0);
                    break;
            }
        }
    }

    return mat;
}

static double **gera_kernel(int K) {
    double **ker = aloca_matriz_double(K, K, 0);
    double soma = 0.0;

    if (!ker) {
        return NULL;
    }

    for (int i = 0; i < K; i++) {
        for (int j = 0; j < K; j++) {
            ker[i][j] = rand_uniform();
            soma += ker[i][j];
        }
    }

    if (soma == 0.0) {
        libera_matriz_double(ker, K);
        return NULL;
    }

    for (int i = 0; i < K; i++) {
        for (int j = 0; j < K; j++) {
            ker[i][j] /= soma;
        }
    }

    return ker;
}

static double **convolucao_exata(double **mat, int N, double **ker, int K) {
    int offset = K / 2;
    double **saida = aloca_matriz_double(N, N, 1);

    if (!saida) {
        return NULL;
    }

    for (int i = offset; i < N - offset; i++) {
        for (int j = offset; j < N - offset; j++) {
            double soma = 0.0;

            for (int ki = 0; ki < K; ki++) {
                for (int kj = 0; kj < K; kj++) {
                    soma += mat[i - offset + ki][j - offset + kj] * ker[ki][kj];
                }
            }

            saida[i][j] = soma;
        }
    }

    return saida;
}

static double **convolucao_float(double **mat, int N, double **ker, int K) {
    int offset = K / 2;
    float **mat_f = aloca_matriz_float(N, N, 0);
    float **ker_f = aloca_matriz_float(K, K, 0);
    double **saida = aloca_matriz_double(N, N, 1);

    if (!mat_f || !ker_f || !saida) {
        libera_matriz_float(mat_f, N);
        libera_matriz_float(ker_f, K);
        libera_matriz_double(saida, N);
        return NULL;
    }

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            mat_f[i][j] = (float)mat[i][j];
        }
    }

    for (int i = 0; i < K; i++) {
        for (int j = 0; j < K; j++) {
            ker_f[i][j] = (float)ker[i][j];
        }
    }

    for (int i = offset; i < N - offset; i++) {
        for (int j = offset; j < N - offset; j++) {
            float soma = 0.0f;

            for (int ki = 0; ki < K; ki++) {
                for (int kj = 0; kj < K; kj++) {
                    soma += mat_f[i - offset + ki][j - offset + kj] * ker_f[ki][kj];
                }
            }

            saida[i][j] = (double)soma;
        }
    }

    libera_matriz_float(mat_f, N);
    libera_matriz_float(ker_f, K);
    return saida;
}

static double **convolucao_skip_kernel(double **mat, int N, double **ker, int K) {
    int offset = K / 2;
    double selected_sum = 0.0;
    double **saida = aloca_matriz_double(N, N, 1);

    if (!saida) {
        return NULL;
    }

    for (int ki = 0; ki < K; ki += 2) {
        for (int kj = 0; kj < K; kj += 2) {
            selected_sum += ker[ki][kj];
        }
    }

    if (selected_sum == 0.0) {
        libera_matriz_double(saida, N);
        return NULL;
    }

    for (int i = offset; i < N - offset; i++) {
        for (int j = offset; j < N - offset; j++) {
            double soma = 0.0;

            for (int ki = 0; ki < K; ki += 2) {
                for (int kj = 0; kj < K; kj += 2) {
                    soma += mat[i - offset + ki][j - offset + kj] * (ker[ki][kj] / selected_sum);
                }
            }

            saida[i][j] = soma;
        }
    }

    return saida;
}

static void calcula_metricas(double **exato, double **aprox, int N,
                             double *checksum, double *erro_abs, double *erro_rel,
                             double *rmse, double *erro_max) {
    double soma_abs = 0.0;
    double soma_rel = 0.0;
    double soma_quad = 0.0;
    double max_abs = 0.0;
    long long size = (long long)N * N;

    *checksum = 0.0;

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            double diff = fabs(aprox[i][j] - exato[i][j]);
            double denom = fabs(exato[i][j]) > 1e-12 ? fabs(exato[i][j]) : 1e-12;

            *checksum += aprox[i][j];
            soma_abs += diff;
            soma_rel += diff / denom;
            soma_quad += diff * diff;

            if (diff > max_abs) {
                max_abs = diff;
            }
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

    double **mat = gera_matriz(N, (dist_t)dist);
    double **ker = gera_kernel(K);

    if (!mat || !ker) {
        libera_matriz_double(mat, N);
        libera_matriz_double(ker, K);
        return 1;
    }

    double **exato = compare_errors ? convolucao_exata(mat, N, ker, K) : NULL;

    if (compare_errors && !exato) {
        libera_matriz_double(mat, N);
        libera_matriz_double(ker, K);
        return 1;
    }

    struct timespec start, end;

    clock_gettime(CLOCK_MONOTONIC, &start);

    double **aprox = approx == APPROX_FLOAT
                         ? convolucao_float(mat, N, ker, K)
                         : convolucao_skip_kernel(mat, N, ker, K);

    clock_gettime(CLOCK_MONOTONIC, &end);

    if (!aprox) {
        libera_matriz_double(mat, N);
        libera_matriz_double(ker, K);
        libera_matriz_double(exato, N);
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
        calcula_metricas(exato, aprox, N, &checksum, &erro_abs, &erro_rel, &rmse, &erro_max);
    } else {
        checksum = 0.0;
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                checksum += aprox[i][j];
            }
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

    libera_matriz_double(mat, N);
    libera_matriz_double(ker, K);
    libera_matriz_double(exato, N);
    libera_matriz_double(aprox, N);

    return 0;
}
