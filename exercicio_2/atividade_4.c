#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "timer.h"

// limite superior para busca de primos
long long int N;

// numero de threads
int T;

// contador atual e total de primos
long long int atual = 1;
long long int total_primos = 0;

// mutex para regiao critica
pthread_mutex_t mutex;

// funcao que verifica se um numero é primo
int is_prime (long long int n) 
{
    if (n <= 1) return 0;
    if (n == 2) return 1;
    if (n % 2 == 0) return 0;
    for (long long int i = 3; i <= sqrt(n); i += 2)
        if (n % i == 0)
            return 0;
    return 1;
}

// funcao executada pelas threads
void* prime_counter (void* arg) 
{
    long long int numero;

    while (1) {
        pthread_mutex_lock(&mutex);
        if (atual > N) {
            pthread_mutex_unlock(&mutex);
            break;
        }
        numero = atual;
        atual++;
        pthread_mutex_unlock(&mutex);

        if (is_prime(numero)) {
            pthread_mutex_lock(&mutex);
            total_primos++;
            pthread_mutex_unlock(&mutex);
        }
    }

    pthread_exit(NULL);
}

int main (int argc, char* argv[]) 
{
    //
    if (argc != 3) {
        printf("Entrada correta: %s <num_threads> <limite_superior>\n", argv[0]);
        return 1;
    }

    // convertendo argumentos
    N = atoll(argv[1]);
    T = atoi(argv[2]);

    // criando vetor de threads
    pthread_t* tid = (pthread_t*) malloc(sizeof(pthread_t) * T);
    if (tid == NULL) {
        printf("ERROR: malloc() do tid\n");
        return 2;
    }

    pthread_mutex_init(&mutex, NULL);

    // medindo tempo
    double ini, fim, tempo;
    GET_TIME(ini);

    // criando threads
    for (int i = 0; i < T; i++) {
        if (pthread_create(&tid[i], NULL, prime_counter, NULL)) {
            printf("ERROR: pthread_create()\n");
            return 3;
        }
    }

    // aguardando termino das threads
    for (int i = 0; i < T; i++) {
        pthread_join(tid[i], NULL);
    }

    GET_TIME(fim);
    tempo = fim - ini;

    printf("Total de primos encontrados entre 1 e %lld: %lld\n", N, total_primos);
    printf("Tempo de execução com %d thread(s): %.6f segundos\n", T, tempo);

    // liberando memoria
    pthread_mutex_destroy(&mutex);
    free(tid);

    return 0;
}
