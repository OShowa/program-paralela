#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int compare(const void *a, const void *b) {
    int int_a = *(int*)a;
    int int_b = *(int*)b;
    return (int_a > int_b) - (int_a < int_b);
}

int* merge_sorted_arrays(int *data, int chunk_size, int num_chunks) {
    int *output = malloc(chunk_size * num_chunks * sizeof(int));
    int *indices = calloc(num_chunks, sizeof(int));
    for (int i = 0; i < chunk_size * num_chunks; ++i) {
        int min_val = __INT_MAX__;
        int min_idx = -1;
        for (int j = 0; j < num_chunks; ++j) {
            if (indices[j] < chunk_size) {
                int val = data[j * chunk_size + indices[j]];
                if (val < min_val) {
                    min_val = val;
                    min_idx = j;
                }
            }
        }
        output[i] = min_val;
        indices[min_idx]++;
    }
    free(indices);
    return output;
}

int main(int argc, char** argv) {
    int N = 400000000; // tamanho do vetor
    int *data = NULL;
    int *local_data;
    int world_size, world_rank;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int chunk_size = (N / world_size) + 1;

    if (world_rank == 0) {
        data = malloc(N * sizeof(int));
        for (int i = 0; i < N; ++i)
            data[i] = rand() % 1000000;
    }

    local_data = malloc(chunk_size * sizeof(int));
    
    double start_time = MPI_Wtime();
    
    MPI_Scatter(data, chunk_size, MPI_INT, local_data, chunk_size, MPI_INT, 0, MPI_COMM_WORLD);
    
    qsort(local_data, chunk_size, sizeof(int), compare);
    
    MPI_Gather(local_data, chunk_size, MPI_INT, data, chunk_size, MPI_INT, 0, MPI_COMM_WORLD);
    
    if (world_rank == 0) {
        int *sorted = merge_sorted_arrays(data, chunk_size, world_size);
        double end_time = MPI_Wtime();
        printf("Tempo total de execução: %.6f segundos\n", end_time - start_time);

        free(sorted);
        free(data);
    }

    free(local_data);
    MPI_Finalize();
    return 0;
}
