//---------------------------------------------------------------------------

#include <mpi.h>
#include <limits>
#include <sys/time.h>
#include "dmer.h"

extern int size, rank;

int m_n_viz_1a = 0;
int m_n_viz_1b = 0;
int m_n_viz_2a = 0;
int m_n_viz_2b = 0;
int m_n_viz_ab = 0;

extern int m_target;
double m_ttt;

//---------------------------------------------------------------------------

Dmer::Dmer(const char * name, int MaxIter, int MaxTime) {
  /* tempo inicial */
  t_ini  = (unsigned long int) clock();

  strcpy(nome, name);
  le_dados_grasp(MaxIter, MaxTime);   /* carrega a estrutura custo a partir do arquivo de entrada e executa grasp */
}

//---------------------------------------------------------------------------

/* funcao de tempo
double Dmer::calcula_tempo(const unsigned long int ini, const unsigned long int fim) {
  double r;

  if(fim >= ini)
    r = ((double)(fim - ini)) / CLOCKS_PER_SEC;
  else
    r = ((double)( (fim + (unsigned long int) - 1) - ini)) / CLOCKS_PER_SEC;
  return r;
}
*/

//---------------------------------------------------------------------------

void Dmer::le_dados_grasp(int MaxIter, int MaxTime) {
  double delta;

  MPI_Status status;
  int result, best;
  double l_ttt = std::numeric_limits<double>::max();

  int jj;
  char arq[256];
  std::ifstream fin;
  Array<int> f;
  Array<int> f2;
  Array<int> f3;
  Array<int> f4;
  Array<int> f5;
  Array<int> f6;
  Array<int> atc;

  //int    DEPU         = 1;
  int TEST = 0;

  // unsigned long int t_ini2;

  timeval start, end;

  strcpy(arq,nome);
  fin.open(arq);

  /* constroi grafo de sinais */
  sg.ini(fin);

  /* aloca */
  f.aloca(sg.n + 5);
  f2.aloca(sg.n + 5);
  f3.aloca(sg.n + 5);
  f4.aloca(sg.n + 5);
  f5.aloca(sg.n + 5);
  f6.aloca(sg.n + 5);

  m_ttt = std::numeric_limits<double>::max();

  /* Grasp */
  gettimeofday(&start, NULL); //marcador de início do processamento
  // t_ini2  = (unsigned long int) clock();
  jj = sg.grasp_sig_v2(sg.vet1, f, sg.vet2, f2, sg.vet3, f3, sg.vet4, f4, f5, f6, MaxIter, MaxTime,
      TEST);
  /*std::cout << std::endl << nome << " GRASP = " << jj << " Tempo = "
      << calcula_tempo(t_ini2, (unsigned long int) clock()) << std::endl;*/
  // gettimeofday(&end, NULL);
  // delta = ((end.tv_sec  - start.tv_sec) * 1000000u + end.tv_usec - start.tv_usec) / 1.e6;
  // std::cout << rank << ":" << nome << " GRASP = " << jj << " Tempo = " << delta << std::endl;
  best = jj;
  /* Processo 0 recebe resultados dos outros processos */
  if (rank == 0) {
    for (int i = 1; i < size; i++) {
      // std::cout << rank << ":esperando processo " << i << " responder." << std::endl;
      MPI_Recv(&result, 1, MPI_INT, i, TAG_FINISHED, MPI_COMM_WORLD, &status);
      // std::cout << "\x1b[1;38m"  << rank << ":resposta [" << result << "] do processo [" << i
      //     << "]." << "\x1b[0m"  << std::endl;
      if (result > best) best = result;
      MPI_Recv(&l_ttt, 1, MPI_DOUBLE, i, TAG_FINISHED, MPI_COMM_WORLD, &status);
      if (l_ttt < m_ttt) m_ttt = l_ttt;
      MPI_Recv(&result, 1, MPI_INT, i, TAG_FINISHED, MPI_COMM_WORLD, &status);
      m_n_viz_1a += result;
      MPI_Recv(&result, 1, MPI_INT, i, TAG_FINISHED, MPI_COMM_WORLD, &status);
      m_n_viz_1b += result;
      MPI_Recv(&result, 1, MPI_INT, i, TAG_FINISHED, MPI_COMM_WORLD, &status);
      m_n_viz_2a += result;
      MPI_Recv(&result, 1, MPI_INT, i, TAG_FINISHED, MPI_COMM_WORLD, &status);
      m_n_viz_2b += result;
      MPI_Recv(&result, 1, MPI_INT, i, TAG_FINISHED, MPI_COMM_WORLD, &status);
      m_n_viz_ab += result;
    }
    gettimeofday(&end, NULL);
    delta = ((end.tv_sec  - start.tv_sec) * 1000000u + end.tv_usec - start.tv_usec) / 1.e6;
    //
    // std::cout << "\x1b[1;36m"  << rank << ":m_n_viz_1a [" << m_n_viz_1a << "]." << "\x1b[0m"
    //     << std::endl;
    // std::cout << "\x1b[1;36m"  << rank << ":m_n_viz_1b [" << m_n_viz_1b << "]." << "\x1b[0m"
    //     << std::endl;
    // std::cout << "\x1b[1;36m"  << rank << ":m_n_viz_2a [" << m_n_viz_2a << "]." << "\x1b[0m"
    //     << std::endl;
    // std::cout << "\x1b[1;36m"  << rank << ":m_n_viz_2b [" << m_n_viz_2b << "]." << "\x1b[0m"
    //     << std::endl;
    // std::cout << "\x1b[1;36m"  << rank << ":m_n_viz_ab [" << m_n_viz_ab << "]." << "\x1b[0m"
    //     << std::endl;

    std::cout << best << "," << delta << "," << m_target << "," << m_ttt << std::endl;

  }

  /* desaloca */
  r.desaloca();
  b.desaloca();
  J.desaloca();
  f.desaloca();
  f2.desaloca();
  f3.desaloca();
  f4.desaloca();
  f5.desaloca();
  f6.desaloca();
  atc.desaloca();
  sg.fini();

  fin.close();
}
