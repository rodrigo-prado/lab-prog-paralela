
#include <mpi.h>
#include <sys/time.h>
#include <omp.h>
#include "sig_graph.h"

extern int size, rank;

int m_neighbor_status[PROC_SIZE_MAX];

int m_it = 0;
int m_flag = -1;
int m_itmax;
int m_msg;
int m_finished = 0;
int m_sended_zero = 0;

extern int m_n_viz_1a;
extern int m_n_viz_1b;
extern int m_n_viz_2a;
extern int m_n_viz_2b;
extern int m_n_viz_ab;

extern int m_target;
extern double m_ttt;

MPI_Request m_request;

/* create thread argument struct for thr_func() */
typedef struct _thread_data_t {
  int tid;
  Array<int> A;
  Array<int> a;
  Array<int> B;
  Array<int> b;
  Array<int> C;
  Array<int> c;
  Array<int> cand1;
  Array<int> cand2;
  int TEST;
  timeval t_ini_g;
  Sigraph *obj;
} thread_data_t;



// gera grafo a partir de um arquivo .G
float Sigraph::ini(std::ifstream &fin) {
  int i, j, n_are, a1, a2, at, ip, in;
  str_e  e;
  int DEPU = 0;
  float cont_pos = 0, cont_neg = 0;
  if (DEPU) std::cout << "construindo sg, " << std::endl;

  /* aloca */
  fin >> n;
  fin >> n_are;
  n_c = 0;
  n_iso = 0;
  n_niso = 0;
  n_vizig = 0;
  n_pontes = 0;
  n_add_preproc = 0;
  /* aloca */
  g.aloca(n, n);
  Gp.aloca(n, n);
  Gn.aloca(n, n);
  map.aloca(n);
  stat.aloca(n);
  comp.aloca(n);
  vet1.aloca(n);
  vet2.aloca(n);
  vet3.aloca(n);
  vet4.aloca(n);

  /* atribui */
  e.pos = false;
  e.neg = false;
  g.set(e);
  Gp.set(-1);
  Gn.set(-1);
  stat.set(1);
  comp.set(-1);

  /*insere arestas (i,j) em g */
  if (DEPU) std::cout << " insere arestas" << std::endl;
  for (i = 1; i <= n_are; ++i) {
    fin >> a1;
    fin >> a2;
    fin >> at;

    /* + */
    if ((at == 1) || (at == 2)) {
  	  g(a1,a2).pos = true;
  	  g(a2,a1).pos = true;

  	  if (at != 2) cont_pos += 1;
    }

    /* - */
    if ((at == -1) || (at == 2)) {
  	  g(a1,a2).neg = true;
  	  g(a2,a1).neg = true;

  	  if (at != 2) cont_neg += 1;
    }
  }

  /* Constroi Gp e Gn */
  if (DEPU) std::cout << " constroi Gp e Gn" << std::endl;
  for (i = 0; i < n; ++i) {
    ip = in = 0;
    for (j = 0; j < n; ++j) {
  	  if (g(i, j).pos) Gp(i, ip++) = j;
  	  if (g(i, j).neg) Gn(i, in++) = j;
    }
  }
  if (DEPU) Gp.imprime();
  if (DEPU) Gn.imprime();

  return (cont_neg / cont_pos);
}

//-------------------------------------------------------------------------------

void Sigraph::fini() {
  /* desaloca */
  g.desaloca();
  Gp.desaloca();
  Gn.desaloca();
  map.desaloca();
  stat.desaloca();
  comp.desaloca();
}


//---------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------

/* vizinhanca do GRASP para o problema do grafo de sinais */
void Sigraph::testa_particoes_grasp_sig_v2(Array<int> &A, Array<int> &a, Array<int> &B,
    Array<int> &b, Array<int> &C, Array<int> &c) {
  int  ii, jj;
  bool achou;

  // testa A,B
  for (ii = 0; ii < n; ii++)
    for (jj = ii + 1; jj < n; jj++)
      if ((A[ii] == 1) && (A[jj] == 1) && (g(ii, jj).neg)) {
        std::cout << "vertice " << ii << " (A)" << " e vertice " << jj << " (A) tem are neg"
            << std::endl; abort();
      }

  for (ii = 0; ii < n; ii++)
    for (jj = ii + 1; jj < n; jj++)
      if ((B[ii] == 1) && (B[jj] == 1) && (g(ii, jj).neg)) {
        std::cout << "vertice " << ii << " (B)" << " e vertice " << jj << " (B) tem are neg"
            << std::endl; abort();
      }

  for (ii = 0; ii < n; ii++)
    for (jj = 0; jj < n; jj++)
      if ((A[ii] == 1) && (B[jj] == 1) && (g(ii, jj).pos)) {
        std::cout << "vertice " << ii << " (A)" << " e vertice " << jj << " (B) tem are pos"
            << std::endl; abort();
      }

  for (ii = 0; ii < n; ii++)
    if (A[ii] + B[ii] + C[ii] != 1) {
      std::cout << "soma do vertice " << ii << " = " << A[ii] + B[ii] + C[ii] << std::endl; abort();
    }

  // testa a,b,c
  for (ii = 0; a[ii] != -1; ii++)
    if ((a[ii] != -2) && (A[a[ii]] == 0)) {
      std::cout << "vertice " << a[ii] << " em (a), mas nao esta em (A)" << std::endl; abort();
    }

  for (ii = 0; b[ii] != -1; ii++)
    if ((b[ii] != -2) && (B[b[ii]] == 0)) {
      std::cout << "vertice " << b[ii] << " em (b), mas nao esta em (B)" << std::endl; abort();
    }

  for (ii = 0; c[ii] != -1; ii++)
    if ((c[ii] != -2) && (C[c[ii]] == 0)) {
      std::cout << "vertice " << c[ii] << " em (c), mas nao esta em (C)" << std::endl; abort();
    }

  for (ii = 0; ii < n; ii++)
    if (A[ii] == 1) {
      for (achou = false, jj = 0; a[jj] != -1; jj++)
        if (a[jj] == ii) {
          achou = true;
          break;
        }
      if (!achou) {
        std::cout << "vertice " << ii << " em (A), mas nao esta em (a)" << std::endl; abort();
      }
    }

  for (ii = 0; ii < n; ii++)
    if (B[ii] == 1) {
      for (achou = false, jj = 0; b[jj] != -1; jj++)
        if (b[jj] == ii) {
          achou = true;
          break;
        }
      if (!achou) {
        std::cout << "vertice " << ii << " em (B), mas nao esta em (b)" << std::endl; abort();
      }
    }

  for (ii = 0; ii < n; ii++)
    if (C[ii] == 1) {
      for (achou = false, jj = 0; c[jj] != -1; jj++)
        if (c[jj] == ii) {
          achou = true;
          break;
        }
      if (!achou) {
        std::cout << "vertice " << ii << " em (C), mas nao esta em (c)" << std::endl; abort();
      }
    }
}



/* atualiza vetores a e b */
void Sigraph::atualiza_a_b_c(Array<int> &A, Array<int> &a, Array<int> &B, Array<int> &b,
    Array<int> &C, Array<int> &c) {
  int temA, temB, temC, i;

  for (temA = temB = temC = i = 0; i < n; i++) {
    if (A[i] == 1) {
      a[temA] = i;
      temA++;
    }
    if (B[i] == 1) {
      b[temB] = i;
      temB++;
    }
    if (C[i] == 1) {
      c[temC] = i;
      temC++;
    }
  }
  a[temA] = -1;
  b[temB] = -1;
  c[temC] = -1;
}


/* metodo construtivo do GRASP para o problema do grafo de sinais */
int Sigraph::metodo_construtivo_grasp_sig_v2(Array<int> &A, Array<int> &a, Array<int> &B,
    Array<int> &b, Array<int> &C,  Array<int> &c, Array<int> &cand1, Array<int> &cand2, int TEST) {
  int i, j, v, vez, temA, temB, num_cand, num_sol;
  bool e_cand;

  int DEPU = 0;

  if (DEPU) std::cout << std::endl <<
      "*************  metodo_construtivo_grasp_sig  ****************** " << std::endl;

  /* inicializa conjuntos */
  A.set(0);
  B.set(0);
  C.set(1);

  /* escolhe vertice inicial aleatorio e coloca em A */
  for (v = (int)(rand() % n); stat[v] != 1; v++)
    if (v == (n-1)) v = -1;
  A[v] = 1; C[v] = 0;
  num_sol = 1;

  if (DEPU) std::cout << " v_ini = " << v << " em A" << std::endl;

  /* tenta botar a proxima vez no lado B */
  vez = 0;
  temA = 1;
  temB = 1;

  while ((temA == 1) || (temB == 1)) {
    /* preenche candidatos a entrar em A */
    if (vez == 1) {
      cand1.set(0);
      for (i = 0, num_cand = 0; i < n; i++)
        if ((stat[i] == 1) && (C[i] == 1)) {
          /* checa vizinhanca em A */
          for (e_cand = true, j = 0; j < n && e_cand == true; j++)
            if ((A[j]==1) && (g(i,j).neg))
              e_cand = false;

          /* checa vizinhanca em B */
          for (j = 0; j < n && e_cand == true; j++)
            if ((B[j] == 1) && (g(i,j).pos))
              e_cand = false;

          if (e_cand) {
            cand1[i]=1;
            num_cand++;
          }
	      }

      /* não tem mais vertice para inserir em A */
      if (num_cand == 0)
        temA = 0;
    } else {
      /* preenche candidatos a entrar em B */
      cand1.set(0);
      for (i = 0, num_cand = 0; i < n; i++)
        if ((stat[i] == 1) && (C[i] == 1)) {
          /* checa vizinhanca em A */
          for (e_cand = true, j = 0; j < n && e_cand == true; j++)
            if ((A[j] == 1) && (g(i,j).pos))
              e_cand = false;

          /* checa vizinhanca em B */
          for (j = 0; j < n && e_cand == true; j++)
            if ((B[j] == 1) && (g(i,j).neg))
              e_cand = false;

          if (e_cand) {
            cand1[i] = 1;
            num_cand++;
          }
        }

      /* não tem mais vertice para inserir em B */
      if (num_cand == 0)
        temB = 0;
    }

    /* insere em A */
    if ((vez == 1) && (num_cand > 0)) {
  	  /* escolhe vertice candidato aleatorio e coloca em A */
  	  for (v = (int)(rand() % n); (stat[v] != 1 || cand1[v] != 1); v++)
  	    if (v == (n-1)) v = -1;
  	  A[v] = 1; C[v] = 0; num_sol++;
  	  if (DEPU) std::cout << "   " << v << " em A" << std::endl;
  	}

    /* insere em B */
    if ((vez == 0) && (num_cand > 0)) {
  	  /* escolhe vertice candidato aleatorio e coloca em B */
  	  for (v = (int)(rand() % n); (stat[v] != 1 || cand1[v] != 1); v++)
  	    if (v == (n-1)) v = -1;
  	  B[v] = 1; C[v] = 0; num_sol++;
  	  if (DEPU) std::cout << "   " << v << " em B" << std::endl;
  	}

    /* escolhe proxima particao para ser inserido vertice */
    if (temA == 1 && temB == 1)
      vez = (int) (rand() % 2);
    else
      if (temA == 1) vez = 1;
      else vez = 0;
    if (DEPU) std::cout << "escolhe de " << vez << " agora" << std::endl;
  }/* while ((temA == 1) || (temB == 1)) */

  // /* prenche vetores "a" e "b" */
  if (DEPU) std::cout << "prenche vetores a e b" << std::endl;
  atualiza_a_b_c(A, a, B, b, C, c);

  if (TEST) testa_particoes_grasp_sig_v2(A, a, B, b, C, c);
  return num_sol;
}



