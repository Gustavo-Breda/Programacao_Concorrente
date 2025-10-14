#include <math.h>
#include <stdio.h>
#include <stdlib.h> 
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

int N;
int M;
int C = 5;
int P = 1;

sem_t bufferCheio, bufferVazio;
sem_t mutexGeral;

int* buffer;
int count = 0;
int out = 0;
int in = 0;

typedef struct t_consumer_args TCA;
struct t_consumer_args 
{
    long int id;
    int count;
};

void printBuffer(int buffer[], int tam) {
    for(int i = 0; i < tam;i++) 
        printf("%d ", buffer[i]); 
    printf("\n");
}

int ehPrimo (int n) 
{
    int i;

    if (n<=1) return 0;
    if (n==2) return 1;
    if (n % 2==0) return 0;
    for (i = 3; i < (sqrt(n) + 1); i += 2) if ((n % i) == 0) return 0;

    return 1;
}

// poderia ser mais simples mais dessa maneira eh mais facil caso queira colocar mais produtores
void insere (long int id, int item) 
{
    sem_wait(&bufferVazio);
    sem_wait(&mutexGeral);
    printf("Produtor [%ld]: iseriu %d\n", id, item);
    buffer[in] = item;
    in = (in + 1) % M;
    count++;
    printBuffer(buffer, M);

    if (count == M) {
        sem_post(&mutexGeral);
        sem_post(&bufferCheio);
    }
    else {
        sem_post(&bufferVazio);
        sem_post(&mutexGeral);
    }
}
int retira (long int id) 
{
    int item;

    sem_wait(&bufferCheio);
    sem_wait(&mutexGeral);
    count--;
    item = buffer[out];
    buffer[out] = 0;
    out = (out + 1) % M;
    if (count == 0) 
    {
        if (item == -1){
            printf("Consumidor [%ld]: recebeu sinal de parada (-1)\n", id);
        } else {
            printf("Consumidor [%ld]: retirou %d\n", id, item);
        }
        printBuffer(buffer, M);
        sem_post(&mutexGeral);
        sem_post(&bufferVazio);

        return item;
    } else {
        if (item == -1){
            printf("Consumidor [%ld]: recebeu sinal de parada (-1)\n", id);
        } else {
            printf("Consumidor [%ld]: retirou %d\n", id, item);
        }
        printBuffer(buffer, M);
        sem_post(&mutexGeral);
        sem_post(&bufferCheio);
        
        return item;
    }
}

void* produtor (void* arg) 
{    
    long int id = (long int) arg;
    static int i = 1;

    while (i < N + 1) 
    {
        insere(id, i);
        i += 1;
    }

    i = 0;
    while (i < C) 
    {
        insere(id, -1);
        i += 1;
    }
    sem_post(&bufferCheio);

    pthread_exit(NULL);
}
void* consumidor (void* arg) 
{
    TCA* args = (TCA*) arg;
    int item;

    while (1)
    {
        item = retira(args->id);

        if (item == -1)
        {
            break;
        }

        if (ehPrimo(item))
        {
            args->count++;
            printf("%d eh primo\n", item);
        };
    }

    pthread_exit(NULL);
}

int main (int argc, char* argv[]) 
{
    if (argc < 3) 
    {
        printf("ERROR: %s <numero_verificados> <numero_buffer> <opcional_numero_consumidoras>\n", argv[0]);
        return 1;
    }
    N = atoi(argv[1]);
    M = atoi(argv[2]);
    C = atoi(argv[3]);

    buffer = (int*) malloc(sizeof(int) * M);
    if (buffer == NULL)
    {
        printf("ERROR: Erro ao alocar buffer\n");
        return 3;
    }
    TCA* c_array = (TCA*) malloc(sizeof(TCA) * C);
    if (c_array == NULL) 
    {
        printf("ERROR: Erro ao alocar c_array\n");
        free(buffer);
        return 4;
    }
    pthread_t tid[P + C];

    sem_init(&mutexGeral, 0, 1);
    sem_init(&bufferCheio, 0, 0);
    sem_init(&bufferVazio, 0, 1);
    for (long int t = 0; t < P; t++) 
    {
        if (pthread_create(&tid[t], NULL, produtor, (void*) t)) 
        {
            printf("ERROR: pthread_create() do produtor\n");
            return 5;
        }
    }
    for (long int t = 0; t < C; t++) 
    {
        c_array[t].id = t;
        c_array[t].count = 0; 
        if (pthread_create(&tid[P + t], NULL, consumidor, (void*) &c_array[t])) 
        {
            printf("ERROR: pthread_create() do consumidor\n");
            return 6;
        }
    }
    for (long int t = 0; t < P + C; t++) 
    {
        pthread_join(tid[t], NULL);
    }
    sem_destroy(&mutexGeral);
    sem_destroy(&bufferCheio);
    sem_destroy(&bufferVazio);

    long int id = 0;
    for (long int t = 0; t < C; t++) 
    {
        if (c_array[t].count > c_array[id].count) 
        {
            id = t;
        }
    }
    int primo_count = 0;
    for (long int t = 0; t < C; t++) 
    {
        primo_count += c_array[t].count;
    }
    printf("\n%d primos verificados\n", primo_count);
    printf("Consumidora %ld venceu com %d numeros primos verificados\n", id, c_array[id].count);

    free(buffer);
    free(c_array);

    return 0;
}
