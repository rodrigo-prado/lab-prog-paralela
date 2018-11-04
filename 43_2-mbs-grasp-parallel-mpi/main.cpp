
//---------------------------------------------------------------------------

#include <limits>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <mpi.h>
#include <math.h>
#include <sys/types.h>
#include <unistd.h>

//---------------------------------------------------------------------------

#ifdef DEBUG_ALL
int naloc = 0;
int nfree = 0;
#endif

#include "dmer.h"
#include "structs.h"

int m_target;

//---------------------------------------------------------------------------

void mserro(const char *clas, const char *func, const char *ms, const int nerr);

//---------------------------------------------------------------------------

int size, rank;

int main(int argc, char *argv[]) {
  /*Inicialização do MPI*/
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  m_target = std::numeric_limits<int>::max();

  int MaxIter = 100;
  int MaxTime = 300; //segundos

  //long int seed = exp(rank * 1024);
  long int seed = time(NULL) * rank * getpid();
  /* inicializa pseudo-aleatoricidade*/
  // srand(time(0));
  srand(seed);
  // std::cout << "Format:" << std::endl;
  // std::cout << "dmer <file> <MaxGraspIter:default=100> <MaxGraspTime:default=300>" << std::endl<<std::endl;

  if (argc < 2) {
    std::cout << "Ex: ./dmer graph.g 50 30" << std::endl;
    abort();
  }

  // File
  // std::cout<<"  <file> = "<<argv[1]<<std::endl;

  if (argc == 2) {
    // std::cout << "  <MaxGraspIter> = " << 100 << std::endl;
    // std::cout << "  <MaxGraspTime> = " << 300 << std::endl;
  }

  if (argc == 3) {
    MaxIter = atoi(argv[2]);
    // std::cout << "  <MaxGraspIter> = " << argv[2] << std::endl;
    // std::cout << "  <MaxGraspTime> = " << 300 << std::endl;
  }

  if (argc == 4) {
    MaxIter = atoi(argv[2]);
    // std::cout << "  <MaxGraspIter> = " << argv[2] << std::endl;
    MaxTime = atoi(argv[3]);
    // std::cout << "  <MaxGraspTime> = " << argv[3] << std::endl;
  }

  if (argc == 5) {
    MaxIter = atoi(argv[2]);
    // std::cout << "  <MaxGraspIter> = " << argv[2] << std::endl;
    MaxTime = atoi(argv[3]);
    // std::cout << "  <MaxGraspTime> = " << argv[3] << std::endl;
    m_target = atoi(argv[4]);
    // std::cout << "  <Target> = " << argv[4] << std::endl;
  }

  /* construtor do problema */
  if (rank < MaxIter % size) {
    MaxIter = (MaxIter / size) + 1;
  } else {
    MaxIter = MaxIter / size;
  }
  //printf("Sending %d rows to task %d\n", rows, i);
  // std::cout << "\x1b[1;34m" << rank << ":Beginning with " << MaxIter << " iterations "
  //     << "for the period max of " << MaxTime << " seconds." << "\x1b[0m" << std::endl;

  Dmer prob(argv[1], MaxIter, MaxTime);

  MPI_Finalize(); //finalizar MPI
}

//---------------------------------------------------------------------------

void mserro(const char *clas, const char *func, const char *ms, const int nerr) {
  std::cout << std::endl << "Erro na classe " << clas << " funcao " << func
    << " - " << ms << std::endl;
  exit(nerr);
}

//---------------------------------------------------------------------------