/* metodo construtivo do GRASP para o problema do grafo de sinais */
void Sigraph::maximiza_solucao_grasp_sig_v2(Array<int> &A, Array<int> &a, Array<int> &B,
    Array<int> &b, Array<int> &C,  Array<int> &c, Array<int> &cand1, Array<int> &cand2,
    int TEST) {
  int i, v, vez, temA, temB, num_cand;
  int DEPU = 0;

  //if (DEPU) std::cout<<std::endl<<"*************  maximiza_solucao_grasp_sig_v2  ****************** "<<std::endl;

  /* tenta botar a proxima vez no lado B */
  vez = (int) (rand() % 2);
  temA = 1;
  temB = 1;

  while ((temA == 1) || (temB == 1)) {
    /* preenche candidatos a entrar em A */
    if (vez == 1) {
      for (num_cand = 0, i = 0; c[i] != -1; i++)
        if ((c[i] != -2) && (stat[c[i]] == 1)) {
          if (can_move_C_X_v2(c[i], a, b, C)) {
            cand1[num_cand] = c[i];
            num_cand++;
          }
        }

      /* nao tem mais vertice para inserir em A */
      if (num_cand==0)
      temA = 0;
    } else {
      /* preenche candidatos a entrar em B */
      for (num_cand = 0, i = 0; c[i] != -1; i++)
        if ((c[i] != -2) && (stat[c[i]] == 1)) {
          if (can_move_C_X_v2(c[i], b, a, C)) {
            cand1[num_cand] = c[i];
            num_cand++;
          }
        }

      /* nao tem mais vertice para inserir em B */
      if (num_cand == 0)
        temB = 0;
    }

    /* insere em A */
    if ((vez == 1) && (num_cand > 0)) {
      /* escolhe vertice candidato aleatorio e coloca em A */
      v = (int) (rand() % num_cand);
      A[cand1[v]] = 1; C[cand1[v]] = 0;
      atualiza_a_b_c(A, a, B, b, C, c);
      if (DEPU) std::cout << "      MAX " << cand1[v] << " de C para A" << std::endl;
    }

    /* insere em B */
    if ((vez == 0) && (num_cand > 0)) {
      /* escolhe vertice candidato aleatorio e coloca em A */
      v = (int)(rand() % num_cand);
      B[cand1[v]] = 1; C[cand1[v]] = 0;
      atualiza_a_b_c(A, a, B, b, C, c);
      if (DEPU) std::cout << "      MAX " << cand1[v] << " de C para B" << std::endl;
    }

    /* escolhe proxima particao para ser inserido vertice */
    if (temA == 1 && temB == 1)
      vez = (int) (rand() % 2);
    else
      if (temA == 1) vez = 1;
      else vez = 0;
      //if (DEPU) std::cout<<"escolhe de "<<vez<<" agora"<<std::endl;

  }/* while ((temA == 1) || (temB == 1)) */

  if (TEST) testa_particoes_grasp_sig_v2(A, a, B, b, C, c);
}



/* testa se pode mover um vertice "l" de C para X (A ou B) e nao Y (A ou B diferente de X) */
bool Sigraph::can_move_C_X_v2(int l, Array<int> &x, Array<int> &y, Array<int> &C) {
  int j;
  bool e_cand;
  int DEPU = 0;

  if ((stat[l] == 1) && (C[l] == 1)) {
    for (e_cand = true, j = 0; x[j] != -1 && e_cand == true; j++)
      if ((x[j] != -2) && (g(l, x[j]).neg)) {
        e_cand = false;
        if (DEPU) std::cout << l << "    are neg com " << x[j] << std::endl;
      }

    for (j = 0; y[j] != -1 && e_cand == true; j++)
      if ((y[j] != -2) && (g(l, y[j]).pos)) {
        e_cand = false;
        if (DEPU) std::cout << l << "  are pos com " << y[j] << std::endl;
      }

    return e_cand;
  }
  return false;
}



/* testa se pode mover dois vertices "l1" e "l2" de C para X (A ou B) e nao Y (A ou B diferente de X) */
bool Sigraph::can_2move_C_X_v2(int l1, int l2, Array<int> &x, Array<int> &y, Array<int> &C) {
  int j;
  bool e_cand;

  if ((stat[l1] == 1) && (C[l1] == 1) && (stat[l2] == 1) && (C[l2] == 1)) {
    if (g(l1, l2).neg)
      return false;

    for (e_cand = true, j = 0; x[j] != -1 && e_cand == true; j++)
      if ((x[j] != -2) && ((g(l1, x[j]).neg) || (g(l2, x[j]).neg)))
        e_cand = false;

    for (j = 0; y[j] != -1 && e_cand == true; j++)
      if ((y[j] != -2) && ((g(l1, y[j]).pos) || (g(l2, y[j]).pos)))
        e_cand = false;

    return e_cand;
  }
  return false;
}



/* testa se pode mover um vertice "l" de C para A ou B -> 0 nao pode, 1 pode para A, 2 pode para B */
int Sigraph::can_move_C_AB_v2(int l, Array<int> &a, Array<int> &b, Array<int> &C, int TEST) {
  int vez;
  bool testa_A, testa_B;

  if ((stat[l] == 1) && (C[l] == 1)) {
    testa_A = false;
    testa_B = false;
    vez = (int) (rand() % 2);

    while (!testa_A || !testa_B) {
      /* -> A */
      if ((vez == 1) && (!testa_A)) {
        if (can_move_C_X_v2(l, a, b, C))
          return 1;

        testa_A = true;
        vez = 0;
      }

      /* -> B */
      if ((vez == 0) && (!testa_B)) {
        if (can_move_C_X_v2(l, b, a, C))
          return 2;

        testa_B = true;
        vez = 1;
      }

    }/* while (!testa_A || !testa_B) */
  }/* if ((stat[l] == 1) && (C[l] == 1)) */
  return 0;
}



/* testa se pode mover dois vertices "c[l1]" e "c[l2]" de C para A ou B ->
0 nao pode,
1 pode, c[l1] (A) c[l2] (A)
2 pode, c[l1] (A) c[l2] (B)
3 pode, c[l1] (B) c[l2] (A)
4 pode, c[l1] (B) c[l2] (B)
*/
int Sigraph::can_2move_C_AB_v2(int l1, int l2, int num_candA, int num_candB, Array<int> &a,
    Array<int> &b, Array<int> &C, Array<int> &c, int TEST) {
  int vez;
  bool testa_AA, testa_AB, testa_BA, testa_BB;

  if ((stat[c[l1]] == 1) && (C[c[l1]] == 1) && (stat[c[l2]] == 1) && (C[c[l2]] == 1)) {
    testa_AA = false;
    testa_AB = false;
    testa_BA = false;
    testa_BB = false;
    vez = (int) (rand() % 4);

    while (!testa_AA || !testa_AB || !testa_BA || !testa_BB) {
  	  /* -> AA */
  	  if ((vez == 0) && (!testa_AA)) {
	      if (can_2move_C_X_v2(c[l1], c[l2], a, b, C))
          return 1;

	      testa_AA = true;
	      vez = 1;
	    }

  	  /* -> AB */
  	  if ((vez == 1) && (!testa_AB)) {
        if (can_move_C_X_v2(c[l1], a, b, C)) {
          a[num_candA] = c[l1];
          a[num_candA + 1] = -1;
          c[l1] = -2;

          if (can_move_C_X_v2(c[l2], b, a, C)) {
  		      c[l1] = a[num_candA]; a[num_candA] = -1;
  		      return 2;
  		    }
          c[l1] = a[num_candA]; a[num_candA] = -1;
        }

        testa_AB = true;
        vez = 2;
      }

  	  /* -> BA */
  	  if ((vez == 2) && (!testa_BA)) {
        if (can_move_C_X_v2(c[l1], b, a, C)) {
          b[num_candB] = c[l1];
          b[num_candB + 1] = -1;
          c[l1] = -2;

          if (can_move_C_X_v2(c[l2], a, b, C)) {
  		      c[l1] = b[num_candB]; b[num_candB] = -1;
  		      return 3;
  		    }
          c[l1] = b[num_candB]; b[num_candB] = -1;
        }

	      testa_BA = true;
	      vez = 3;
	    }

  	  /* -> BB */
  	  if ((vez == 3) && (!testa_BB)) {
	      if (can_2move_C_X_v2(c[l1], c[l2], b, a, C))
		      return 4;

  	      testa_BB = true;
  	      vez = 0;
	    }
    }/* while (!testa_AA || !testa_AB || !testa_BA || !testa_BB) */
  }/* if ((stat[l1] == 1) && (C[l1] == 1) && (stat[l2] == 1) && (C[l2] == 1)) */
  return 0;
}



