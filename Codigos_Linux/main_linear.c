#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

typedef enum { UNIFORME, NORMAL, EXPONENCIAL } dist_t;

double rand_uniform() {
    return (double)rand() / RAND_MAX;
}

double rand_normal() {
    double u1 = rand_uniform();
    double u2 = rand_uniform();
    return sqrt(-2.0 * log(u1)) * cos(2 * M_PI * u2);
}

double rand_exponencial(double lambda) {
    double u = rand_uniform();
    return -log(1.0 - u) / lambda;
}

double** gera_matriz(int N, dist_t tipo) {

    double **mat = malloc(N * sizeof(double*));

    for(int i=0;i<N;i++){

        mat[i] = malloc(N*sizeof(double));

        for(int j=0;j<N;j++){

            switch(tipo){
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

double** gera_kernel(int K){

    double **ker = malloc(K*sizeof(double*));

    double soma = 0.0;

    for(int i=0;i<K;i++){

        ker[i] = malloc(K*sizeof(double));

        for(int j=0;j<K;j++){

            ker[i][j] = rand_uniform();

            soma += ker[i][j];
        }
    }

    for(int i=0;i<K;i++)
        for(int j=0;j<K;j++)
            ker[i][j] /= soma;

    return ker;
}

double** convolucao(double **mat, int N, double **ker, int K){

    int offset = K/2;

    double **saida = malloc(N*sizeof(double*));

    for(int i=0;i<N;i++)
        saida[i] = calloc(N,sizeof(double));

    for(int i=offset;i<N-offset;i++){

        for(int j=offset;j<N-offset;j++){

            double soma = 0.0;

            for(int ki=0;ki<K;ki++)
                for(int kj=0;kj<K;kj++)
                    soma += mat[i-offset+ki][j-offset+kj]*ker[ki][kj];

            saida[i][j] = soma;
        }
    }

    return saida;
}

int main(int argc, char *argv[]) {

    if(argc < 4){
        printf("Uso: %s <N> <dist:0,1,2> <K>\n",argv[0]);
        return 1;
    }

    int N = atoi(argv[1]);
    dist_t tipo = atoi(argv[2]);
    int K = atoi(argv[3]);

    srand(time(NULL));

    struct timespec start,end;

    clock_gettime(CLOCK_MONOTONIC,&start);

    double **mat = gera_matriz(N,tipo);
    double **ker = gera_kernel(K);

    double **saida = convolucao(mat,N,ker,K);

    clock_gettime(CLOCK_MONOTONIC,&end);

    double tempo = (end.tv_sec-start.tv_sec)+
                   (end.tv_nsec-start.tv_nsec)/1e9;

    printf("N=%d dist=%d K=%d Tempo=%.6f s\n",N,tipo,K,tempo);

    double checksum = 0.0;

    for(int i=0;i<N;i++)
        for(int j=0;j<N;j++)
            checksum += saida[i][j];

    printf("Checksum %.6f\n",checksum);

    for(int i=0;i<N;i++){
        free(mat[i]);
        free(saida[i]);
    }

    for(int i=0;i<K;i++)
        free(ker[i]);

    free(mat);
    free(saida);
    free(ker);

    return 0;
}