
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

#define TAG_REQUEST 10
#define TAG_WORKLOAD 20
// #define TAG_TURN          30
#define TAG_FINISHED 99

#define MSG_DEFAULT 0

#define PROC_SIZE_MAX 256

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