/* tenta mover 2 vertices de C para A ou B */
bool Sigraph::move_2_C_AB_v2(int desceu, int origem, Array<int> &A, Array<int> &a, Array<int> &B,
    Array<int> &b, Array<int> &C, Array<int> &c, Array<int> &cand1, Array<int> &cand2, int TEST) {
  int j, l, parada_2, parada_3, vert_swap, i_swap, vez, num_candA, num_candB, num_candC;
  bool e_cand, moveu, testa_A, testa_B;

  int DEPU = 0;

  moveu = false;
  testa_A = false;
  testa_B = false;
  vez = (int)(rand() % 2);

  /* ----------------------------- conta vertices (incluindo -2) */
  for (num_candA = 0; a[num_candA] != -1; num_candA++);
  for (num_candB = 0; b[num_candB] != -1; num_candB++);
  for (num_candC = 0; c[num_candC] != -1; num_candC++);

  while ((!testa_A || !testa_B) && (!moveu)) {
    /* -----------> 2 NIVEL) C -> A */
    if ((vez == 1) && (!moveu) && (!testa_A)) {
      for (parada_2 = num_candC, i_swap = (int) (rand() % num_candC);
          parada_2 > 0 && (moveu == false); i_swap++) {
        vert_swap = c[i_swap];
	      if ((vert_swap != -2) && (stat[vert_swap] == 1)
            && (C[vert_swap] == 1) && ((desceu != vert_swap) || (origem != 0))) {
          e_cand = can_move_C_X_v2(vert_swap, a, b, C);

          if (e_cand) {
  		      if (DEPU) std::cout << std::endl << "  vert_CA = " << vert_swap << std::endl;
  		      A[vert_swap] = 1;
  		      C[vert_swap] = 0;
  		      c[i_swap] = -2;
  		      a[num_candA] = vert_swap;
  		      a[num_candA + 1] = -1;

            /* 3 NIVEL) C -> A ou B */
            for (parada_3 = num_candC, l = (int) (rand() % num_candC);
                parada_3 > 0 && (moveu == false); l++) {
              if (c[l] != -2) {
                j = can_move_C_AB_v2(c[l], a, b, C, TEST);

                if (j == 1) {
                  if (DEPU) std::cout << std::endl << "    vert_CA = " << c[l] << " MOVEU !"
                      << std::endl;
  	              A[c[l]] = 1; C[c[l]] = 0;
  		            atualiza_a_b_c(A, a, B, b, C, c);
  		            moveu = true;
  		          }
                if (j == 2) {
                  if (DEPU) std::cout << std::endl << "    vert_CB = " << c[l] << " MOVEU !"
                      << std::endl;
                  B[c[l]] = 1; C[c[l]] = 0;
                  atualiza_a_b_c(A, a, B, b, C, c);
                  moveu = true;
                }
              }

              if (l == (num_candC-1)) l = -1;
              parada_3--;
            }

            if (!moveu) {
              C[vert_swap] = 1;
              A[vert_swap] = 0;
              c[i_swap]    = vert_swap;
              a[num_candA] = -1;
              if (TEST) testa_particoes_grasp_sig_v2(A, a, B, b, C, c);
            }
          }/* if (e_cand)  */
        }/*  if ((vert_swap != -2) && (stat[vert_swap] == 1) &&...  */
	      if (i_swap == (num_candC-1)) i_swap = -1;
	      parada_2--;
	    }/* for (parada_2=n, i=(int)(rand() % n); parada_2 > 0 && (moveu==false); i++) */
      testa_A = true;
      vez     = 0;
    }/* if (vez == 1) */

    /* -----------> 2 NIVEL) C -> B */
    if ((vez == 0) && (!moveu) && (!testa_B)) {
      for (parada_2 = num_candC, i_swap = (int) (rand() % num_candC);
          parada_2 > 0 && (moveu == false); i_swap++) {
        vert_swap = c[i_swap];
	      if ((vert_swap != -2) && (stat[vert_swap] == 1) && (C[vert_swap] == 1)
            && ((desceu != vert_swap) || (origem != 1))) {
          e_cand = can_move_C_X_v2(vert_swap, b, a, C);

          if (e_cand) {
  		      if (DEPU) std::cout << std::endl << "  vert_CB = " << vert_swap << std::endl;
  		      B[vert_swap] = 1;
  		      C[vert_swap] = 0;
  		      c[i_swap] = -2;
  		      b[num_candB] = vert_swap;
  		      b[num_candB + 1] = -1;

  		      /* 3 NIVEL) C -> A ou B */
  		      for (parada_3 = num_candC, l=(int)(rand() % num_candC);
                parada_3 > 0 && (moveu==false); l++) {
              if (c[l] != -2) {
                j = can_move_C_AB_v2(c[l], a, b, C, TEST);

    			      if (j == 1) {
                  if (DEPU) std::cout << std::endl << "    vert_CA = " << c[l] << " MOVEU !"
                      << std::endl;
                  A[c[l]] = 1; C[c[l]] = 0;
                  atualiza_a_b_c(A, a, B, b, C, c);
                  moveu = true;
                }
    			      if (j == 2) {
                  if (DEPU) std::cout << std::endl << "    vert_CB = " << c[l] << " MOVEU !"
                      << std::endl;
                  B[c[l]] = 1; C[c[l]] = 0;
                  atualiza_a_b_c(A, a, B, b, C, c);
                  moveu = true;
                }
              }

              if (l == (num_candC-1)) l = -1;
              parada_3--;
            }

  		      if (!moveu) {
      			  C[vert_swap] = 1;
      			  B[vert_swap] = 0;
      			  c[i_swap] = vert_swap;
      			  b[num_candB] = -1;
      			  if (TEST) testa_particoes_grasp_sig_v2(A, a, B, b, C, c);
            }
  		    }/* if (e_cand)  */
        }/* if ((vert_swap != -2) && (stat[vert_swap] == 1) && ... */
        if (i_swap == (num_candC-1)) i_swap = -1;
        parada_2--;
      }/* for (parada_2=num_cand, i_swap=(int)(rand()... */
  	  testa_B = true;
  	  vez = 1;
  	}/* if (vez == 0)*/
  }/* while (!testa_A || !testa_B) */
  return moveu;
}



/* tenta mover 3 vertices de C para A ou B */
bool Sigraph::move_3_C_AB_v2(int desceu1, int desceu2, int origem1, int origem2, Array<int> &A,
    Array<int> &a, Array<int> &B, Array<int> &b, Array<int> &C, Array<int> &c, Array<int> &cand1,
    Array<int> &cand2, int TEST) {
  int j, l1, l2, parada_2, parada_3, parada_4, vert_swap, i_swap, vez, num_candA, num_candB, num_candC;
  bool e_cand, moveu, testa_A, testa_B;

  int DEPU = 0;

  moveu = false;
  testa_A = false;
  testa_B = false;
  vez = (int) (rand() % 2);

  /* ----------------------------- conta vertices (incluindo -2) */
  for (num_candA = 0; a[num_candA] != -1; num_candA++);
  for (num_candB = 0; b[num_candB] != -1; num_candB++);
  for (num_candC = 0; c[num_candC] != -1; num_candC++);

  while ((!testa_A || !testa_B) && (!moveu)) {
    /* -----------> 2 NIVEL) C -> A */
    if ((vez == 1) && (!moveu) && (!testa_A)) {
      for (parada_2 = num_candC, i_swap = (int) (rand() % num_candC);
          parada_2 > 0 && (moveu == false); i_swap++) {
	      vert_swap = c[i_swap];

	      if ((vert_swap != -2) && (stat[vert_swap] == 1) && (C[vert_swap] == 1)
            && ((desceu1 != vert_swap) || (origem1 != 0)) && ((desceu2 != vert_swap)
            || (origem2 != 0))) {
          e_cand = can_move_C_X_v2(vert_swap, a, b, C);

	        if (e_cand) {
            if (DEPU) std::cout << std::endl << "  vert_CA = " << vert_swap << std::endl;
              A[vert_swap] = 1;
              C[vert_swap] = 0;
              c[i_swap] = -2;
              a[num_candA] = vert_swap;
              a[num_candA + 1] = -1;

            /* 3 NIVEL) 2C -> A ou B */
            for (parada_3 = num_candC, l1 = (int) (rand() % num_candC);
                parada_3 > 0 && (moveu == false); l1++) {
              if ((c[l1] != -2) && (stat[c[l1]] == 1) && (C[c[l1]] == 1))
                if ((can_move_C_X_v2(c[l1], a, b, C)) || (can_move_C_X_v2(c[l1], b, a, C)))
                  for (parada_4 = num_candC, l2 = (int) (rand() % num_candC);
                      parada_4 > 0 && (moveu == false); l2++) {
                    if ((c[l2] != -2) && (c[l1] < c[l2]) && (stat[c[l2]] == 1) && (C[c[l2]] == 1)) {
                      j = can_2move_C_AB_v2(l1, l2, num_candA+1, num_candB, a, b, C, c, TEST);
                      if (j == 1) {
                        if (DEPU) std::cout << std::endl << "    vert_C_AA = " << l1 << "," << l2
                            << " 2MOVEU !" << std::endl;
                        A[c[l1]] = 1; C[c[l1]] = 0; A[c[l2]] = 1; C[c[l2]] = 0;
                        atualiza_a_b_c(A, a, B, b, C, c); moveu = true;
                      }
                      if (j == 2) {
                        if (DEPU) std::cout << std::endl << "    vert_C_AB = " << l1 << "," << l2
                            << " 2MOVEU !" << std::endl;
                        A[c[l1]] = 1; C[c[l1]] = 0; B[c[l2]] = 1; C[c[l2]] = 0;
                        atualiza_a_b_c(A, a, B, b, C, c); moveu = true;
                      }
                      if (j == 3) {
                        if (DEPU) std::cout << std::endl << "    vert_C_BA = " << l1 << "," << l2
                            << " 2MOVEU !" << std::endl;
                        B[c[l1]] = 1; C[c[l1]] = 0; A[c[l2]] = 1; C[c[l2]] = 0;
                        atualiza_a_b_c(A, a, B, b, C, c); moveu = true;
                      }
                      if (j == 4) {
                        if (DEPU) std::cout << std::endl << "    vert_C_BB = " << l1 << "," << l2
                            << " 2MOVEU !" << std::endl;
                        B[c[l1]]  = 1; C[c[l1]]  = 0; B[c[l2]]  = 1; C[c[l2]]  = 0;
                        atualiza_a_b_c(A, a, B, b, C, c); moveu = true;
                      }
                  }/*if ((c[l2] != -2) && (c[l1] < c[l2]) && (stat[c[l2]] == 1) && (C[c[l2]] == 1)) */
                  if (l2 == (num_candC - 1)) l2 = -1;
                  parada_4--;
                }/* for (parada_4=num_candC, l2=(int)(rand() % num_candC); parada_4 > 0 && (moveu==false); l2++) */
              if (l1 == (num_candC - 1)) l1 = -1;
              parada_3--;
            }/* for (parada_3=num_candC, l1=(int)(rand() % num_candC); parada_3 > 0 && (moveu==false); l1++) */
            if (!moveu) {
      			  C[vert_swap] = 1;
      			  A[vert_swap] = 0;
      			  c[i_swap] = vert_swap;
      			  a[num_candA] = -1;
      			  if (TEST) testa_particoes_grasp_sig_v2(A, a, B, b, C, c);
      			}
          }/* if (e_cand)  */
        }/* if ((vert_swap != -2) && (stat[vert_swap] == 1) &&... */
        if (i_swap == (num_candC-1)) i_swap = -1;
        parada_2--;
      }/* for (parada_2=num_candC, i_swap=(int)(rand() ...*/
  	  testa_A = true;
  	  vez = 0;
    }/* if ((vez == 1) && (!moveu) && (!testa_A)) */

    /* -----------> 2 NIVEL) C -> B */
    if ((vez == 0) && (!moveu) && (!testa_B)) {
      for (parada_2 = num_candC, i_swap = (int) (rand() % num_candC); parada_2 > 0
          && (moveu == false); i_swap++) {
        vert_swap = c[i_swap];

        if ((vert_swap != -2) && (stat[vert_swap] == 1) && (C[vert_swap] == 1)
            && ((desceu1 != vert_swap) || (origem1 != 1)) && ((desceu2 != vert_swap)
            || (origem2 != 1))) {
          e_cand = can_move_C_X_v2(vert_swap, b, a, C);

          if (e_cand) {
            if (DEPU)
              std::cout << std::endl << "  vert_CB = " << vert_swap << std::endl;
  		      B[vert_swap] = 1;
  		      C[vert_swap] = 0;
  		      c[i_swap] = -2;
  		      b[num_candB] = vert_swap;
  		      b[num_candB + 1] = -1;

            /* 3 NIVEL) 2C -> A ou B */
            for (parada_3 = num_candC, l1 = (int) (rand() % num_candC);
                parada_3 > 0 && (moveu == false); l1++) {
              if ((c[l1] != -2) && (stat[c[l1]] == 1) && (C[c[l1]] == 1))
                if ((can_move_C_X_v2(c[l1], a, b, C)) || (can_move_C_X_v2(c[l1], b, a, C)))
                for (parada_4 = num_candC, l2 = (int) (rand() % num_candC);
                    parada_4 > 0 && (moveu == false); l2++) {
                  if ((c[l2] != -2) && (c[l1] < c[l2]) && (stat[c[l2]] == 1) && (C[c[l2]] == 1)) {
                    j = can_2move_C_AB_v2(l1, l2, num_candA, num_candB+1, a, b, C, c, TEST);
                    if (j == 1) {
                      if (DEPU) std::cout << std::endl << "    vert_C_AA = " << l1 << "," << l2
                          << " 2MOVEU !" << std::endl;
                      A[c[l1]] = 1; C[c[l1]] = 0; A[c[l2]] = 1; C[c[l2]] = 0;
                      atualiza_a_b_c(A, a, B, b, C, c); moveu = true;
                    }
                    if (j == 2) {
                      if (DEPU) std::cout << std::endl << "    vert_C_AB = " << l1 << "," << l2
                          << " 2MOVEU !" << std::endl;
                      A[c[l1]] = 1; C[c[l1]] = 0; B[c[l2]] = 1; C[c[l2]] = 0;
                      atualiza_a_b_c(A, a, B, b, C, c); moveu = true;
                    }
                    if (j == 3) {
                      if (DEPU) std::cout << std::endl << "    vert_C_BA = " << l1 << "," << l2
                          << " 2MOVEU !" << std::endl;
                      B[c[l1]]  = 1; C[c[l1]]  = 0; A[c[l2]]  = 1; C[c[l2]]  = 0;
                      atualiza_a_b_c(A, a, B, b, C, c); moveu = true;
                    }
                    if (j == 4) {
                      if (DEPU) std::cout << std::endl << "    vert_C_BB = " << l1 << "," << l2
                          << " 2MOVEU !" << std::endl;
                      B[c[l1]] = 1; C[c[l1]] = 0; B[c[l2]] = 1; C[c[l2]] = 0;
                      atualiza_a_b_c(A, a, B, b, C, c); moveu = true;
                    }
                  }/*if ((c[l2] != -2) && (c[l1] < c[l2]) && (stat[c[l2]] == 1) && (C[c[l2]] == 1)) */
                  if (l2 == (num_candC-1)) l2 = -1;
                  parada_4--;
                }/* for (parada_4=num_candC, l2=(int)(rand() % num_candC); parada_4 > 0 && (moveu==false); l2++) */
              if (l1 == (num_candC-1)) l1 = -1;
              parada_3--;
            }/* for (parada_3=num_candC, l1=(int)(rand() % num_candC); parada_3 > 0 && (moveu==false); l1++) */
            if (!moveu) {
      			  C[vert_swap] = 1;
      			  B[vert_swap] = 0;
      			  c[i_swap] = vert_swap;
      			  b[num_candB] = -1;
      			  if (TEST) testa_particoes_grasp_sig_v2(A, a, B, b, C, c);
      			}
          }/* if (e_cand)  */
        }/* if ((vert_swap != -2) && (stat[vert_swap] == 1) &&... */
        if (i_swap == (num_candC - 1)) i_swap = -1;
        parada_2--;
      }/* for (parada_2=num_candC, i_swap=(int)(rand() ...*/
      testa_B = true;
      vez = 1;
    }/* if ((vez == 1) && (!moveu) && (!testa_B)) */
  }/* while ((!testa_A || !testa_B) && (!moveu)) */
  return moveu;
}



