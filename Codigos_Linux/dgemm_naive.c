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

    int N = atoi(argv[1]);

    double *A = malloc(N*N*sizeof(double));
    double *B = malloc(N*N*sizeof(double));
    double *C = calloc(N*N,sizeof(double));

    for (long long i=0;i<(long long)N*N;i++) {
        A[i] = (double)rand()/RAND_MAX;
        B[i] = (double)rand()/RAND_MAX;
    }

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC,&start);

    int BS = 32;  // tamanho do bloco

    dgemm_blocked(N, BS, A, B, C);

    clock_gettime(CLOCK_MONOTONIC,&end);

    double tempo = (end.tv_sec - start.tv_sec) +
                   (end.tv_nsec - start.tv_nsec)/1e9;

    printf("N=%d Tempo=%.6f s\n",N,tempo);

    free(A); free(B); free(C);
}
