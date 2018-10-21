#include <semaphore.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <math.h>

/* --- Procedures Prototypes --- */
void integral(int *my_rank);
float calcula(float local_a, float local_b, int local_n, float h);
float f(float x); // função a integrar
/* --- Procedures Prototypes --- */

float total = 0.0;   // integral total
sem_t total_mutex;   /* acesso a variavel total */

main(int argc, char** argv) {

  pthread_t thr0, thr1, thr2, thr3, thr4, thr5, thr6, thr7, thr8, thr9, thr10, thr11;
  int nThrID0;
  int nThrID1, nThrID2, nThrID3, nThrID4, nThrID5, nThrID6;
  int nThrID7, nThrID8, nThrID9, nThrID10, nThrID11;

  nThrID0 = 0;
  nThrID1 = 1; nThrID2 = 2; nThrID3 = 3; nThrID4 = 4;
  nThrID5 = 5; nThrID6 = 6; nThrID7 = 7; nThrID8 = 8;
  nThrID9 = 9; nThrID10 = 10; nThrID11 = 11;

  sem_init(&total_mutex, 0, 1);

  pthread_create(&thr0, NULL, (void*) integral, &nThrID0);
  pthread_create(&thr1, NULL, (void*) integral, &nThrID1);
  pthread_create(&thr2, NULL, (void*) integral, &nThrID2);
  pthread_create(&thr3, NULL, (void*) integral, &nThrID3);
  pthread_create(&thr4, NULL, (void*) integral, &nThrID4);
  pthread_create(&thr5, NULL, (void*) integral, &nThrID5);
  pthread_create(&thr6, NULL, (void*) integral, &nThrID6);
  pthread_create(&thr7, NULL, (void*) integral, &nThrID7);
  pthread_create(&thr8, NULL, (void*) integral, &nThrID8);
  pthread_create(&thr9, NULL, (void*) integral, &nThrID9);
  pthread_create(&thr10, NULL, (void*) integral, &nThrID10);
  pthread_create(&thr11, NULL, (void*) integral, &nThrID11);

  pthread_join(thr0, NULL);
  pthread_join(thr1, NULL);
  pthread_join(thr2, NULL);
  pthread_join(thr3, NULL);
  pthread_join(thr4, NULL);
  pthread_join(thr5, NULL);
  pthread_join(thr6, NULL);
  pthread_join(thr7, NULL);
  pthread_join(thr8, NULL);
  pthread_join(thr9, NULL);
  pthread_join(thr10, NULL);
  pthread_join(thr11, NULL);

  sem_close(&total_mutex);

  printf("a=%f, b=%f\n", 0.0, 1.0);
  printf("Numero de trapezoides: %d\n", 64 * 1024);
  printf("Numero de threads: %d\n", 12);
  printf("Resultado: %f\n", total);
}

void integral(int *my_rank) {
  float a = 0.0, b = 1.0; // intervalo a calcular
  int n = 64 * 1024; // número de trapezóides
  float h; // base do trapezóide
  float local_a, local_b; // intervalo local
  int local_n; // número de trapezóides local
  float integral; // integral no meu intervalo
  int p = 12; // número de threads

  h = (b - a) / n;
  local_n = n / p;
  local_a = a + (*my_rank * local_n * h);
  if (*my_rank == p - 1) {
    local_b = local_a + ((local_n + (n % p)) * h);
    integral = calcula(local_a, local_b, local_n + (n % p), h);
    printf("Thread: %d, local_n: %d, local_a: %f, local_b: %f\n", *my_rank, local_n + (n % p), local_a, local_b);
  } else {
    local_b = local_a + (local_n * h);
    integral = calcula(local_a, local_b, local_n, h);
    printf("Thread: %d, local_n: %d, local_a: %f, local_b: %f\n", *my_rank, local_n, local_a, local_b);
  }

  sem_wait(&total_mutex);
  total += integral;
  sem_post(&total_mutex);
}

float calcula(float local_a, float local_b, int local_n, float h) {
  float integral;
  float x, i;
  integral = (f(local_a) + f(local_b)) / 2.0;
  x = local_a;
  for (i = 1; i < local_n; i++) {
    x += h;
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