/* vizinhanca do GRASP para o problema do grafo de sinais -> desce um de A e sobem 2 para A ou B */
int Sigraph::viz_down_A_v2(Array<int> &A, Array<int> &a, Array<int> &B, Array<int> &b,
    Array<int> &C, Array<int> &c, Array<int> &cand1, Array<int> &cand2, int TEST, timeval t_ini_g) {

  // int num_candA, num_candC, vert_AC, i_AC, parada_1;
  int num_candA, num_candC, vert_AC;
  bool moveu = false;

  int DEPU = 0;

  if (DEPU) std::cout << std::endl << "*************  viz_down_A  ****************** " << std::endl;

  /* ----------------------------- conta numero de candidatos de A e C */
  for (num_candA = 0; a[num_candA] != -1; num_candA++);
  for (num_candC = 0; c[num_candC] != -1; num_candC++);

  if (num_candA == 0)
    return 0;

  #pragma omp parallel private(vert_AC)
  {
    bool i_moveu = false;
    int iam = 0;
    int np = 1;

    iam = omp_get_thread_num();
    np = omp_get_num_threads();

    thread_data_t thr_data;
    thr_data.tid = iam;
    thr_data.A = A;
    thr_data.a = a;
    thr_data.B = B;
    thr_data.b = b;
    thr_data.C = C;
    thr_data.c = c;
    thr_data.cand1 = cand1;
    thr_data.cand2 = cand2;
    thr_data.TEST = TEST;
    thr_data.t_ini_g = t_ini_g;
    thr_data.obj = this;

    #pragma omp barrier

    // divisão do trabalho
		int local_init, local_end, chunk;
    chunk = num_candA / np;
		int l_rest = num_candA % np;
    if (l_rest == 0) {
      local_init = iam * chunk;
  		local_end = (iam + 1) * chunk;
    } else {
      if (iam < l_rest) {
        local_init = iam * (chunk + 1);
    		local_end = (iam + 1) * (chunk + 1);
      } else {
        // int pli = (l_rest - 1) * (chunk + 1);  // previous local init
        int ple = (l_rest) * (chunk + 1);  // previous local end
        local_init = ple + (iam - l_rest) * chunk;
    		local_end = ple + (iam - l_rest + 1) * chunk;
      }
    }
    // #pragma omp critical
    // {
    //   std::cout << rank << ":" << iam << ":local_init [" << local_init << "]" << " local_end ["
    //       << local_end << "] " << "num_candA [" << num_candA << "]." << std::endl;
    // }
    /* 1 NIVEL) A -> C */
    // for (vert_AC = -1, parada_1 = num_candA, i_AC = (int) (rand() % num_candA); parada_1 > 0; i_AC++) {
    // for (int i_AC = 0; i_AC < num_candA; i_AC++) {
    for (int i_AC = local_init; i_AC < local_end; i_AC++) {
      if (moveu) break;
  		if (thr_data.a[i_AC] != -2) {
  	    vert_AC = thr_data.a[i_AC];
  	    if (DEPU) std::cout << std::endl << "vert_AC = " << vert_AC << std::endl;

  	    /* ---> */
  	    thr_data.A[vert_AC] = 0;
  	    thr_data.C[vert_AC] = 1;
  	    thr_data.a[i_AC] = -2;
  	    thr_data.c[num_candC] = vert_AC;
  	    thr_data.c[num_candC + 1] = -1;

  	    i_moveu = move_2_C_AB_v2(vert_AC, 0, thr_data.A, thr_data.a, thr_data.B, thr_data.b,
            thr_data.C, thr_data.c, thr_data.cand1, thr_data.cand2, thr_data.TEST);

        #pragma omp critical
        {
    	    if (i_moveu == true) {
            if (!moveu) {
              m_n_viz_1a++;
              A = thr_data.A;
              a = thr_data.a;
              B = thr_data.B;
              b = thr_data.b;
              C = thr_data.C;
              c = thr_data.c;
              moveu = true;
            }
          }
        }

        if (moveu) break;

        if (!i_moveu) {
      		/* <--- */
      		thr_data.C[vert_AC] = 0;
      		thr_data.A[vert_AC] = 1;
      		thr_data.a[i_AC] = vert_AC;
      		thr_data.c[num_candC] = -1;
        }
      }
      // if (i_AC == (num_candA - 1)) i_AC = -1;
      // parada_1--;
      // if (moveu) break;
    } // for (int i_AC = local_init; i_AC < local_end; i_AC++) {
  }

  if (TEST) testa_particoes_grasp_sig_v2(A, a, B, b, C, c);
  if (moveu) {
    /* maximiza solucao */
    maximiza_solucao_grasp_sig_v2(A, a, B, b, C, c, cand1, cand2, TEST);
    return 1;
  } else
    return 0;
}



