#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include <math.h>

int primo (long int n) { /* mpi_primos.c  */
	int i;
       
	for (i = 3; i < (int)(sqrt(n) + 1); i+=2) {
			if(n%i == 0) return 0;
	}
	return 1;
}

int main(int argc, char *argv[]) {
	double t_inicial, t_final, t_final_paralelo;
	int cont = 0, total = 0;
	long int i, n;
	int meu_ranque, num_procs, inicio, salto;

	if (argc < 2) {
        	printf("Valor inválido! Entre com um valor do maior inteiro\n");
       	 	return 0;
    	} else {
        	n = strtol(argv[1], (char **) NULL, 10);
       	}

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &meu_ranque);
	MPI_Comm_size(MPI_COMM_WORLD, &num_procs);	
    t_inicial = MPI_Wtime();
    inicio = 3 + meu_ranque*2;
    salto = num_procs*2;
	for (i = inicio; i <= n; i += salto) 
	{	
		if(primo(i) == 1) cont++;
	}
    t_final_paralelo = MPI_Wtime();
		
	if(num_procs > 1) {
		MPI_Reduce(&cont, &total, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
	} else {
		total = cont;
	}
	
	t_final = MPI_Wtime();

	if (meu_ranque == 0) {
        total += 1;    /* Acrescenta o dois, que também é primo */
		printf("Quant. de primos entre 1 e n: %d \n", total);
        double t_exec = t_final - t_inicial;
        double t_para = t_final_paralelo - t_inicial;
		printf("Tempo de execucao: %1.3f \n", t_exec);
        printf("Fracao tempo sequencial: %1.3f (t_para = %1.3f)\n", 1 - (t_para/t_exec), t_para);
	}
	MPI_Finalize();
	return(0);
}