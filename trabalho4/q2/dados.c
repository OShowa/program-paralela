#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[]) {
    FILE *f = fopen("dados.txt", "w");
    if (!f) {
        perror("Erro ao abrir arquivo");
        return 1;
    }

    int N = 100; // número de pontos padrão
    if (argc > 1) {
        N = atoi(argv[1]);
        if (N <= 0) {
            fprintf(stderr, "Valor de N inválido.\n");
            fclose(f);
            return 1;
        }
    }

    double a = 3.0, b = 2.5; // y = 3x + 2.5
    srand(time(NULL));

    fprintf(f, "%d\n", N);
    for (int i = 0; i < N; i++) {
        double x = i;
        double noise = ((rand() % 100) / 100.0 - 0.5); // erro entre -0.5 e +0.5
        double y = a * x + b + noise;
        fprintf(f, "%lf %lf\n", x, y);
    }

    fclose(f);
    return 0;
}