/* vizinhanca do GRASP para o problema do grafo de sinais -> desce um de B e sobem 2 para A ou B */
int Sigraph::viz_down_B_v2(Array<int> &A, Array<int> &a, Array<int> &B, Array<int> &b,
    Array<int> &C, Array<int> &c, Array<int> &cand1, Array<int> &cand2, int TEST,
    timeval t_ini_g) {
  // int num_candB, num_candC, vert_BC, i_BC, parada_1;
  int num_candB, num_candC, vert_BC;
  bool moveu = false;

  int DEPU = 0;

  if (DEPU) std::cout << std::endl << "*************  viz_down_B  ****************** " << std::endl;

  /* ----------------------------- conta numero de candidatos de B e C */
  for (num_candB = 0; b[num_candB] != -1; num_candB++);
  for (num_candC = 0; c[num_candC] != -1; num_candC++);

  if (num_candB == 0)
    return 0;

  #pragma omp parallel private(vert_BC)
  {
    bool i_moveu = false;
    int iam = 0;
    int np = 1;

    iam = omp_get_thread_num();
    np = omp_get_num_threads();

    thread_data_t thr_data;
    thr_data.tid = iam;
    thr_data.A = A;
    thr_data.a = a;
    thr_data.B = B;
    thr_data.b = b;
    thr_data.C = C;
    thr_data.c = c;
    thr_data.cand1 = cand1;
    thr_data.cand2 = cand2;
    thr_data.TEST = TEST;
    thr_data.t_ini_g = t_ini_g;
    thr_data.obj = this;

    #pragma omp barrier

    // divisão do trabalho
		int local_init, local_end, chunk;
    chunk = num_candB / np;
		int l_rest = num_candB % np;
    if (l_rest == 0) {
      local_init = iam * chunk;
  		local_end = (iam + 1) * chunk;
    } else {
      if (iam < l_rest) {
        local_init = iam * (chunk + 1);
    		local_end = (iam + 1) * (chunk + 1);
      } else {
        // int pli = (l_rest - 1) * (chunk + 1);  // previous local init
        int ple = (l_rest) * (chunk + 1);  // previous local end
        local_init = ple + (iam - l_rest) * chunk;
    		local_end = ple + (iam - l_rest + 1) * chunk;
      }
    }
    // #pragma omp critical
    // {
    //   std::cout << rank << ":" << iam << ":local_init [" << local_init << "]" << " local_end ["
    //       << local_end << "] " << "num_candA [" << num_candA << "]." << std::endl;
    // }
    /* 1 NIVEL) B -> C */
    /* for (vert_BC = -1, parada_1 = num_candB, i_BC = (int) (rand() % num_candB);
        parada_1 > 0; i_BC++) { */
    // for (int i_BC = 0; i_BC < num_candB; i_BC++) {
    for (int i_BC = local_init; i_BC < local_end; i_BC++) {
      if (moveu) break;
      if (b[i_BC] != -2) {
  	    vert_BC = thr_data.b[i_BC];
  	    if (DEPU) std::cout << std::endl << "vert_BC = " << vert_BC << std::endl;

  	    /* ---> */
  	    thr_data.B[vert_BC] = 0;
  	    thr_data.C[vert_BC] = 1;
  	    thr_data.b[i_BC] = -2;
  	    thr_data.c[num_candC] = vert_BC;
  	    thr_data.c[num_candC + 1] = -1;

  	    i_moveu = move_2_C_AB_v2(vert_BC, 1, thr_data.A, thr_data.a, thr_data.B, thr_data.b,
            thr_data.C, thr_data.c, thr_data.cand1, thr_data.cand2, thr_data.TEST);

        #pragma omp critical
        {
    	    if (i_moveu == true) {
            m_n_viz_1b++;
            if (!moveu) {
              A = thr_data.A;
              a = thr_data.a;
              B = thr_data.B;
              b = thr_data.b;
              C = thr_data.C;
              c = thr_data.c;
              moveu = true;
            }
          }
        }
        if (moveu) break;
  	    if (!i_moveu) {
      		/* <--- */
      		thr_data.C[vert_BC] = 0;
      		thr_data.B[vert_BC] = 1;
      		thr_data.b[i_BC] = vert_BC;
      		thr_data.c[num_candC] = -1;
        }
  	  }
      // if (i_BC == (num_candB - 1)) i_BC = -1;
      // parada_1--;
      // if (moveu == true) break;
    } // for (int i_BC = local_init; i_BC < local_end; i_BC++) {
  }
  if (TEST) testa_particoes_grasp_sig_v2(A, a, B, b, C, c);
  if (moveu) {
    /* maximiza solucao */
    maximiza_solucao_grasp_sig_v2(A, a, B, b, C, c, cand1, cand2, TEST);
    return 1;
  } else
    return 0;
}



/* vizinhanca do GRASP para o problema do grafo de sinais -> desce 2 de A e sobem 3 para A ou B */
int Sigraph::viz_2down_A_v2(Array<int> &A, Array<int> &a, Array<int> &B, Array<int> &b,
    Array<int> &C, Array<int> &c, Array<int> &cand1, Array<int> &cand2, int TEST, timeval t_ini_g,
    int TIMEMAX) {
  timeval end;
  // int vert_AC1, vert_AC2, i_AC1, i_AC2, parada_1, parada_2, num_candA, num_candC;
  int vert_AC1, vert_AC2, i_AC2, parada_2, num_candA, num_candC;
  bool moveu = false;

  int DEPU = 0;

  if (DEPU) std::cout << std::endl << "*************  viz_2down_A_v2  ****************** "
      << std::endl;

  /* ----------------------------- conta numero de candidatos de A e C */
  for (num_candA = 0; a[num_candA] != -1; num_candA++);
  for (num_candC = 0; c[num_candC] != -1; num_candC++);

  if (num_candA <= 1)
    return 0;

  #pragma omp parallel private(vert_AC1, vert_AC2, parada_2, i_AC2)
  {
    bool i_moveu = false;
    int iam = 0;
    int np = 1;

    iam = omp_get_thread_num();
    np = omp_get_num_threads();

    thread_data_t thr_data;
    thr_data.tid = iam;
    thr_data.A = A;
    thr_data.a = a;
    thr_data.B = B;
    thr_data.b = b;
    thr_data.C = C;
    thr_data.c = c;
    thr_data.cand1 = cand1;
    thr_data.cand2 = cand2;
    thr_data.TEST = TEST;
    thr_data.t_ini_g = t_ini_g;
    thr_data.obj = this;

    #pragma omp barrier

    // divisão do trabalho
		int local_init, local_end, chunk;
    chunk = num_candA / np;
		int l_rest = num_candA % np;
    if (l_rest == 0) {
      local_init = iam * chunk;
  		local_end = (iam + 1) * chunk;
    } else {
      if (iam < l_rest) {
        local_init = iam * (chunk + 1);
    		local_end = (iam + 1) * (chunk + 1);
      } else {
        // int pli = (l_rest - 1) * (chunk + 1);  // previous local init
        int ple = (l_rest) * (chunk + 1);  // previous local end
        local_init = ple + (iam - l_rest) * chunk;
    		local_end = ple + (iam - l_rest + 1) * chunk;
      }
    }
    // #pragma omp critical
    // {
    //   std::cout << rank << ":" << iam << ":local_init [" << local_init << "]" << " local_end ["
    //       << local_end << "] " << "num_candA [" << num_candA << "]." << std::endl;
    // }
    /* 1 NIVEL) A -> C */
    // for (vert_AC = -1, parada_1 = num_candA, i_AC = (int) (rand() % num_candA); parada_1 > 0; i_AC++) {

    /* 1 NIVEL) 1A -> C */
    // for (vert_AC1 = -1, parada_1 = num_candA, i_AC1 = (int) (rand() % num_candA);
    //     parada_1 > 0; i_AC1++) {
    // for (int i_AC1 = 0; i_AC1 < num_candA; i_AC1++) {
    for (int i_AC1 = local_init; i_AC1 < local_end; i_AC1++) {
      if (moveu) break;
      if (a[i_AC1] != -2) {
  	     vert_AC1 = thr_data.a[i_AC1];

    	  /* ---> */
    	  thr_data.A[vert_AC1] = 0;
    	  thr_data.C[vert_AC1] = 1;
    	  thr_data.a[i_AC1] = -2;
    	  thr_data.c[num_candC] = vert_AC1;
    	  thr_data.c[num_candC + 1] = -1;

        /* 1 NIVEL) 2A -> C */
        for (vert_AC2 = -1, parada_2 = num_candA, i_AC2 = (int) (rand() % num_candA);
            parada_2 > 0; i_AC2++) {
  	      if ((a[i_AC2] != -2) && (a[i_AC2] > vert_AC1)) {
            vert_AC2 = thr_data.a[i_AC2];
            if (DEPU) std::cout << std::endl << "vert_AA_C = " << vert_AC1 << "," << vert_AC2
                << std::endl;

            /* ---> */
      		  thr_data.A[vert_AC2] = 0;
      		  thr_data.C[vert_AC2] = 1;
      		  thr_data.a[i_AC2] = -2;
      		  thr_data.c[num_candC + 1] = vert_AC2;
      		  thr_data.c[num_candC + 2] = -1;

            i_moveu = move_3_C_AB_v2(vert_AC1, vert_AC2, 0, 0, thr_data.A, thr_data.a, thr_data.B,
                thr_data.b, thr_data.C, thr_data.c, thr_data.cand1, thr_data.cand2, thr_data.TEST);

            #pragma omp critical
            {
        	    if (i_moveu == true) {
                m_n_viz_2a++;
                if (!moveu) {
                  A = thr_data.A;
                  a = thr_data.a;
                  B = thr_data.B;
                  b = thr_data.b;
                  C = thr_data.C;
                  c = thr_data.c;
                  moveu = true;
                }
              }
            }
            if (moveu) break;
            if (!i_moveu) {
    		      /* <--- */
    		      thr_data.C[vert_AC2] = 0;
    		      thr_data.A[vert_AC2] = 1;
    		      thr_data.a[i_AC2] = vert_AC2;
    		      thr_data.c[num_candC + 1] = -1;

              // if (calcula_tempo(t_ini_g, (unsigned long int) clock()) > TIMEMAX)
              gettimeofday(&end, NULL);
              if (calcula_tempo(t_ini_g, end) > TIMEMAX) {
      		      /* <--- */
      		      thr_data.C[vert_AC1] = 0;
      		      thr_data.A[vert_AC1] = 1;
      		      thr_data.a[i_AC1] = vert_AC1;
      		      thr_data.c[num_candC] = -1;
      		      // return 0;
      		    }
            }
          }/* if ((a[i_AC2] != -2) && (a[i_AC2] > vert_AC1)) */
  	      if (i_AC2 == (num_candA - 1)) i_AC2 = -1;
  	      parada_2--;
          // if (moveu == true) break;
  	    }/* for (vert_AC2 = -1, parada_2=num_candA, i_AC2=(int)(rand() % num_candA); parada_2 > 0; i_AC2++) */
        // if (moveu == true)
        //   break;
        if (moveu) break;
        if (!i_moveu) {
  	      /* <--- */
  	      thr_data.C[vert_AC1] = 0;
  	      thr_data.A[vert_AC1] = 1;
  	      thr_data.a[i_AC1] = vert_AC1;
  	      thr_data.c[num_candC] = -1;
  	    }
      }/* if (a[i_AC1] != -2) */
      // if (i_AC1 == (num_candA - 1)) i_AC1 = -1;
      // parada_1--;
      // if (moveu == true) break;
    }/* for (vert_AC1 = -1, parada_1=num_candA, i_AC1=(int)(rand() % num_candA); parada_1 > 0; i_AC1++) */
  }
  if (TEST) testa_particoes_grasp_sig_v2(A, a, B, b, C, c);
  if (moveu) {
    /* maximiza solucao */
    maximiza_solucao_grasp_sig_v2(A, a, B, b, C, c, cand1, cand2, TEST);
    return 1;
  } else
    return 0;
}



