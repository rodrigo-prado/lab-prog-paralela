#include <stdio.h>
#include <mpi.h>

int numeroDeTarefas = 6;
int matrizVizinhanca[6][6] = {
                                {0, 1, 1, 0, 0, 0},
                                {1, 0, 1, 1, 1, 0},
                                {1, 1, 0, 0, 0, 1},
                                {0, 1, 0, 0, 0, 0},
                                {0, 1, 0, 0, 0, 0},
                                {0, 0, 1, 0, 0, 0}
                             };

/* retorna o número de vizinhos da tarefa myRank */
int contaNumeroDeVizinhos(int myRank)
{
    int i;
    int contador = 0;
    
    for (i = 0; i < numeroDeTarefas; i++)
    {
        if (matrizVizinhanca[myRank][i] == 1)
        {
            contador++;
        }
    }
    
    return contador;
}

/* programa principal */
int main(int argc, char** argv)
{
    int i, j;
    int numeroDeVizinhos;
    int myRank;
    int tag, tag1 = 80, tag2 = 20;
    int envia;
    MPI_Status status;
    int origem;
    int state;                          /* marca o pulso atual deste processo */
    int dist[numeroDeTarefas];          /* distancia */
    int first[numeroDeTarefas];         /* primeiro no no caminho */
    int set[numeroDeTarefas];
    int level[numeroDeTarefas];         /* pulso dos meus vizinho */

    /*inicializacao do MPI*/
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myRank);

    numeroDeVizinhos = contaNumeroDeVizinhos(myRank);
    printf("\x1b[32;1mIniciando processo %d...\n", myRank);
    fflush(stdout);

    /* inicializando vetor dist e first */
    for (i = 0; i < numeroDeTarefas; i++)
    {
        dist[i] = numeroDeTarefas; /* considerado como infinito */
        first[i] = -1;
        set[i] = 0;
        level[i] = 0;
    }

    dist[myRank] = 0;
    set[myRank] = 1;
    state = 0;

    /* enviando meu conjunto para os meus vizinhos */
    for (i = 0; i < numeroDeTarefas; i++)
        if (matrizVizinhanca[myRank][i] == 1)
        {
            /* pede aos vizinhos para enviar */
            MPI_Send(set, numeroDeTarefas, MPI_INT, i, tag1, MPI_COMM_WORLD);
        }

    while (state < numeroDeTarefas - 1)
    {
        MPI_Recv(set, numeroDeTarefas, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        origem = status.MPI_SOURCE;
        
        printf("\x1b[34;1m[Processo: %d] [Origem: %d] [Set: %d, %d, %d, %d, %d, %d]\n", myRank, origem, set[0], set[1], set[2], set[3], set[4], set[5]);
        
        tag = status.MPI_TAG;
        
        if (tag == tag2)
        {
            printf("\x1b[34;1mProcesso %d recebendo mensagem final de %d\n", myRank, origem);
            MPI_Send(set, numeroDeTarefas, MPI_INT, origem, tag2, MPI_COMM_WORLD);
            matrizVizinhanca[myRank][origem] = 0;
        }
        
        level[origem]++;
        envia = 1;
        
        for (i = 0; i < numeroDeTarefas; i++)
        {
            if (set[i] == 1)
            {
                if (dist[i] > level[origem])
                {
                    dist[i] = level[origem];
                    first[i] = origem;
                }
            } /*for (i=0; i<numeroDeTarefas; i++) {*/
        
            /* testando se meu state e menor ou igual ao level de todos os meus vizinhos */
            for (j = 0; j < numeroDeTarefas; j++)
            {
                if (matrizVizinhanca[myRank][j] == 1 && state >= level[j])
                {
                    envia = 0;
                    break;
                }
            }
        } /*if (set[i]==1) {*/
        
        if (envia)
        {
            int modificou = 0;
            state++;
            
            /* se minha distancia ate j for igual a state, entao o adiciono a set */
            for (j = 0; j < numeroDeTarefas; j++)
            {
                if (dist[j] == state)
                {
                    set[j] = 1;
                    modificou = 1;
                }
                else
                    set[j] = 0;
            }
            
            for (j = 0; j < numeroDeTarefas; j++)
            {
                if (matrizVizinhanca[myRank][j] == 1)
                {
                    MPI_Send(set, numeroDeTarefas, MPI_INT, j, (modificou) ? tag1 : tag2, MPI_COMM_WORLD);
                }
            }
            
            // Se nao modificou, nao vai mais, portanto espera mensagens dos vizinhos e encerra atividade
            if (!modificou)
            {
                /*for (j=0; j<numeroDeTarefas; j++)
                        printf("dist(%d)=%d, ", j, dist[j]); 
                printf("\n");*/
                printf("\x1b[36;1m[Processo: %d] [State : %d] [Set: %d, %d, %d, %d, %d, %d]\n > Não será mais atualizado. Encerrando atividades.\n", myRank, state, set[0], set[1], set[2], set[3], set[4], set[5]);
                for (j = 0; j < numeroDeTarefas; j++)
                    if (matrizVizinhanca[myRank][j] == 1)
                    {
                        MPI_Recv(set, numeroDeTarefas, MPI_INT, MPI_ANY_SOURCE, tag2, MPI_COMM_WORLD, &status);
                        origem = status.MPI_SOURCE;
                        printf("\x1b[34;1mProcesso %d confirmando mensagem final de %d\n", origem, myRank);
                    }
                state = numeroDeTarefas;
            }
        } /*if (envia) {*/
        //printf("Processo %d: state=%d\n", myRank, state);
        //fflush(stdout);
    } /*while (state < numeroDeTarefas - 1) {*/
    
    /* Imprimindo as distancias */
    printf("\x1b[37;1mProcesso %d: ", myRank);
    for (i = 0; i < numeroDeTarefas - 1; i++)
        printf("\x1b[37;1mDist[%d]=%d  ", i, dist[i]);
    printf("\x1b[37;1mDist[%d]=%d\n", i, dist[numeroDeTarefas - 1]);
    
    /* Imprimindo os firsts */
    printf("\x1b[37;1mProcesso %d: ", myRank);
    for (i = 0; i < numeroDeTarefas - 1; i++)
        printf("\x1b[37;1mFirst[%d]=%d  ", i, first[i]);
    printf("\x1b[37;1mFirst[%d]=%d\n", i, first[numeroDeTarefas - 1]);
    
    /* Limpa o buffer */
    fflush(stdout);
    
    /* Reseta config de cores */
    printf("\x1b[0m");
    
    /*Finalizacao do MPI*/
    MPI_Finalize();
}

