#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef enum { UNIFORME, NORMAL, EXPONENCIAL } dist_t;

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

static int valida_argumentos(int N, int dist, int K) {
    if (N <= 0) {
        fprintf(stderr, "Erro: N deve ser maior que zero.\n");
        return 0;
    }

    if (dist < UNIFORME || dist > EXPONENCIAL) {
        fprintf(stderr, "Erro: dist deve ser 0=uniforme, 1=normal ou 2=exponencial.\n");
        return 0;
    }

    if (K <= 0 || K % 2 == 0) {
        fprintf(stderr, "Erro: K deve ser positivo e impar.\n");
        return 0;
    }

    if (K > N) {
        fprintf(stderr, "Erro: K nao pode ser maior que N.\n");
        return 0;
    }

    return 1;
}

static void libera_matriz(double **mat, int rows) {
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

static double **gera_matriz(int N, dist_t tipo) {
    double **mat = calloc((size_t)N, sizeof(double *));

    if (!mat) {
        fprintf(stderr, "Erro: falha ao alocar ponteiros da matriz.\n");
        return NULL;
    }

    for (int i = 0; i < N; i++) {
        mat[i] = malloc((size_t)N * sizeof(double));

        if (!mat[i]) {
            fprintf(stderr, "Erro: falha ao alocar linha da matriz.\n");
            libera_matriz(mat, N);
            return NULL;
        }

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
    double **ker = calloc((size_t)K, sizeof(double *));
    double soma = 0.0;

    if (!ker) {
        fprintf(stderr, "Erro: falha ao alocar ponteiros do kernel.\n");
        return NULL;
    }

    for (int i = 0; i < K; i++) {
        ker[i] = malloc((size_t)K * sizeof(double));

        if (!ker[i]) {
            fprintf(stderr, "Erro: falha ao alocar linha do kernel.\n");
            libera_matriz(ker, K);
            return NULL;
        }

        for (int j = 0; j < K; j++) {
            ker[i][j] = rand_uniform();
            soma += ker[i][j];
        }
    }

    if (soma == 0.0) {
        fprintf(stderr, "Erro: soma do kernel igual a zero.\n");
        libera_matriz(ker, K);
        return NULL;
    }

    for (int i = 0; i < K; i++) {
        for (int j = 0; j < K; j++) {
            ker[i][j] /= soma;
        }
    }

    return ker;
}

static double **convolucao(double **mat, int N, double **ker, int K) {
    int offset = K / 2;
    double **saida = calloc((size_t)N, sizeof(double *));

    if (!saida) {
        fprintf(stderr, "Erro: falha ao alocar ponteiros da saida.\n");
        return NULL;
    }

    for (int i = 0; i < N; i++) {
        saida[i] = calloc((size_t)N, sizeof(double));

        if (!saida[i]) {
            fprintf(stderr, "Erro: falha ao alocar linha da saida.\n");
            libera_matriz(saida, N);
            return NULL;
        }
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

int main(int argc, char *argv[]) {
    int N;
    int dist;
    int K;
    int seed;
    double **mat = NULL;
    double **ker = NULL;
    double **saida = NULL;

    if (argc != 4 && argc != 5) {
        fprintf(stderr, "Uso: %s <N> <dist:0,1,2> <K> [seed]\n", argv[0]);
        return 1;
    }

    if (!parse_int(argv[1], &N) || !parse_int(argv[2], &dist) || !parse_int(argv[3], &K)) {
        fprintf(stderr, "Erro: argumentos devem ser numeros inteiros validos.\n");
        return 1;
    }

    if (!valida_argumentos(N, dist, K)) {
        return 1;
    }

    if (argc == 5) {
        if (!parse_int(argv[4], &seed)) {
            fprintf(stderr, "Erro: seed deve ser um numero inteiro valido.\n");
            return 1;
        }
    } else {
        seed = (int)time(NULL);
    }

    srand((unsigned int)seed);

    struct timespec start, end;

    mat = gera_matriz(N, (dist_t)dist);
    ker = gera_kernel(K);

    if (!mat || !ker) {
        libera_matriz(mat, N);
        libera_matriz(ker, K);
        return 1;
    }

    clock_gettime(CLOCK_MONOTONIC, &start);

    saida = convolucao(mat, N, ker, K);

    if (!saida) {
        libera_matriz(mat, N);
        libera_matriz(ker, K);
        return 1;
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    double tempo = (end.tv_sec - start.tv_sec) +
                   (end.tv_nsec - start.tv_nsec) / 1e9;

    printf("N=%d dist=%d K=%d seed=%d Tempo=%.6f s\n", N, dist, K, seed, tempo);

    double checksum = 0.0;

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            checksum += saida[i][j];
        }
    }

    printf("Checksum %.6f\n", checksum);

    libera_matriz(mat, N);
    libera_matriz(ker, K);
    libera_matriz(saida, N);

    return 0;
}