/* vizinhanca do GRASP para o problema do grafo de sinais -> desce 2 de B e sobem 3 para A ou B */
int Sigraph::viz_2down_B_v2(Array<int> &A, Array<int> &a, Array<int> &B, Array<int> &b,
    Array<int> &C, Array<int> &c, Array<int> &cand1, Array<int> &cand2, int TEST, timeval t_ini_g,
    int TIMEMAX) {
  timeval end;
  // int vert_BC1, vert_BC2, i_BC1, i_BC2, parada_1, parada_2, num_candB, num_candC;
  int vert_BC1, vert_BC2, i_BC2, parada_2, num_candB, num_candC;
  bool moveu = false;

  int DEPU = 0;

  if (DEPU) std::cout << std::endl << "*************  viz_2down_B_v2  ****************** "
      << std::endl;

  /* ----------------------------- conta numero de candidatos de B e C */
  for (num_candB = 0; b[num_candB] != -1; num_candB++);
  for (num_candC = 0; c[num_candC] != -1; num_candC++);

  if (num_candB <= 1)
    return 0;

  #pragma omp parallel private(vert_BC1, vert_BC2, parada_2, i_BC2)
  {
    bool i_moveu = false;
    int iam = 0;
    int np = 1;

    iam = omp_get_thread_num();
    np = omp_get_num_threads();

    thread_data_t thr_data;
    thr_data.tid = iam;
    thr_data.A = A;
    thr_data.a = a;
    thr_data.B = B;
    thr_data.b = b;
    thr_data.C = C;
    thr_data.c = c;
    thr_data.cand1 = cand1;
    thr_data.cand2 = cand2;
    thr_data.TEST = TEST;
    thr_data.t_ini_g = t_ini_g;
    thr_data.obj = this;

    #pragma omp barrier

    // divisão do trabalho
		int local_init, local_end, chunk;
    chunk = num_candB / np;
		int l_rest = num_candB % np;
    if (l_rest == 0) {
      local_init = iam * chunk;
  		local_end = (iam + 1) * chunk;
    } else {
      if (iam < l_rest) {
        local_init = iam * (chunk + 1);
    		local_end = (iam + 1) * (chunk + 1);
      } else {
        // int pli = (l_rest - 1) * (chunk + 1);  // previous local init
        int ple = (l_rest) * (chunk + 1);  // previous local end
        local_init = ple + (iam - l_rest) * chunk;
    		local_end = ple + (iam - l_rest + 1) * chunk;
      }
    }
    // #pragma omp critical
    // {
    //   std::cout << rank << ":" << iam << ":local_init [" << local_init << "]" << " local_end ["
    //       << local_end << "] " << "num_candA [" << num_candA << "]." << std::endl;
    // }
    /* 1 NIVEL) A -> C */
    // for (vert_AC = -1, parada_1 = num_candA, i_AC = (int) (rand() % num_candA); parada_1 > 0; i_AC++) {

    /* 1 NIVEL) 1A -> C */
    // for (vert_AC1 = -1, parada_1 = num_candA, i_AC1 = (int) (rand() % num_candA);
    //     parada_1 > 0; i_AC1++) {
    // for (int i_AC1 = 0; i_AC1 < num_candA; i_AC1++) {

    /* 1 NIVEL) 1B -> C */
    // for (vert_BC1 = -1, parada_1 = num_candB, i_BC1 = (int) (rand() % num_candB);
    //     parada_1 > 0; i_BC1++) {
    // for (int i_BC1 = 0; i_BC1 < num_candB; i_BC1++) {
    for (int i_BC1 = local_init; i_BC1 < local_end; i_BC1++) {
      if (moveu) break;
      if (b[i_BC1] != -2) {
        vert_BC1 = thr_data.b[i_BC1];

        /* ---> */
        thr_data.B[vert_BC1] = 0;
        thr_data.C[vert_BC1] = 1;
        thr_data.b[i_BC1] = -2;
        thr_data.c[num_candC] = vert_BC1;
        thr_data.c[num_candC + 1] = -1;

        /* 1 NIVEL) 2B -> C */
        for (vert_BC2 = -1, parada_2 = num_candB, i_BC2 = (int) (rand() % num_candB);
            parada_2 > 0; i_BC2++) {
  	      if ((b[i_BC2] != -2) && (b[i_BC2] > vert_BC1)) {
      		  vert_BC2 = thr_data.b[i_BC2];
      		  if (DEPU) std::cout << std::endl << "vert_BB_C = " << vert_BC1 << "," << vert_BC2
                << std::endl;

            /* ---> */
      		  thr_data.B[vert_BC2] = 0;
      		  thr_data.C[vert_BC2] = 1;
      		  thr_data.b[i_BC2] = -2;
      		  thr_data.c[num_candC + 1] = vert_BC2;
      		  thr_data.c[num_candC + 2] = -1;

            i_moveu = move_3_C_AB_v2(vert_BC1, vert_BC2, 1, 1, thr_data.A, thr_data.a, thr_data.B,
                thr_data.b, thr_data.C, thr_data.c, thr_data.cand1, thr_data.cand2, thr_data.TEST);

            #pragma omp critical
            {
        	    if (i_moveu == true) {
                if (!moveu) {
                  m_n_viz_2b++;
                  A = thr_data.A;
                  a = thr_data.a;
                  B = thr_data.B;
                  b = thr_data.b;
                  C = thr_data.C;
                  c = thr_data.c;
                  moveu = true;
                }
              }
            }
            if (moveu) break;
            if (!i_moveu) {
    		      /* <--- */
    		      thr_data.C[vert_BC2] = 0;
    		      thr_data.B[vert_BC2] = 1;
    		      thr_data.b[i_BC2] = vert_BC2;
    		      thr_data.c[num_candC + 1] = -1;

              // if (calcula_tempo(t_ini_g, (unsigned long int) clock()) > TIMEMAX)
              gettimeofday(&end, NULL);
              if (calcula_tempo(t_ini_g, end) > TIMEMAX) {
      		      /* <--- */
      		      thr_data.C[vert_BC1] = 0;
      		      thr_data.B[vert_BC1] = 1;
      		      thr_data.b[i_BC1] = vert_BC1;
      		      thr_data.c[num_candC] = -1;

      		      // return 0;
              }
            }
          } /* if ((b[i_BC2] != -2) && (b[i_BC2] > vert_BC1)) */
  	      if (i_BC2 == (num_candB-1)) i_BC2 = -1;
  	      parada_2--;
          // if (moveu == true) break;
  	    } /* for (vert_BC2 = -1, parada_2=num_candB, i_BC2=(int)(rand() % num_candB); parada_2 > 0; i_BC2++) */
    	  // if (moveu == true)
    	  //   break;
        if (moveu) break;
    	  if (!i_moveu) {
  	      /* <--- */
  	      thr_data.C[vert_BC1] = 0;
  	      thr_data.B[vert_BC1] = 1;
  	      thr_data.b[i_BC1] = vert_BC1;
  	      thr_data.c[num_candC] = -1;
  	    }
      } /* if (b[i_BC1] != -2) */
      // if (i_BC1 == (num_candB-1)) i_BC1 = -1;
      // parada_1--;
      // if (moveu == true) break;
    } /* for (vert_BC1 = -1, parada_1=num_candB, i_BC1=(int)(rand() % num_candB); parada_1 > 0; i_BC1++) */
  }
  if (TEST) testa_particoes_grasp_sig_v2(A, a, B, b, C, c);
  if (moveu) {
    /* maximiza solucao */
    maximiza_solucao_grasp_sig_v2(A, a, B, b, C, c, cand1, cand2, TEST);
    return 1;
  } else
    return 0;
}



