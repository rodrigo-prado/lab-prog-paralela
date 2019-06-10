#include <stdio.h>
#include <math.h>
#include <omp.h>
// #include <mpi.h>

float calcula(float local_a, float local_b, int local_n, float h);

float f(float x); // função a integrar

int main(int argc, char** argv) {
  int my_rank;
  int t; // número de threads
  float a = 0.0, b = 1.0; // intervalo a calcular
  int n = 64 * 1024; // número de trapezóides
  float h; // base do trapezóide
  float integral; // integral no meu intervalo

  h = (b - a) / n;

  integral = calcula(a, b, n, h);

#ifdef _OPENMP
  t = 12;
  omp_set_num_threads(t);
#else
  t = 1;
#endif

  printf("a = %f, b = %f\n", a, b);
  printf("Numero de trapezoides: %d\n", n);
  printf("Numero de threads: %d\n", t);
  printf("Resultado: %f\n", integral);
}

float calcula(float local_a, float local_b, int local_n, float h) {
  float integral;
  float x;

  integral = (f(local_a) + f(local_b)) / 2.0;
  // x = local_a;

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
