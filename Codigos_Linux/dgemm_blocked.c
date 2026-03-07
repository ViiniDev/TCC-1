#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void dgemm_blocked(int N, int BS, double *A, double *B, double *C) {

    for (int ii = 0; ii < N; ii += BS) {
        for (int jj = 0; jj < N; jj += BS) {
            for (int kk = 0; kk < N; kk += BS) {

                for (int i = ii; i < ii + BS && i < N; i++) {
                    for (int j = jj; j < jj + BS && j < N; j++) {

                        double sum = C[i*N + j];

                        for (int k = kk; k < kk + BS && k < N; k++) {
                            sum += A[i*N + k] * B[k*N + j];
                        }

                        C[i*N + j] = sum;
                    }
                }

            }
        }
    }
}

int main(int argc, char *argv[]) {

    if(argc < 3){
        printf("Uso: %s <N> <BS>\n", argv[0]);
        return 1;
    }

    int N = atoi(argv[1]);
    int BS = atoi(argv[2]);

    srand(time(NULL));

    long long size = (long long)N * N;

    double *A = malloc(size * sizeof(double));
    double *B = malloc(size * sizeof(double));
    double *C = calloc(size, sizeof(double));

    if(!A || !B || !C){
        printf("Erro de alocação\n");
        return 1;
    }

    for(long long i = 0; i < size; i++){
        A[i] = (double)rand()/RAND_MAX;
        B[i] = (double)rand()/RAND_MAX;
    }

    struct timespec start, end;

    clock_gettime(CLOCK_MONOTONIC, &start);

    dgemm_blocked(N, BS, A, B, C);

    clock_gettime(CLOCK_MONOTONIC, &end);

    double tempo = (end.tv_sec - start.tv_sec) +
                   (end.tv_nsec - start.tv_nsec) / 1e9;

    // cálculo de FLOPs
    double flops = 2.0 * N * N * N;

    // cálculo de GFLOPS
    double gflops = flops / (tempo * 1e9);

    printf("N=%d BS=%d Tempo=%.6f s GFLOPS=%.2f\n", N, BS, tempo, gflops);

    free(A);
    free(B);
    free(C);

    return 0;
}