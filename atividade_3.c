#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

// tamanho do vetor
int N;

// numero de thread
int T;

// endereco do file
FILE* file;

// endereco do vetor1
double* vec1;

// endereco do vetor2
double* vec2;

typedef struct t_args T_args;
struct t_args 
{
    int id;
};

double seq_inner_product (double* vec1, double* vec2, int N) {
    double sum = 0;

    for (int i = 0; i < N; i++) {
        sum += vec1[i] * vec2[i];
    }

    return sum;
}

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

    double sum_thread = 0;
    for (int i = ini; i < fim; i++) {
        sum_thread += vec1[i] * vec2[i];
    }
   
    double* ret = (double*) malloc(sizeof(double));
    if (ret != NULL) *ret = sum_thread;
    else printf("ERROR: retorno nulo\n");

    pthread_exit((void*) ret);
}

int main (int argc, char* argv[]) {
    // verificando as entradas
    if (argc != 3) { 
        printf("Entrada correta: %s <num_threads> <num_vetores> \n", argv[0]); 
        return 1; 
    }

    // transformando os dados de str_to_int
    T = atoi(argv[1]);
    N = atoi(argv[2]);

    // limitando o numero de threads 
    if (T > N) T = N;

    // alocando os vetores dinamicamente na memoria
    vec1 = (double*) malloc(sizeof(double) * N);
    if (vec1 == NULL) { 
        printf("ERROR: malloc() do vec1\n"); 
        return 2; 
    }
    vec2 = (double*) malloc(sizeof(double) * N);
    if (vec2 == NULL) { 
        printf("ERROR: malloc() do vec2\n"); 
        return 3; 
    }

    // vetores com valores randomicos entre 0 e 100
    srand(time(NULL));
    for (int i = 0; i < N; i++) {
        vec1[i] = ((double) rand() / RAND_MAX) * 100;
        vec2[i] = ((double) rand() / RAND_MAX) * 100;
    }

    // escrevendo os vetores
    file = fopen("vetores.bin", "wb");
    fwrite(vec1, sizeof(double), N, file);
    fwrite(vec2, sizeof(double), N, file);
    
    // fim da escrita
    fclose(file);

    // lendo os vetores
    file = fopen("vetores.bin", "rb");
    fread(vec1, sizeof(double), N, file);
    fread(vec2, sizeof(double), N, file);
    
    // fim da leitura
    fclose(file);

    // alocando os vetores de identificadores das threads no sistema
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
    double res_coc = 0;
    for (int i = 0; i < T; i++) {
        double* res;
        pthread_join(tid_sistema[i], (void**) &res);
        res_coc += *res;
        free(res);
    }

    // resultado do produto interno sequencial
    double res_seq = seq_inner_product(vec1, vec2, N);

    // imprimando os valores
    printf("Resultado concorrente: %.64f\n", res_coc);
    printf("Resultado sequencial: %.64f\n", res_seq);

    // calculando a variacao relativas
    double variacao_relativa = (res_seq - res_coc) / res_seq;
    printf("error %.100f\n", variacao_relativa);

    // liberando memoria alocada
    free(tid_sistema);
    free(vec1);
    free(vec2);

    return 0;
}