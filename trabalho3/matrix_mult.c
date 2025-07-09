#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define BLOCK_SIZE 64  // Ajustável conforme tamanho de cache
#define EPSILON 1e-9

void transpose_matrix(int N, const double *b, double *bt) {
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            bt[j*N + i] = b[i*N + j];
}

void optimized_matrix_multiply(int N, const double *a, const double *b, double *c) {
    // Transpor B para melhorar localidade de acesso
    double *bt = malloc(N * N * sizeof(double));
    transpose_matrix(N, b, bt);

    // Zerar C (importante, pois vamos acumular valores)
    memset(c, 0, N * N * sizeof(double));

    for (int ii = 0; ii < N; ii += BLOCK_SIZE)
        for (int jj = 0; jj < N; jj += BLOCK_SIZE)
            for (int kk = 0; kk < N; kk += BLOCK_SIZE)

                for (int i = ii; i < ii + BLOCK_SIZE && i < N; i++)
                    for (int j = jj; j < jj + BLOCK_SIZE && j < N; j++) {
                        double sum = 0.0;
                        int k;

                        // Desenrolamento de loop no k
                        for (k = kk; k <= kk + BLOCK_SIZE - 4 && k + 3 < N; k += 4) {
                            sum += a[i*N + k]     * bt[j*N + k];
                            sum += a[i*N + k + 1] * bt[j*N + k + 1];
                            sum += a[i*N + k + 2] * bt[j*N + k + 2];
                            sum += a[i*N + k + 3] * bt[j*N + k + 3];
                        }
                        // Loop restante
                        for (; k < kk + BLOCK_SIZE && k < N; k++)
                            sum += a[i*N + k] * bt[j*N + k];

                        c[i*N + j] += sum;
                    }

    free(bt);
}

// Ingênua
void naive_matrix_multiply(int N, const double *a, const double *b, double *c) {
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++) {
            double sum = 0.0;
            for (int k = 0; k < N; k++)
                sum += a[i*N + k] * b[k*N + j];
            c[i*N + j] = sum;
        }
}

// Verifica se duas matrizes são iguais (com tolerância)
int compare_matrices(int N, const double *c1, const double *c2) {
    for (int i = 0; i < N * N; i++) {
        if (fabs(c1[i] - c2[i]) > EPSILON) {
            printf("Mismatch at index %d: %.12f vs %.12f\n", i, c1[i], c2[i]);
            return 0;
        }
    }
    return 1;
}

int main() {
    int N = 1024;  // Tamanho da matriz
    double *A = malloc(N * N * sizeof(double));
    double *B = malloc(N * N * sizeof(double));
    double *C_naive = malloc(N * N * sizeof(double));
    double *C_opt = malloc(N * N * sizeof(double));

    // Inicializar A, B com dados desejados...
    for (int i = 0; i < N*N; i++) {
        A[i] = 1.0;
        B[i] = 2.0;
    }

    clock_t start_naive = clock();
    naive_matrix_multiply(N, A, B, C_naive);
    clock_t end_naive = clock();
    double time_naive = (double)(end_naive - start_naive) / CLOCKS_PER_SEC;
    
    clock_t start_opt = clock();
    optimized_matrix_multiply(N, A, B, C_opt);
    clock_t end_opt = clock();
    double time_opt = (double)(end_opt - start_opt) / CLOCKS_PER_SEC;

    // 🖨️ Resultados
    printf("Tempo versão ingênua     : %.6f segundos\n", time_naive);
    printf("Tempo versão otimizada   : %.6f segundos\n", time_opt);

    // (Opcional) imprimir ou verificar C
    if (compare_matrices(N, C_naive, C_opt))
        printf("As matrizes coincidem! Implementação correta.\n");
    else
        printf("As matrizes são diferentes! Verifique a otimização.\n");

    free(A); free(B); free(C_naive); free(C_opt);
    return 0;
}

