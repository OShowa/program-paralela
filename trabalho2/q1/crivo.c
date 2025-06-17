#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

#define N_VALUE 15000

int main() {
    int N = N_VALUE;

    int omp_threads = DEFAULT_OMP_THREADS;
    if (argc > 1)
    {
        omp_threads = atoi(argv[1]);
        if (omp_threads < 1)
            omp_threads = DEFAULT_OMP_THREADS;
    }

    omp_set_num_threads(omp_threads);

    char *primos = malloc((N + 1) * sizeof(char));
    if (primos == NULL) {
        printf("Erro de alocação de memória.\n");
        return 1;
    }

    for (int i = 0; i <= N; i++) {
        primos[i] = 1;
    }

    primos[0] = 0;
    primos[1] = 0;

    int limite = (int) sqrt(N);

    #pragma omp parallel for schedule(dynamic)
    for (int i = 2; i <= limite; i++) {
        if (primos[i]) {
            #pragma omp parallel for schedule(dynamic)
            for (int j = i * i; j <= N; j += i) {
                primos[j] = 0;
            }
        }
    }

    printf("Numeros primos ate %d:\n", N);
    for (int i = 2; i <= N; i++) {
        if (primos[i]) {
            printf("%d ", i);
        }
    }
    printf("\n");

    free(primos);
    return 0;
}
