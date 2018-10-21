/* http://www.revistabw.com.br/revistabw/mpi_5-c/ */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>
#include <mpi.h>

#define N 912 //N fixo para evitar limitação do valor de N - BAD TERMINATION 134

MPI_Status status;
MPI_Request request;
long a[N][N], b[N][N], c[N][N];

//Função que pode ser chamada no sistema para printar a matrix
//Usar para valores baixos de N
void print_matrix(long matrix[N][N]) {

  int i, j;
  for (i = 0; i < N; i++) {
    for (j = 0; j < N; j++) {
      printf("%ld ", matrix[i][j]);
    }
    printf("\n");
  }
}

//Função principal
int main (int argc, char *argv[]) {

  /* Declaração das variáveis */
  int size, rank, slaves, rows, control, i, j, k, rc, mul = 5;
  struct timeval start, end;
  int col_sum = N * (N - 1) / 2;

  /*Inicialização do MPI*/
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  slaves = size-1;

  /* Processo Mestre */
  if (rank == 0) {

    printf("MPI has started with %d tasks.\n", size);
    gettimeofday(&start, NULL); //marcador de início do processamento
    for (i = 0; i < N; i++) {

      for (j = 0; j < N; j++) {

        a[i][j] = i * mul;
        b[i][j] = i;
      }
    }

    printf ("Matrix generation finished.\n");
    if (slaves > 0) {

      control = 0;
      for (i = 1; i <= slaves; i++) {

        if (i <= (int) (N % slaves)) {
          rows = (int) (N / slaves) + 1;
        } else {
          rows = (int) (N / slaves);
        }
        printf("Sending %d rows to task %d\n", rows, i);

        /* Diferentes tipos de envio */
        /* http://www.mcs.anl.gov/research/projects/mpi/sendmode.html */
        /* http://stackoverflow.com/questions/21512975/what-is-the-difference-between-isend-and-issend-in-openmpi */
        MPI_Ssend(&control, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
        MPI_Rsend(&rows, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
        MPI_Isend(&a[control][0], rows * N, MPI_LONG, i, 1, MPI_COMM_WORLD, &request);
        MPI_Issend(&b, N * N, MPI_LONG, i, 1, MPI_COMM_WORLD, &request);
        control += rows;
      }

      /* Receba resultados dos processos escravos */
      for (i = 1; i <= slaves; i++) {
        MPI_Irecv(&control, 1, MPI_INT, i, 2, MPI_COMM_WORLD, &request);
        MPI_Wait(&request, &status);
        MPI_Recv(&rows, 1, MPI_INT, i, 2, MPI_COMM_WORLD, &status);
        MPI_Recv(&c[control][0], rows * N, MPI_LONG, i, 2, MPI_COMM_WORLD, &status);
      }
    } else { //opção caso o número de processos escravos seja 0

      for (i = 0; i < N; i++)
        for (j = 0; j < N; j++)
          for (k = 0; k < N; k++)
            c[i][j] += a[i][k] * b[k][j];
    }

    printf ("Multiplication finished.\n");
    gettimeofday(&end, NULL);
    double delta = ((end.tv_sec  - start.tv_sec) * 1000000u + end.tv_usec - start.tv_usec) / 1.e6;

    printf("Time of processing: %.16g (s)\n", delta);
    for (i = 0; i < N; i++)
      for (j = 0; j < N; j++)
        assert (c[i][j] == i * mul * col_sum);

    printf ("Test finished.\n");
  } // final do processo mestre

  /* Processo Escravo */
  if (rank > 0) {

    /* Diferentes tipos de recepção de dados */
    MPI_Irecv(&control, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &request);
    MPI_Wait(&request, &status);
    MPI_Recv(&rows, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
    MPI_Recv(&a, rows * N, MPI_LONG, 0, 1, MPI_COMM_WORLD, &status);
    MPI_Irecv(&b, N * N, MPI_LONG, 0, 1, MPI_COMM_WORLD, &request);
    MPI_Wait(&request, &status);

    for (k = 0; k < N; k++)
      for (i = 0; i < rows; i++) {

        c[i][k] = 0.0;
        for (j = 0; j < N; j++)
          c[i][k] += a[i][j] * b[j][k];
      }

    MPI_Issend(&control, 1, MPI_INT, 0, 2, MPI_COMM_WORLD, &request);
    MPI_Send(&rows, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
    MPI_Send(&c, rows * N, MPI_LONG, 0, 2, MPI_COMM_WORLD);
  }
  MPI_Finalize(); //finalizar MPI
}