/* vizinhanca do GRASP para o problema do grafo de sinais -> desce 2 de AB (1 de A e um de B )e sobem 3 para A ou B */
int Sigraph::viz_2down_AB_v2(Array<int> &A, Array<int> &a, Array<int> &B, Array<int> &b,
    Array<int> &C, Array<int> &c, Array<int> &cand1, Array<int> &cand2, int TEST, timeval t_ini_g,
    int TIMEMAX) {
  timeval end;
  // int vert_AC1, vert_BC1, i_AC1, i_BC1, parada_1, parada_2, num_candA, num_candB, num_candC;
  int vert_AC1, vert_BC1, i_BC1, parada_2, num_candA, num_candB, num_candC;
  bool moveu = false;

  int DEPU = 0;

  if (DEPU) std::cout<<std::endl<<"*************  viz_2down_AB_v2  ****************** "<<std::endl;

  /* ----------------------------- conta numero de candidatos de A, B e C */
  for (num_candA = 0; a[num_candA] != -1; num_candA++);
  for (num_candB = 0; b[num_candB] != -1; num_candB++);
  for (num_candC = 0; c[num_candC] != -1; num_candC++);

  if ((num_candB == 0) || (num_candA == 0))
    return 0;

  #pragma omp parallel private(vert_AC1, vert_BC1, parada_2, i_BC1)
  {
    bool i_moveu = false;
    int iam = 0;
    int np = 1;

    iam = omp_get_thread_num();
    np = omp_get_num_threads();

    thread_data_t thr_data;
    thr_data.tid = iam;
    thr_data.A = A;
    thr_data.a = a;
    thr_data.B = B;
    thr_data.b = b;
    thr_data.C = C;
    thr_data.c = c;
    thr_data.cand1 = cand1;
    thr_data.cand2 = cand2;
    thr_data.TEST = TEST;
    thr_data.t_ini_g = t_ini_g;
    thr_data.obj = this;

    #pragma omp barrier

    // divisão do trabalho
		int local_init, local_end, chunk;
    chunk = num_candA / np;
		int l_rest = num_candA % np;
    if (l_rest == 0) {
      local_init = iam * chunk;
  		local_end = (iam + 1) * chunk;
    } else {
      if (iam < l_rest) {
        local_init = iam * (chunk + 1);
    		local_end = (iam + 1) * (chunk + 1);
      } else {
        // int pli = (l_rest - 1) * (chunk + 1);  // previous local init
        int ple = (l_rest) * (chunk + 1);  // previous local end
        local_init = ple + (iam - l_rest) * chunk;
    		local_end = ple + (iam - l_rest + 1) * chunk;
      }
    }
    // #pragma omp critical
    // {
    //   std::cout << rank << ":" << iam << ":local_init [" << local_init << "]" << " local_end ["
    //       << local_end << "] " << "num_candA [" << num_candA << "]." << std::endl;
    // }
    /* 1 NIVEL) A -> C */
    // for (vert_AC = -1, parada_1 = num_candA, i_AC = (int) (rand() % num_candA); parada_1 > 0; i_AC++) {

    /* 1 NIVEL) 1A -> C */
    // for (vert_AC1 = -1, parada_1 = num_candA, i_AC1 = (int) (rand() % num_candA);
    //     parada_1 > 0; i_AC1++) {
    // for (int i_AC1 = 0; i_AC1 < num_candA; i_AC1++) {

    /* 1 NIVEL) 1A -> C */
    // for (vert_AC1 = -1, parada_1 = num_candA, i_AC1 = (int) (rand() % num_candA);
    //     parada_1 > 0; i_AC1++) {
    // for (int i_AC1 = 0; i_AC1 < num_candA; i_AC1++) {
    for (int i_AC1 = local_init; i_AC1 < local_end; i_AC1++) {
      if (moveu) break;
      if (a[i_AC1] != -2) {
        vert_AC1 = thr_data.a[i_AC1];

    	  /* ---> */
    	  thr_data.A[vert_AC1] = 0;
    	  thr_data.C[vert_AC1] = 1;
    	  thr_data.a[i_AC1] = -2;
    	  thr_data.c[num_candC] = vert_AC1;
    	  thr_data.c[num_candC + 1] = -1;

        /* 1 NIVEL) 2B -> C */
        for (vert_BC1 = -1, parada_2 = num_candB, i_BC1 = (int) (rand() % num_candB);
            parada_2 > 0; i_BC1++) {
  	      if (b[i_BC1] != -2) {
            vert_BC1 = thr_data.b[i_BC1];
            if (DEPU) std::cout << std::endl << "vert_AB_C = " << vert_AC1 << "," << vert_BC1
                << std::endl;

            /* ---> */
      		  thr_data.B[vert_BC1] = 0;
      		  thr_data.C[vert_BC1] = 1;
      		  thr_data.b[i_BC1] = -2;
      		  thr_data.c[num_candC + 1] = vert_BC1;
      		  thr_data.c[num_candC + 2] = -1;

            i_moveu = move_3_C_AB_v2(vert_AC1, vert_BC1, 0, 1, thr_data.A, thr_data.a, thr_data.B,
                thr_data.b, thr_data.C, thr_data.c, thr_data.cand1, thr_data.cand2, thr_data.TEST);

            #pragma omp critical
            {
        	    if (i_moveu == true) {
                if (!moveu) {
                  m_n_viz_ab++;
                  A = thr_data.A;
                  a = thr_data.a;
                  B = thr_data.B;
                  b = thr_data.b;
                  C = thr_data.C;
                  c = thr_data.c;
                  moveu = true;
                }
              }
            }
            if (moveu) break;
            if (!i_moveu) {
    		      /* <--- */
    		      thr_data.C[vert_BC1] = 0;
    		      thr_data.B[vert_BC1] = 1;
    		      thr_data.b[i_BC1] = vert_BC1;
    		      thr_data.c[num_candC + 1] = -1;

              // if (calcula_tempo(t_ini_g, (unsigned long int) clock()) > TIMEMAX)
              gettimeofday(&end, NULL);
              if (calcula_tempo(t_ini_g, end) > TIMEMAX) {
        	      /* <--- */
        	      thr_data.C[vert_AC1] = 0;
        	      thr_data.A[vert_AC1] = 1;
        	      thr_data.a[i_AC1] = vert_AC1;
        	      thr_data.c[num_candC] = -1;

        	      // return 0;
        	    }
            }
          }/* if (b[i_BC1] != -2) */
  	      if (i_BC1 == (num_candB - 1)) i_BC1 = -1;
  	      parada_2--;
          // if (moveu == true) break;
  	    }/* for (vert_BC1 = -1, parada_2=num_candB, i_BC1=(int)(rand() % num_candB); parada_2 > 0; i_BC1++) */

        // if (moveu == true)
        //   break;
        if (moveu) break;
        if (!i_moveu) {
  	      /* <--- */
  	      thr_data.C[vert_AC1] = 0;
  	      thr_data.A[vert_AC1] = 1;
  	      thr_data.a[i_AC1] = vert_AC1;
  	      thr_data.c[num_candC] = -1;
  	    }
      }/* if (a[i_AC1] != -2) */
      // if (i_AC1 == (num_candA - 1)) i_AC1 = -1;
      // parada_1--;
      // if (moveu == true) break;
    }/* for (vert_AC1 = -1, parada_1=num_candA, i_AC1=(int)(rand() % num_candA); parada_1 > 0; i_AC1++) */
  }
  if (TEST) testa_particoes_grasp_sig_v2(A, a, B, b, C, c);
  if (moveu) {
    /* maximiza solucao */
    maximiza_solucao_grasp_sig_v2(A, a, B, b, C, c, cand1, cand2, TEST);
    return 1;
  } else
    return 0;
}



/* funcao objetivo do GRASP para o problema do grafo de sinais */
int Sigraph::obj_grasp_sig_v2(Array<int> &A, Array<int> &B) {
  int i, obj = 0;

  for (i = 0; i < n; i++)
    if (A[i] == 1)
      obj++;

  for (i = 0; i < n; i++)
    if (B[i] == 1)
      obj++;

  return obj;
}



void send_requests() {
  for (int i = 0; i < size; i++) {
    // if (!terminou[i] && !enviei[i]) {
    if (m_neighbor_status[i] == 0) {
      int inner_msg = MSG_DEFAULT;
      // std::cout << rank << ":enviando REQUEST para o processo [" << i << "]." << std::endl;
      MPI_Send(&inner_msg, 1, MPI_INT, i, TAG_REQUEST, MPI_COMM_WORLD);
      m_neighbor_status[i] = 1;
    }
  }
}



void receive_request(int origin) {
  int carga = (m_itmax - m_it) / (size);
  if (carga == 0) {
    if ((m_itmax - m_it) > 1) {
      carga = 1;
    } else {
      m_sended_zero++;
    }
  }
  m_itmax -= carga;
  MPI_Send(&carga, 1, MPI_INT, origin, TAG_WORKLOAD, MPI_COMM_WORLD);
  // std::cout << rank << ":enviando WORKLOAD [" << carga << "] para o processo [" << origin << "]." << std::endl;
}



bool receive_workload(int msg, int origin) {
  bool has_workload = false;
  if (msg > 0) {
    m_itmax += msg;
    has_workload = true;
    m_neighbor_status[origin] = 0;
  } else {
    m_neighbor_status[origin] = 2;
    m_finished++;
  }
  // std::cout << rank << ":recebendo carga [" << msg << "] do processo [" << origin << "]."
  //     << std::endl;
  return has_workload;
}



bool process_message(int t, int m, int o) {
  bool rc = false;
  // std::cout << rank << ":TAG [" << t << "] MSG [" << m << "] PROC [" << o << "]." << std::endl;
  if (t == TAG_REQUEST) {
    receive_request(o);
  } else if (t == TAG_WORKLOAD) {
    rc = receive_workload(m, o);
  } else {
    std::cout << rank << "!!! ERROR --> UNDEFINED TAG !!!" << std::endl;
  }
  return rc;
}



void handle_messages_finalization() {
  MPI_Status status;
  while (m_sended_zero < size - 1) {
    if (m_flag) {
      MPI_Irecv(&m_msg, 1, MPI_INT, MPI_ANY_SOURCE, TAG_REQUEST, MPI_COMM_WORLD, &m_request);
      m_flag = 0;
    }
    MPI_Test(&m_request, &m_flag, &status);
    if (m_flag) {
      int origin = status.MPI_SOURCE;
      int tag = status.MPI_TAG;
      process_message(tag, m_msg, origin);
      m_flag = -1;
    } else {
      MPI_Wait(&m_request, &status);
      int origin = status.MPI_SOURCE;
      int tag = status.MPI_TAG;
      process_message(tag, m_msg, origin);;
      m_flag = -1;
    }
  } // while ((!has_workload) && (m_finished < size - 1)) {
} // void handle_messages_iteration() {



void handle_messages_iteration() {
  bool has_workload = false;
  int msg;
  MPI_Status status;
  while ((!has_workload) && (m_finished < size - 1)) {
    MPI_Recv(&msg, 1, MPI_INT, MPI_ANY_SOURCE, TAG_WORKLOAD, MPI_COMM_WORLD, &status);
    int origin = status.MPI_SOURCE;
    int tag = status.MPI_TAG;
    has_workload = process_message(tag, msg, origin);
  } // while ((!has_workload) && (m_finished < size - 1)) {
} // void handle_messages_iteration() {



void handle_messages_calculo() {
  bool checked_msgs = false;
  MPI_Status status;
  while ((!checked_msgs)) {
    if (m_flag) {
      MPI_Irecv(&m_msg, 1, MPI_INT, MPI_ANY_SOURCE, TAG_REQUEST, MPI_COMM_WORLD, &m_request);
      m_flag = 0;
    }
    MPI_Test(&m_request, &m_flag, &status);
    if (m_flag) {
      int origin = status.MPI_SOURCE;
      int tag = status.MPI_TAG;
      process_message(tag, m_msg, origin);
      m_flag = -1;
    } else {
      checked_msgs = true;
    }
  } // while ((!checked_msgs) && (m_finished < size - 1)) {
} // void handle_messages_calculo() {



/* calculo do tempo */
double Sigraph::calcula_tempo(const timeval start, const timeval end) {
  #pragma omp critical
  {
    handle_messages_calculo();
  }
  return (((end.tv_sec  - start.tv_sec) * 1000000u + end.tv_usec - start.tv_usec) / 1.e6);
}



/* calculo do tempo
double Sigraph::calcula_tempo(const unsigned long int ini,
    const unsigned long int fim) {
  double r;
  // std::cout << "CLOCKS_PER_SEC = " << CLOCKS_PER_SEC << std::endl;
  if (fim >= ini)
    r = ((double)(fim - ini)) / CLOCKS_PER_SEC;
  else {
    std::cout << "Não faz sentido!" << std::endl;

    r = ((double)( (fim + (unsigned long int)-1) - ini)) / CLOCKS_PER_SEC;
  }
  return r;
} */

