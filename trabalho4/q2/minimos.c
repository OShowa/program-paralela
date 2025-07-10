#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(int argc, char** argv) {
    int rank, size, N;
    double *x = NULL, *y = NULL;
    double local_sum_x = 0, local_sum_y = 0, local_sum_x2 = 0, local_sum_xy = 0;
    double sum_x, sum_y, sum_x2, sum_xy;

    double t_total_start, t_total_end;
    double t_file_start, t_file_end;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    t_total_start = MPI_Wtime();

    if (rank == 0) {
        if (argc < 2) {
            fprintf(stderr, "Uso: %s <arquivo_dados.txt>\n", argv[0]);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        t_file_start = MPI_Wtime();

        FILE *f = fopen(argv[1], "r");
        if (!f) {
            perror("Erro ao abrir o arquivo");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        fscanf(f, "%d", &N);
        x = malloc(N * sizeof(double));
        y = malloc(N * sizeof(double));
        for (int i = 0; i < N; i++) {
            fscanf(f, "%lf %lf", &x[i], &y[i]);
        }
        fclose(f);

        t_file_end = MPI_Wtime();
        printf("Tempo de leitura do arquivo: %.6f segundos\n", t_file_end - t_file_start);
    }

    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Alocar memória nos processos slaves
    if (rank != 0) {
        x = malloc(N * sizeof(double));
        y = malloc(N * sizeof(double));
    }

    // Broadcast dos vetores
    MPI_Bcast(x, N, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Bcast(y, N, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Cada processo trata um bloco
    int start = rank * (N / size);
    int end = (rank == size - 1) ? N : start + (N / size);

    for (int i = start; i < end; i++) {
        local_sum_x += x[i];
        local_sum_y += y[i];
        local_sum_x2 += x[i] * x[i];
        local_sum_xy += x[i] * y[i];
    }

    // Redução para somar os parciais
    MPI_Reduce(&local_sum_x, &sum_x, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&local_sum_y, &sum_y, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&local_sum_x2, &sum_x2, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&local_sum_xy, &sum_xy, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        double a = (N * sum_xy - sum_x * sum_y) / (N * sum_x2 - sum_x * sum_x);
        double b = (sum_y - a * sum_x) / N;
        printf("Equação ajustada: y = %.5lf * x + %.5lf\n", a, b);

        t_total_end = MPI_Wtime(); // Fim da execução total
        printf("Tempo total de execução MPI: %.6f segundos\n", t_total_end - t_total_start);
    }

    free(x);
    free(y);
    MPI_Finalize();
    return 0;
}
