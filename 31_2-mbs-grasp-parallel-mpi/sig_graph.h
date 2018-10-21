//--------------------------------------------------------------------------- 
 
#ifndef SIGGRAPH
#define SIGGRAPH 
 
//--------------------------------------------------------------------------- 

#include <fstream>
#include <string.h>
#include <stdlib.h>

// --------------------------------------------------------------------------

#include "array.cpp"
#include "matriz.cpp" 
#include "structs.h"

//--------------------------------------------------------------------------- 
 
#ifdef DEBUG_ALL 
#define DEBUG_SIGGRAPH

extern int nfree;
extern int naloc;
#endif 
 
//--------------------------------------------------------------------------- 

class Sigraph
{ 
public: 
  Sigraph() {} 
  ~Sigraph() {g.desaloca(); Gp.desaloca(); Gn.desaloca(); map.desaloca(); stat.desaloca(); comp.desaloca();  vet1.desaloca(); vet2.desaloca(); vet3.desaloca(); vet4.desaloca();}
  
  Matriz<str_e> g;    // grafo booleano de arestas positivas e negativas
  Matriz<int>   Gp;   // grafo positivo representado por listas (vetores) de arestas. (informacao duplicada)
  Matriz<int>   Gn;   // grafo negativo representado por listas (vetores) de arestas. (informacao duplicada)
  Array<int>    map;  // mapa de indices de g -> A
  Array<int>    stat; // status do vertice. -1 -> nao ativo
                      //                    1  -> ativo
                      //                    2  -> vertice isolado
                      //                    3  -> vertice negativamente isolado
                      //                    4  -> vertice isolado por ser folha e ser extremidade de uma ponte
                      //                    5  -> vertice retirado pela regra 5a
  
  Array<int>    comp; // componente do vertice
  
  // Vetores de uso generico 
  Array<int> vet1;   // preproc_Separator_1, solve_dumb_comp, solve_dumb_rec, grasp_stable_set
  Array<int> vet2;   // preproc_Separator_1, solve_dumb_comp, solve_dumb_rec, grasp_stable_set
  Array<int> vet3;   // grasp_stable_set
  Array<int> vet4;   // grasp_stable_set

  int           n;             // numero de vertices
  int           n_c;           // numero de componentes

  int           n_iso;         // numero de vertices isolados retirados
  int           n_niso;        // numero de vertices negativamente isolados retirados
  int           n_vizig;       // numero de vertices retirados que a vizinhanca eh igual a outro vertice
  int           n_pontes;      // numero de pontes retiradas
  int           n_add_preproc; // numero adicionado a solucao por causa dos vertices retirados no preproc do Separator

  float ini(std::ifstream &fin); 
  void  fini();

  void testa_particoes_grasp_sig_v2(Array<int> &A, Array<int> &a, Array<int> &B, Array<int> &b, Array<int> &C, Array<int> &c);
  void atualiza_a_b_c(Array<int> &A, Array<int> &a, Array<int> &B, Array<int> &b, Array<int> &C, Array<int> &c);
  int  metodo_construtivo_grasp_sig_v2(Array<int> &A, Array<int> &a, Array<int> &B, Array<int> &b, Array<int> &C, Array<int> &c, Array<int> &cand1, Array<int> &cand2, int TEST);
  void maximiza_solucao_grasp_sig_v2(Array<int> &A, Array<int> &a, Array<int> &B, Array<int> &b, Array<int> &C,  Array<int> &c, Array<int> &cand1, Array<int> &cand2, int TEST);
  bool can_move_C_X_v2(int l, Array<int> &x, Array<int> &y, Array<int> &C);
  bool can_2move_C_X_v2(int l1, int l2, Array<int> &x, Array<int> &y, Array<int> &C);
  int  can_move_C_AB_v2(int l, Array<int> &a, Array<int> &b, Array<int> &C, int TEST);
  int  can_2move_C_AB_v2(int l1, int l2, int num_candA, int num_candB, Array<int> &a, Array<int> &b, Array<int> &C, Array<int> &c, int TEST);
  bool move_2_C_AB_v2(int desceu, int origem, Array<int> &A, Array<int> &a, Array<int> &B, Array<int> &b, Array<int> &C , Array<int> &c, Array<int> &cand1, Array<int> &cand2, int TEST);
  bool move_3_C_AB_v2(int desceu1, int desceu2, int origem1, int origem2, Array<int> &A, Array<int> &a, Array<int> &B, Array<int> &b, Array<int> &C, Array<int> &c, Array<int> &cand1, Array<int> &cand2, int TEST);
  int  viz_down_A_v2(Array<int> &A, Array<int> &a, Array<int> &B, Array<int> &b, Array<int> &C, Array<int> &c, Array<int> &cand1, Array<int> &cand2, int TEST, unsigned long int t_ini_g);
  int  viz_down_B_v2(Array<int> &A, Array<int> &a, Array<int> &B, Array<int> &b, Array<int> &C, Array<int> &c, Array<int> &cand1, Array<int> &cand2, int TEST, unsigned long int t_ini_g);
  int  viz_2down_A_v2(Array<int> &A, Array<int> &a, Array<int> &B, Array<int> &b, Array<int> &C, Array<int> &c, Array<int> &cand1, Array<int> &cand2, int TEST, unsigned long int t_ini_g, int TIMEMAX);
  int  viz_2down_B_v2(Array<int> &A, Array<int> &a, Array<int> &B, Array<int> &b, Array<int> &C, Array<int> &c, Array<int> &cand1, Array<int> &cand2, int TEST, unsigned long int t_ini_g, int TIMEMAX);
  int  viz_2down_AB_v2(Array<int> &A, Array<int> &a, Array<int> &B, Array<int> &b, Array<int> &C, Array<int> &c, Array<int> &cand1, Array<int> &cand2, int TEST, unsigned long int t_ini_g, int TIMEMAX);
  int  obj_grasp_sig_v2(Array<int> &A, Array<int> &B);
  double calcula_tempo(const unsigned long int ini, const unsigned long int fim);
  int  grasp_sig_v2(Array<int> &A, Array<int> &a, Array<int> &B, Array<int> &b, Array<int> &C, Array<int> &c, Array<int> &cand1, Array<int> &cand2, Array<int> &b_A, Array<int> &b_B, int ITMAX, int TIMEMAX, int TEST);

  /* ------- TAD ------ */
  void write_sol(const char * name, Array<int> &A, Array<int> &B);

private: 

}; 
 
//--------------------------------------------------------------------------- 

#endif

//--------------------------------------------------------------------------- 

