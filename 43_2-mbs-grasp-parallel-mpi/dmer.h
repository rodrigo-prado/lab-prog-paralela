//--------------------------------------------------------------------------- 
 
#ifndef DMERCLASS 
#define DMERCLASS 

//--------------------------------------------------------------------------- 

#include <fstream>
#include <string.h>
#include <stdlib.h>

// --------------------------------------------------------------------------

#include "array.cpp"
#include "matriz.cpp" 
#include "structs.h"
#include "sig_graph.h"

//--------------------------------------------------------------------------- 
 
#ifdef DEBUG_ALL 
#define DEBUG_DMER

extern int nfree;
extern int naloc;
#endif 
 
//--------------------------------------------------------------------------- 


class Dmer
{ 
public: 
  Dmer(const char * name, int MaxIter, int MaxTime); 
  ~Dmer() { A.desaloca();}

  double calcula_tempo(const unsigned long int ini, const unsigned long int fim);

  unsigned long int t_ini;

private: 
  void le_dados_grasp(int MaxIter, int MaxTime); 

  Array<char*>   RowsNames; // Nomes das linhas
  Matriz<double> A;         // Matriz de entrada
  Sigraph        sg;        // signed graph
  Sigraph        sg2;       // auxiliar signed graph
 
  // Scaling
  Array<bool> r;  
  Array<bool> b;
  Array<bool> J;
 
  int               nrow;
  int               ncol;
  int               nz;
  char              nome[256];

  int               opc_arv_heu;    // qual metodo eh usado para construcao da arvore na heuristica
}; 
 
//--------------------------------------------------------------------------- 

#endif

//--------------------------------------------------------------------------- 
