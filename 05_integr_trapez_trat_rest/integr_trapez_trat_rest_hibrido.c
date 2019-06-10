#include <stdio.h>
#include <math.h>
#include <mpi.h>
#include <omp.h>

float calcula(float local_a, float local_b, int local_n, float h);

float f(float x); // função a integrar

int main(int argc, char** argv) {
  int my_rank;
  int p; // número de processos
  int t; // número de threads
  float a = 0.0, b = 1.0; // intervalo a calcular
  int n = 64 * 1024; // número de trapezóides
  float h; // base do trapezóide
  float local_a, local_b; // intervalo local
  int local_n; // número de trapezóides local
  float integral; // integral no meu intervalo
  float total; // integral total
  int source; // remetente da integral
  int dest = 0; // destino das integrais (nó 0)
  int tag = 200; // tipo de mensagem (único)

  MPI_Status status;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &p);

#ifdef _OPENMP
  t = 4;
  omp_set_num_threads(t);
#else
  t = 1;
#endif

  h = (b - a) / n;
  local_n = n / p;
  local_a = a + (my_rank * local_n * h);
  if (my_rank == p - 1) {
    local_b = local_a + ((local_n + (n % p)) * h);
    integral = calcula(local_a, local_b, local_n + (n % p), h);
    printf("rank: %d, local_n: %d, local_a: %f, local_b: %f\n", my_rank, local_n + (n % p), local_a,
        local_b);
  } else {
    local_b = local_a + (local_n * h);
    integral = calcula(local_a, local_b, local_n, h);
    printf("rank: %d, local_n: %d, local_a: %f, local_b: %f\n", my_rank, local_n, local_a, local_b);
  }

  if (my_rank == 0) {
    printf("a = %f, b = %f\n", a, b);
    printf("Numero de trapezoides: %d\n", n);
    printf("Numero de threads por processo: %d\n", t);
    printf("Numero de processos: %d\n", p);
    fflush(stdout);

    total = integral;
    for (source = 1; source < p; source++) {
      MPI_Recv(&integral, 1, MPI_FLOAT, source, tag, MPI_COMM_WORLD, &status);
      total += integral;
    }
  } else
    MPI_Send(&integral, 1, MPI_FLOAT, dest, tag, MPI_COMM_WORLD);

  if (my_rank == 0) printf("Resultado: %f\n", total);

  MPI_Finalize();
}

float calcula(float local_a, float local_b, int local_n, float h) {
  float integral;
  float x;

  integral = (f(local_a) + f(local_b)) / 2.0;
  x = local_a;

  #pragma omp parallel for reduction(+:integral) private(x)
  for (int i = 1; i < local_n; i++) {
    x = local_a + (h * i);
    integral += f(x);
  }
  integral *= h;
  return integral;
}


float f(float x) {
  float fx; // valor de retorno
  // esta é a função a integrar
  // exemplo: função quadrática
  fx = sin(x * x) * cos(0.1 * x);
  return fx;
}
