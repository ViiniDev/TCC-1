#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

// Tipos de distribuição
typedef enum { UNIFORME, NORMAL, EXPONENCIAL } dist_t;

// Geração uniforme [0,1]
static inline double rand_uniform() {
    return (double)rand() / RAND_MAX;
}

// Geração normal (Box-Muller)
static inline double rand_normal() {
    double u1 = rand_uniform();
    double u2 = rand_uniform();
    return sqrt(-2.0 * log(u1)) * cos(2 * M_PI * u2);
}

// Geração exponencial (lambda = 1.0)
static inline double rand_exponencial(double lambda) {
    double u = rand_uniform();
    return -log(1.0 - u) / lambda;
}

// Gera matriz NxN em vetor linear
double* gera_matriz(int N, dist_t tipo) {
    double *mat = malloc((long long)N * N * sizeof(double));
    if (!mat) {
        fprintf(stderr, "Erro: memória insuficiente para matriz %dx%d\n", N, N);
        exit(1);
    }

    for (long long i = 0; i < (long long)N * N; i++) {
        switch (tipo) {
            case UNIFORME: mat[i] = rand_uniform(); break;
            case NORMAL:   mat[i] = rand_normal();  break;
            case EXPONENCIAL: mat[i] = rand_exponencial(1.0); break;
        }
    }
    return mat;
}

// Gera kernel quadrático KxK normalizado
double* gera_kernel(int K) {
    double *ker = malloc((long long)K * K * sizeof(double));
    double soma = 0.0;

    for (int i = 0; i < K * K; i++) {
        ker[i] = rand_uniform();
        soma += ker[i];
    }
    for (int i = 0; i < K * K; i++) {
        ker[i] /= soma;
    }
    return ker;
}

// Convolução 2D usando vetores lineares
double* convolucao(double *mat, int N, double *ker, int K) {
    int offset = K / 2;
    double *saida = calloc((long long)N * N, sizeof(double));

    for (int i = offset; i < N - offset; i++) {
        for (int j = offset; j < N - offset; j++) {
            double soma = 0.0;
            for (int ki = 0; ki < K; ki++) {
                for (int kj = 0; kj < K; kj++) {
                    int mi = i - offset + ki;
                    int mj = j - offset + kj;
                    soma += mat[(long long)mi * N + mj] * ker[ki * K + kj];
                }
            }
            saida[(long long)i * N + j] = soma;
        }
    }
    return saida;
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("Uso: %s <tamanho_matriz> <tipo_distribuicao: 0=uniforme,1=normal,2=exponencial> <tamanho_kernel>\n", argv[0]);
        return 1;
    }

    int N = atoi(argv[1]);
    dist_t tipo = atoi(argv[2]);
    int K = atoi(argv[3]);

    srand(time(NULL));

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    double *mat = gera_matriz(N, tipo);
    double *ker = gera_kernel(K);
    double *saida = convolucao(mat, N, ker, K);

    clock_gettime(CLOCK_MONOTONIC, &end);

    double tempo = (end.tv_sec - start.tv_sec) +
                   (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("N=%d, Distribuicao=%d, Kernel=%d, Tempo=%.6f s\n", N, tipo, K, tempo);

    // checksum para validar resultado
    double checksum = 0.0;
    for (long long i = 0; i < (long long)N * N; i++)
        checksum += saida[i];
    printf("Checksum: %.6f\n", checksum);

    free(mat);
    free(ker);
    free(saida);

    return 0;
}
