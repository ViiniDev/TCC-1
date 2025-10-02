#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <sys/time.h>
#endif

// -------------------- Temporizador --------------------
#ifdef _WIN32
double get_time_ms() {
    static LARGE_INTEGER freq;
    static int init = 0;
    if (!init) {
        QueryPerformanceFrequency(&freq);
        init = 1;
    }
    LARGE_INTEGER t;
    QueryPerformanceCounter(&t);
    return (double)t.QuadPart * 1000.0 / (double)freq.QuadPart;
}
#else
double get_time_ms() {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (double)t.tv_sec * 1000.0 + (double)t.tv_nsec / 1.0e6;
}
#endif

// -------------------- Geradores Aleatórios --------------------
static inline double rand_uniform() {
    return (double)rand() / (double)RAND_MAX;
}

static inline double rand_exponential() {
    double u = rand_uniform();
    return -log(1.0 - u);
}

static inline double rand_normal() {
    double u1 = rand_uniform();
    double u2 = rand_uniform();
    return sqrt(-2.0 * log(u1)) * cos(2 * M_PI * u2);
}

// -------------------- Convolução --------------------
void convolution(double *matrix, double *kernel, double *output, int n, int k) {
    int pad = k / 2;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            double sum = 0.0;
            for (int ki = 0; ki < k; ki++) {
                for (int kj = 0; kj < k; kj++) {
                    int x = i + ki - pad;
                    int y = j + kj - pad;
                    if (x >= 0 && x < n && y >= 0 && y < n) {
                        sum += matrix[x * n + y] * kernel[ki * k + kj];
                    }
                }
            }
            output[i * n + j] = sum;
        }
    }
}

// -------------------- Programa Principal --------------------
int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("Uso: %s <tamanho_matriz> <tipo_distribuicao> <tamanho_kernel>\n", argv[0]);
        printf("Distribuições: 0=Uniforme, 1=Exponencial, 2=Normal\n");
        return 1;
    }

    int n = atoi(argv[1]);
    int dist = atoi(argv[2]);
    int k = atoi(argv[3]);

    srand(time(NULL));

    double *matrix = malloc(n * n * sizeof(double));
    double *kernel = malloc(k * k * sizeof(double));
    double *output = malloc(n * n * sizeof(double));

    if (!matrix || !kernel || !output) {
        printf("Erro ao alocar memória!\n");
        return 1;
    }

    // Preencher matriz com valores aleatórios
    for (int i = 0; i < n * n; i++) {
        if (dist == 0)
            matrix[i] = rand_uniform();
        else if (dist == 1)
            matrix[i] = rand_exponential();
        else
            matrix[i] = rand_normal();
    }

    // Preencher kernel com valores uniformes
    for (int i = 0; i < k * k; i++) {
        kernel[i] = rand_uniform();
    }

    double start = get_time_ms();
    convolution(matrix, kernel, output, n, k);
    double end = get_time_ms();

    printf("Convolução concluída em %.3f ms\n", end - start);

    free(matrix);
    free(kernel);
    free(output);

    return 0;
}
