#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <windows.h>  // para QueryPerformanceCounter no Windows

// Função para medir tempo de execução no Windows
double get_time() {
    LARGE_INTEGER freq, counter;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&counter);
    return (double) counter.QuadPart / (double) freq.QuadPart;
}

// Uniforme entre 0 e 1
static inline double rand_uniform() {
    return (rand() + 1.0) / (RAND_MAX + 2.0);
}

// Normal (Box-Muller)
static inline double rand_normal() {
    double u1 = rand_uniform();
    double u2 = rand_uniform();
    return sqrt(-2.0 * log(u1)) * cos(2.0 * M_PI * u2);
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("Uso: %s <N> <mu> <sigma>\n", argv[0]);
        return 1;
    }

    int N = atoi(argv[1]);
    double mu = atof(argv[2]);
    double sigma = atof(argv[3]);

    srand((unsigned int)time(NULL));

    double start = get_time();

    for (int i = 0; i < N; i++) {
        double x = mu + sigma * rand_normal();
        if (i < 10)  // imprime só os 10 primeiros
            printf("%f\n", x);
    }

    double end = get_time();
    printf("Tempo: %f segundos\n", end - start);

    return 0;
}
