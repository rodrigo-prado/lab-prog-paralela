/* http://www.revistabw.com.br/revistabw/mpi_5-c/ */
#include <semaphore.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>

#define N 912 //N fixo para evitar limitação do valor de N - BAD TERMINATION 134
#define SIZE 12 //N fixo para evitar limitação do valor de N - BAD TERMINATION 134

long a[N][N], b[N][N], c[N][N];

int m_control[SIZE];
int m_rows[SIZE];

/* --- Procedures Prototypes --- */
void multiplica(int *my_rank);
/* --- Procedures Prototypes --- */

// sem_t multiplica_mutex;			/* acesso a variavel total */

//Função que pode ser chamada no sistema para printar a matrix
//Usar para valores baixos de N
void print_matrix(long matrix[N][N]) {

	int i, j;
	for (i = 0; i < N; i++) {
		for (j = 0; j < N; j++) {
			printf("%ld ",matrix[i][j]);
		}
		printf("\n");
	}
}

//Função principal
int main (int argc, char *argv[]) {

	/* Declaração das variáveis */
	int slaves, rows, control, i, j, k, rc, mul = 5;
	struct timeval start, end;
	int col_sum = N * (N-1) / 2;

	slaves = SIZE - 1;

	//pthread_t thr0;
	pthread_t thr1, thr2, thr3, thr4, thr5, thr6, thr7, thr8, thr9, thr10, thr11;
	//int nThrID0;
	int nThrID1, nThrID2, nThrID3, nThrID4, nThrID5, nThrID6;
	int nThrID7, nThrID8, nThrID9, nThrID10, nThrID11;

	//nThrID0 = 0;
	nThrID1 = 1; nThrID2 = 2; nThrID3 = 3; nThrID4 = 4;
	nThrID5 = 5; nThrID6 = 6; nThrID7 = 7; nThrID8 = 8;
	nThrID9 = 9; nThrID10 = 10; nThrID11 = 11;

	gettimeofday(&start, NULL); //marcador de início do processamento
	for (i = 0; i < N; i++) {

		for (j = 0; j < N; j++) {

			a[i][j] = i * mul;
			b[i][j] = i;
		}
	}
	printf ("Matrix generation finished.\n");

	control = 0;
	for (i = 1; i <= slaves; i++) {

		if (i <= (int) (N % slaves)) {
			rows = (int) (N / slaves) + 1;
		} else {
			rows = (int) (N / slaves);
		}

		printf("Fetching %d rows for thread %d\n", rows, i);

		// MPI_Ssend(&control, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
		m_control[i] = control;
		// MPI_Rsend(&rows, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
		m_rows[i] = rows;
		// MPI_Isend(&a[control][0], rows * N, MPI_LONG, i, 1, MPI_COMM_WORLD, &request);
		// MPI_Issend(&b, N * N, MPI_LONG, i, 1, MPI_COMM_WORLD, &request);
		control += rows;
	}

	/* Receba resultados dos processos escravos
	for (i = 1; i <= slaves; i++) {
		MPI_Irecv(&control, 1, MPI_INT, i, 2, MPI_COMM_WORLD, &request);
		MPI_Wait(&request, &status);
		MPI_Recv(&rows, 1, MPI_INT, i, 2, MPI_COMM_WORLD, &status);
		MPI_Recv(&c[control][0], rows * N, MPI_LONG, i, 2, MPI_COMM_WORLD, &status);
	} */

	printf("Starting %d threads.\n", SIZE);
	//pthread_create(&thr0, NULL, (void*) multiplica, &nThrID0);
	pthread_create(&thr1, NULL, (void*) multiplica, &nThrID1);
	pthread_create(&thr2, NULL, (void*) multiplica, &nThrID2);
	pthread_create(&thr3, NULL, (void*) multiplica, &nThrID3);
	pthread_create(&thr4, NULL, (void*) multiplica, &nThrID4);
	pthread_create(&thr5, NULL, (void*) multiplica, &nThrID5);
	pthread_create(&thr6, NULL, (void*) multiplica, &nThrID6);
	pthread_create(&thr7, NULL, (void*) multiplica, &nThrID7);
	pthread_create(&thr8, NULL, (void*) multiplica, &nThrID8);
	pthread_create(&thr9, NULL, (void*) multiplica, &nThrID9);
	pthread_create(&thr10, NULL, (void*) multiplica, &nThrID10);
	pthread_create(&thr11, NULL, (void*) multiplica, &nThrID11);

	//pthread_join(thr0, NULL);
	pthread_join(thr1, NULL);
	pthread_join(thr2, NULL);
	pthread_join(thr3, NULL);
	pthread_join(thr4, NULL);
	pthread_join(thr5, NULL);
	pthread_join(thr6, NULL);
	pthread_join(thr7, NULL);
	pthread_join(thr8, NULL);
	pthread_join(thr9, NULL);
	pthread_join(thr10, NULL);
	pthread_join(thr11, NULL);

	printf ("Multiplication finished.\n");
	gettimeofday(&end, NULL);
	double delta = ((end.tv_sec  - start.tv_sec) * 1000000u + end.tv_usec - start.tv_usec) / 1.e6;

	printf("Time of processing: %.16g (s)\n", delta);
	for (i = 0; i < N; i++)
		for (j = 0; j < N; j++)
			assert (c[i][j] == i * mul * col_sum);

	printf ("Test finished.\n");
}

void multiplica(int *rank) {
	int i, j, k;

	for (k = 0; k < N; k++)
		for (i = m_control[*rank]; i < m_control[*rank] + m_rows[*rank]; i++) {

			c[i][k] = 0.0;
			for (j = 0; j < N; j++)
				c[i][k] += a[i][j] * b[j][k];
		}
}
