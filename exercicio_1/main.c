#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "timer.h"


// tamanho do vetor
int N;

// numero de thread
int T;

// endereco do vetor1
float* vec1;

// endereco do vetor2
float* vec2;

typedef struct t_args T_args;
struct t_args 
{
    int id;
};


void* conc_inner_product (void* arg) {
    T_args* args = (T_args*) arg;

    int id = (int) args -> id;

    int bloco;
    int ini;
    int fim;

    bloco = N / T;
    ini = id * bloco;
    fim = ini + bloco;
    if (id == (T - 1)) fim = N;

    float sum_thread = 0;
    for (int i = ini; i < fim; i++) {
        sum_thread += vec1[i] * vec2[i];
    }
   
    float* ret = (float*) malloc(sizeof(float));
    if (ret != NULL) *ret = sum_thread;
    else {exit("ERROR: retorno nulo\n");}

    pthread_exit((void*) ret);
}

int main (int argc, char* argv[]) {
    // nome do binario
    char *file_name;

    // endereco do binario
    FILE* file;

    // resultado do sequencial
    float res;

    // variaveis da macro de tempo
    double ini, fim;

    if (argc != 3) { 
        printf("Entrada correta: %s <num_threads> <nome_arquivo> \n", argv[0]); 
        return 1; 
    }

    // recebendo os dados
    T = atoi(argv[1]);
    file_name = argv[2];

    // iniciado da leitura
    file = fopen(file_name, "rb");

    // lendo os vetores
    fread(&N, sizeof(long int), 1, file);

    // numero maximo de threads eh N
    if (T > N) T = N;

    // alocando os vetores dinamicamente na memoria
    vec1 = (float*) malloc(sizeof(float) * N);
    if (vec1 == NULL) { 
        printf("ERROR: malloc() do vec1\n"); 
        return 2; 
    }
    vec2 = (float*) malloc(sizeof(float) * N);
    if (vec2 == NULL) { 
        printf("ERROR: malloc() do vec2\n"); 
        return 3; 
    }

    fread(vec1, sizeof(float), N, file);
    fread(vec2, sizeof(float), N, file);
    fread(&res, sizeof(float), 1, file);
    
    // fim da leitura
    fclose(file);

    GET_TIME(ini);
    pthread_t* tid_sistema = (pthread_t*) malloc(sizeof(pthread_t) * T);
    if (tid_sistema == NULL) { 
        printf("ERROR: malloc() do tid\n"); 
        return 4; 
    }

    for (int i = 0; i < T; i++) {
        T_args* args = (T_args*) malloc(sizeof(T_args));
        if (args == NULL) {printf("ERROR: malloc() args\n"); 
            return 5;
        }
        args->id = i;

        if (pthread_create(&tid_sistema[i], NULL, conc_inner_product, (void*) args)) {
            printf("ERROR: pthread_create()\n");
            return 6;
        }
    }

    // colentando os resultados parciais das threads
    float res_coc = 0;
    for (int i = 0; i < T; i++) {
        float* res_parcial;
        pthread_join(tid_sistema[i], (void**) &res_parcial);
        res_coc += *res_parcial;
        free(res_parcial);
    }
    GET_TIME(fim);

    // imprimando os valores
    printf("Resultado concorrente: %.7g em %.6fs\n", res_coc, fim - ini);
    printf("Resultado sequencial: %.7g\n", res);

    // calculando a variacao relativas
    float variacao_relativa = (res - res_coc) / res;
    printf("error %.9f\n", variacao_relativa);

    // liberando memoria alocada
    free(tid_sistema);
    free(vec1);
    free(vec2);

    return 0;
}