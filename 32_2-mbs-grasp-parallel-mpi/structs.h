
#ifndef STRUCT
#define STRUCT

#ifndef PRECISAO
#define PRECISAO 0.00001
#endif

#ifndef INFINITO
#define INFINITO 1000000000
#endif

#ifndef TEMPOMAXGRASP
#define TEMPOMAXGRASP 300
#endif

// estrutura aresta do grafo
typedef struct edge_st
{
  bool pos;
  bool neg;
} str_e;


// estrutura pilha
struct ststackno
{
   int ind;
   struct ststackno* proximo;
};
typedef struct ststackno* ptstackno;

#endif
