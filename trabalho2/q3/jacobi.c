#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <omp.h>

#define COLUMNS 1022
#define ROWS 1022
#define MAX_TEMP_ERROR 0.01
#define DEFAULT_OMP_THREADS 1


double Anew[ROWS + 2][COLUMNS + 2];
double A[ROWS + 2][COLUMNS + 2];

void iniciar();

int main(int argc, char *argv[])
{ /* jacobi_seq.c  */
    int i, j;
    int max_iterations = 4098;
    int iteration = 1;
    double dt = 100;

    int omp_threads = DEFAULT_OMP_THREADS;
    if (argc > 1)
    {
        omp_threads = atoi(argv[1]);
        if (omp_threads < 1)
            omp_threads = DEFAULT_OMP_THREADS;
    }

    omp_set_num_threads(omp_threads);

    double start_time, end_time;
    start_time = omp_get_wtime();

    iniciar();
    while (dt > MAX_TEMP_ERROR && iteration <= max_iterations)
    {
        #pragma omp parallel for private(j) shared(A, Anew) schedule(static)
        for (i = 1; i <= ROWS; i++)
            for (j = 1; j <= COLUMNS; j++)
            {
                Anew[i][j] = 0.25 * (A[i + 1][j] +
                                     A[i - 1][j] + A[i][j + 1] + A[i][j - 1]);
            }
        dt = 0.0;

        #pragma omp parallel for private(j) reduction(max:dt) shared(A, Anew) schedule(static)
        for (i = 1; i <= ROWS; i++)
            for (j = 1; j <= COLUMNS; j++)
            {
                dt = fmax(fabs(Anew[i][j] - A[i][j]), dt);
                A[i][j] = Anew[i][j];
            }
        iteration++;
    }

    end_time = omp_get_wtime();

    printf("\n Erro maximo na iteracao %d era %f\n", iteration - 1, dt);
    printf("Elapsed time: %f seconds\n", end_time - start_time);

    return (0);
}

void iniciar()
{
    int i, j;

    #pragma omp parallel for private(j) shared(A) schedule(static)
    for (i = 0; i <= ROWS + 1; i++)
    {
        for (j = 0; j <= COLUMNS + 1; j++)
        {
            A[i][j] = 20.0;
        }
    }

    A[800][800] = 100.0;
}
