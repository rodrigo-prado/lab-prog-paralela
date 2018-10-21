#include <stdio.h>
#include <string.h>
#include "mpi.h"

/*Definir o grafo da aplicação antes de executar*/
int numeroDeTarefas = 6;
int matrizVizinhanca[6][6] = {
    {0, 1, 1, 0, 0, 0},
    {1, 0, 1, 1, 1, 0},
    {1, 1, 0, 0, 0, 1},
    {0, 1, 0, 0, 0, 0},
    {0, 1, 0, 0, 0, 0},
    {0, 0, 1, 0, 0, 0}
};

/*retorna o número de vizinhos da tarefa myRank*/
int contaNumeroDeVizinhos(int myRank)
{
    int i;
    int contador = 0;
    for (i = 0; i < numeroDeTarefas; i++)
        if (matrizVizinhanca[myRank][i] == 1)
            contador++;
    return contador;
}

/*programa principal*/
int main(int argc, char** argv)
{
    int i;
    int numeroDeVizinhos;
    int myRank;
    int source;
    int tag = 50;
    char message[100] = "Oi!";
    MPI_Status status;

    //inicialização do MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
    numeroDeVizinhos = contaNumeroDeVizinhos(myRank);

    if (myRank == 0)
    {
        /*enviando para todos os vizinhos de myRank*/
        for (i = 0; i < numeroDeTarefas; i++)
            if (matrizVizinhanca[myRank][i] == 1)
            {
                printf("Enviando mensagem para %d\n", i);
                MPI_Send(message, strlen(message) + 1, MPI_CHAR, i, tag, MPI_COMM_WORLD);
            }
        /*recebendo de todos os vizinhos de myRank*/
        for (i = 0; i < numeroDeVizinhos; i++)
        {
            MPI_Recv(message, 100, MPI_CHAR, MPI_ANY_SOURCE, tag, MPI_COMM_WORLD, &status);
            printf("Recebendo msg de %d\n", status.MPI_SOURCE);
        }
    }
    else
    {
        /*recebendo msg de uma tarefa vizinha qualquer*/
        MPI_Recv(message, 100, MPI_CHAR, MPI_ANY_SOURCE, tag, MPI_COMM_WORLD, &status);

        /*enviando para todos os vizinhos de myRank*/
        for (i = 0; i < numeroDeTarefas; i++)
            if (matrizVizinhanca[myRank][i] == 1)
            {
                printf("Enviando mensagem de %d para %d\n", myRank, i);
                MPI_Send(message, strlen(message) + 1, MPI_CHAR, i, tag, MPI_COMM_WORLD);
            }

        /*recebendo de todos os vizinhos de myRank menos 1*/
        for (i = 0; i < (numeroDeVizinhos - 1); i++)
            MPI_Recv(message, 100, MPI_CHAR, MPI_ANY_SOURCE, tag, MPI_COMM_WORLD, &status);
    }

    // Finalização do MPI
    MPI_Finalize();
}
