#include <stdio.h>
#include <mpi.h>

int numeroDeTarefas = 6; 
int matrizVizinhanca[6][6] = {	{0,1,1,0,0,0}, 
				{1,0,1,1,1,0}, 
				{1,1,0,0,0,1}, 
				{0,1,0,0,0,0}, 
				{0,1,0,0,0,0}, 
				{0,0,1,0,0,0}	};

/* retorna o número de vizinhos da tarefa myRank */
int contaNumeroDeVizinhos(int myRank) {
	int i; 
	int contador = 0;
	for (i = 0; i < numeroDeTarefas; i++) 
		if (matrizVizinhanca[myRank][i] == 1)
			contador++;
	return contador;
}

int finaliza(int contador[], int numeroDeVizinhos, int myRank) {
	int i, todosiguaisa0 = 1;
	if (contador[myRank] != numeroDeVizinhos)
		return 0;
	else
		return 1;
}

/* programa principal */
int main(int argc, char** argv) {
	int i, j, contador;
	int numeroDeVizinhos;
	int myRank;
	int source;
	int tag = 50;
	int pai;
	MPI_Status status;
	int origem;
	int state; /* marca o pulso atual deste processo */
	int dist[numeroDeTarefas]; /* distancia */
	int first[numeroDeTarefas]; /* primeiro no no caminho */
	int set[numeroDeTarefas];
	int level[numeroDeTarefas]; /* pulso dos meus vizinho */

	/*inicialização do MPI*/
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &myRank);

	numeroDeVizinhos = contaNumeroDeVizinhos(myRank);
	printf("iniciando...\n");
	fflush(stdout);
	/* inicializando vetor dist e first */
	for (i = 0; i < numeroDeTarefas; i++) {
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
		if (matrizVizinhanca[myRank][i] == 1) {
			/* pede aos vizinhos para enviar */
			MPI_Send(set, numeroDeTarefas, MPI_INT, i, tag, MPI_COMM_WORLD);
		}
	while (state < numeroDeTarefas-1) {
		MPI_Recv(set, numeroDeTarefas, MPI_INT, MPI_ANY_SOURCE, tag, MPI_COMM_WORLD, &status);
		origem = status.MPI_SOURCE; 
		level[origem]++; 
		for (i=0; i<numeroDeTarefas; i++) {
			if (set[i]==1) {
				if (dist[i]>level[origem]) {
					dist[i]=level[origem];
					first[i]=origem;
				}
				/* testando se meu state e menor ou igual ao level de todos os meus vizinhos */
				int continua=1;
				for (j=0; j<numeroDeTarefas; j++) 
					if (matrizVizinhanca[myRank][j]==1 && state>=level[j]) {
						continua=0;
						break;
					}
				if (continua) {
					state++;
					/* se minha distancia ate j for igual a state, entao o adiciono a set */
					for (j=0; j<numeroDeTarefas; j++) {
						if (dist[j]==state)
							set[j]=1;
					}
					for (j=0; j<numeroDeTarefas; j++)
						if (matrizVizinhanca[myRank][j]==1)
							MPI_Send(set, numeroDeTarefas, MPI_INT, j, tag, MPI_COMM_WORLD);
				} /*if (continua) {*/
			} /*if (set[i]==1) {*/
		} /*for (i=0; i<numeroDeTarefas; i++) {*/
		printf("Processo %d: state=%d\n", myRank, state);
		fflush(stdout);
	} /*while (state < numeroDeTarefas - 1) {*/
	/*Imprimindo as distancias*/
	printf("processo %d: ", myRank); 
	for (i=0; i<numeroDeTarefas; i++)
		printf("dist(%d)=%d, ", i, dist[i]); 
	printf("\n");
	printf("processo %d: ", myRank);
	for (i=0; i<numeroDeTarefas; i++)
		printf("first(%d)=%d, ", i, first[i]); 
	printf("\n");
	fflush(stdout); 
	/*Finalização do MPI*/
	MPI_Finalize();
}
