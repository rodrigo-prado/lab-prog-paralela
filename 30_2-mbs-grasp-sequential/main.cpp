
//---------------------------------------------------------------------------

#include <limits>
#include <iostream>
#include <cstdlib>
#include <cstring>

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

int main(int argc, char *argv[]) {
  m_target = std::numeric_limits<int>::max();

  int MaxIter = 100;
  int MaxTime = 300; //segundos

  /* inicializa pseudo-aleatoricidade*/
  srand(time(0));
  // std::cout << "Format:" << std::endl;
  // std::cout << "dmer <file> <MaxGraspIter:default=100> <MaxGraspTime:default=300>" << std::endl
  //     << std::endl;

  if (argc < 2) {
    std::cout << "Ex: ./dmer graph.g 50 30" << std::endl;
    abort();
  }

  // File
  // std::cout << "  <file> = " << argv[1] << std::endl;

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
  Dmer prob(argv[1], MaxIter, MaxTime);
}

//---------------------------------------------------------------------------

void mserro(const char *clas, const char *func, const char * ms,const int nerr) {
  std::cout << std::endl << "Erro na classe " << clas << " funcao " << func << " - " << ms
      << std::endl;
  exit(nerr);
}

//---------------------------------------------------------------------------