/* GRASP para o problema do grafo de sinais */
/* O metodo trabalha com 3 conjuntos de vertices */
/* A -> primeira componente do subgrafo */
/* B -> segunda componente do subgrafo */
/* C -> vertices que nao estao no subgrafo */
/* O metodo constroi iterativamente um subgrafo maximal induzido pelas componentes A e B que seja balanceado */

int Sigraph::grasp_sig_v2(Array<int> &A, Array<int> &a, Array<int> &B, Array<int> &b, Array<int> &C,
    Array<int> &c, Array<int> &cand1, Array<int> &cand2, Array<int> &b_A, Array<int> &b_B,
    int ITMAX, int TIMEMAX, int TEST) {
  m_itmax = ITMAX;
  int np = 2;
  omp_set_num_threads(np);
  // concurentThreadsSupported = (int) std::thread::hardware_concurrency();
  // concurentThreadsSupported = omp_get_num_threads();
  // std::cout << "\x1b[1;35m" << "concurentThreadsSupported = " << np << "\x1b[0m" << std::endl;

  timeval t_ini_g, end;
  // int it, sol, b_sol = -1, vez, ii;
  int sol, b_sol = -1, vez, ii;
  bool moveu, viz_A, viz_B, viz_AB;
  //unsigned long int t_ini_g;

  /* PARAMETROS */
  int DEPU = 0;

  for (int i = 0; i < size; ++i) {
    m_neighbor_status[i] = 0;
  }
  m_neighbor_status[rank] = 2;

  // t_ini_g  = (unsigned long int) clock();
  gettimeofday(&t_ini_g, NULL); //marcador de início do processamento

    /*int iam = 0, np = 1;
    #pragma omp parallel default(shared) private(iam, np)
    {/ *int iam = 0, np = 1;
      np = omp_get_num_threads();
      iam = omp_get_thread_num();
      printf("Hello from thread %d out of %d from process %d out of Foo on Bar\n",
             iam, np, rank);
    }*/

  // for (it = 0; it < ITMAX + carga; it++) {
  for (m_it = 0; m_it < m_itmax; m_it++) {
    sol = metodo_construtivo_grasp_sig_v2(A, a, B, b, C, c, cand1, cand2, TEST);
    if (DEPU) std::cout << std::endl << "IT = " << m_it << ") solucao inicial com " << sol
        << " vertices" << std::endl;

    moveu = true;
    while (moveu) {
      moveu = false;

      // ---------> 1-swap
  	  viz_A = false;
  	  viz_B = false;
  	  vez = (int)(rand() % 2);
  	  while ((!viz_A) || (!viz_B)) {
	      if ((vez == 1) && (!viz_A)) {
          if (viz_down_A_v2(A, a, B, b, C, c, cand1, cand2, TEST, t_ini_g) == 1) {
            if (DEPU) std::cout << "MOVEU -> viz_down_A_v2" << std::endl;
            moveu = true;
            break;
          }
          vez = 0;
          viz_A = true;
        }

	      /* teste de parada por tempo */
	      // if (calcula_tempo(t_ini_g, (unsigned long int) clock()) > TIMEMAX)
        gettimeofday(&end, NULL);
        if (calcula_tempo(t_ini_g, end) > TIMEMAX)
          break;

	      if ((vez == 0)&& (!viz_B)) {
          if (viz_down_B_v2(A, a, B, b, C, c, cand1, cand2, TEST, t_ini_g) == 1) {
            if (DEPU) std::cout << "MOVEU -> viz_down_B_v2" << std::endl;
            moveu = true;
            break;
	        }
          vez = 1;
          viz_B = true;
        }

	      /* teste de parada por tempo */
	      // if (calcula_tempo(t_ini_g, (unsigned long int) clock()) > TIMEMAX)
        gettimeofday(&end, NULL);
        if (calcula_tempo(t_ini_g, end) > TIMEMAX)
		      break;
	    }

  	  /* teste de parada por tempo */
  	  // if (calcula_tempo(t_ini_g, (unsigned long int) clock()) > TIMEMAX)
      gettimeofday(&end, NULL);
      if (calcula_tempo(t_ini_g, end) > TIMEMAX)
        break;

      // ---------> 2-swap
      if (!moveu) {
	      viz_A = false;
	      viz_B = false;
	      viz_AB = false;
	      vez = (int) (rand() % 3);

	      while ((!viz_A) || (!viz_B) || (!viz_AB)) {
          if ((vez == 0) && (!viz_A)) {
            if (viz_2down_A_v2(A, a, B, b, C, c, cand1, cand2, TEST, t_ini_g, TIMEMAX) == 1) {
              if (DEPU) std::cout << "MOVEU -> viz_2down_A_v2" << std::endl;
              moveu = true;
              break;
            }
  		      vez   = 1;
  		      viz_A = true;
          }
          /* teste de parada por tempo */
          // if (calcula_tempo(t_ini_g, (unsigned long int) clock()) > TIMEMAX)
          gettimeofday(&end, NULL);
          if (calcula_tempo(t_ini_g, end) > TIMEMAX)
            break;

          if ((vez == 1) && (!viz_B)) {
            if (viz_2down_B_v2(A, a, B, b, C, c, cand1, cand2, TEST, t_ini_g, TIMEMAX) == 1) {
              if (DEPU) std::cout << "MOVEU -> viz_2down_B_v2" << std::endl;
              moveu = true;
              break;
            }
            vez = 2;
            viz_B = true;
          }

    		  /* teste de parada por tempo */
    		  // if (calcula_tempo(t_ini_g, (unsigned long int) clock()) > TIMEMAX)
          gettimeofday(&end, NULL);
          if (calcula_tempo(t_ini_g, end) > TIMEMAX)
  		      break;

          if ((vez == 2) && (!viz_AB)) {
            if (viz_2down_AB_v2(A, a, B, b, C, c, cand1, cand2, TEST, t_ini_g, TIMEMAX) == 1) {
              if (DEPU) std::cout << "MOVEU -> viz_2down_AB_v2" << std::endl;
              moveu = true;
              break;
            }
            vez = 0;
            viz_AB = true;
          }

          /* teste de parada por tempo */
          // if (calcula_tempo(t_ini_g, (unsigned long int) clock()) > TIMEMAX)
          gettimeofday(&end, NULL);
          if (calcula_tempo(t_ini_g, end) > TIMEMAX)
            break;
        }/* while ((!viz_A) || (!viz_B) || (!viz_AB))*/
	    }/* if (!moveu) */

  	  if (moveu) {
	      sol = obj_grasp_sig_v2(A, B);
	      if (TEST) testa_particoes_grasp_sig_v2(A, a, B, b, C, c);
	    }

      /* teste de parada por tempo */
      if (DEPU) {
        gettimeofday(&end, NULL);
        //if (calcula_tempo(t_ini_g, end) > TIMEMAX)
        std::cout << std::endl << "TEMPO = " << calcula_tempo(t_ini_g, end) << " seg" << std::endl;
      } // if (calcula_tempo(t_ini_g, (unsigned long int) clock()) > TIMEMAX)
      gettimeofday(&end, NULL);
      if (calcula_tempo(t_ini_g, end) > TIMEMAX)
        break;
    }

    if (moveu) {
      sol = obj_grasp_sig_v2(A, B);
      if (TEST) testa_particoes_grasp_sig_v2(A, a, B, b, C, c);
    }

    if (sol > b_sol) {
      b_sol = sol;
      double l_delta;
      b_sol = sol;

      gettimeofday(&end, NULL);
      l_delta = calcula_tempo(t_ini_g, end);
      if (b_sol >= m_target && l_delta < m_ttt) {
        m_ttt = l_delta;
      }

      if (DEPU) std::cout << std::endl << "IT = " << m_it
          << ") ---------------------------------- MELHOR SOLUCAO COM " << sol << " VERTICES"
          << std::endl;

      for (ii = 0; ii < n; ii++) {
	      b_A[ii] = A[ii];
	      b_B[ii] = B[ii];
	    }
    }

    /* teste de parada por tempo */
    if (DEPU) {
      gettimeofday(&end, NULL);
      std::cout<<std::endl << "TEMPO = " << calcula_tempo(t_ini_g, end) << " seg" << std::endl;
    }

    // gettimeofday(&end, NULL);
    // std::cout << "\x1b[1;31m" << rank << ":" << m_it << ":TEMPO = " << calcula_tempo(t_ini_g, end)
    //     << " seg, melhor solucao " << b_sol << ", solucao atual " << sol << "\x1b[0m" << std::endl;

    gettimeofday(&end, NULL);
    if (calcula_tempo(t_ini_g, end) > TIMEMAX)
      break;

    if (m_it == m_itmax - 1) {
      send_requests();
      handle_messages_iteration();
    }
  } /* for (it=0; it<ITMAX; it++) */
  handle_messages_finalization();
  if (rank != 0) {
    // std::cout << rank << ":enviando melhor solução [" << b_sol << "] para o processo 0" << std::endl;
    MPI_Send(&b_sol, 1, MPI_INT, 0, TAG_FINISHED, MPI_COMM_WORLD);
    MPI_Send(&m_ttt, 1, MPI_DOUBLE, 0, TAG_FINISHED, MPI_COMM_WORLD);
    MPI_Send(&m_n_viz_1a, 1, MPI_INT, 0, TAG_FINISHED, MPI_COMM_WORLD);
    MPI_Send(&m_n_viz_1b, 1, MPI_INT, 0, TAG_FINISHED, MPI_COMM_WORLD);
    MPI_Send(&m_n_viz_2a, 1, MPI_INT, 0, TAG_FINISHED, MPI_COMM_WORLD);
    MPI_Send(&m_n_viz_2b, 1, MPI_INT, 0, TAG_FINISHED, MPI_COMM_WORLD);
    MPI_Send(&m_n_viz_ab, 1, MPI_INT, 0, TAG_FINISHED, MPI_COMM_WORLD);
  } else {
    // std::cout << "\x1b[1;38m"  << rank << ":resposta [" << b_sol << "] do processo [" << rank
    //     << "]." << "\x1b[0m"  << std::endl;
  }
  return b_sol;
}



// gera um arquivo .sol de solucao a partir do grafo
void Sigraph::write_sol(const char *name, Array<int> &A, Array<int> &B) {
  int i;
  std::ofstream arq_out;
  char name_G[256];

  /* abre arquivo de saida */
  strcpy (name_G, name);
  strcat (name_G, ".sol");
  arq_out.open(name_G);

  /* escreve vertices de A */
  for (i = 0; i < n; i++)
    if (A[i] == 1)
      arq_out << i << std::endl;

  /* escreve vertices de A */
  for (i = 0; i < n; i++)
    if (B[i] == 1)
      arq_out << (-i) << std::endl;

  /* fecha arquivo de saida */
  arq_out.close();
}
