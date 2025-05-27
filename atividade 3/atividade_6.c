#include <math.h>
#include <stdio.h>
#include <stdlib.h> 
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

int N;
int M;
int C;
int P = 1;

sem_t slotCheio, slotVazio;
sem_t mutexGeral;

long long int* buffer;
int count = 0;
int out = 0;
int in = 0;

typedef struct t_cons_args TCA;
struct t_cons_args 
{
    int id;
    int count;
};

void printBuffer(long long int buffer[], int tam) {
    for(int i=0;i<tam;i++) 
        printf("%lld ", buffer[i]); 
    printf("\n");
}

int ehPrimo (long long int n) 
{
    int i;

    if (n<=1) return 0;
    if (n==2) return 1;
    if (n % 2==0) return 0;
    for (i = 3; i < (sqrt(n) + 1); i += 2) if ((n % i) == 0) return 0;

    return 1;
}

void insere (long int id, long long int item) 
{
    sem_wait(&slotVazio);
    sem_wait(&mutexGeral);
    buffer[in] = item;
    in = (in + 1) % M;
    count++;
    printf("Produtor [%ld]: inseriu %lld \n", id, item);
    printBuffer(buffer, M);
    sem_post(&mutexGeral);
    sem_post(&slotCheio);
}
int retira (int id) 
{
    long long int item;

    sem_wait(&slotCheio);
    sem_wait(&mutexGeral); 
    item = buffer[out];
    buffer[out] = 0;
    out = (out + 1) % M;
    count--;
    if (item == -1) 
    {
        sem_post(&mutexGeral);
        sem_post(&slotVazio);
        return item;
    }
    printf("Consumidor [%d]: retirou %lld\n", id, item);
    printBuffer(buffer, M);
    sem_post(&mutexGeral);
    sem_post(&slotVazio);
    
    return item;
}

void* produtor (void* arg) 
{    
    long int id = (long int) arg;
    for (long long int i = 1; i < N; i++) 
    {
        insere(id, i);
    }

    for (int i = 0; i < C; i++) {
        insere(id, -1);
    }

    pthread_exit(NULL);
}
void* consumidor (void* arg) 
{
    TCA* args = (TCA*) arg;
    long long int item;

    while (1)
    {
        item = retira(args->id);

        if (item == -1)
        {
            printf("Consumidor [%d]: recebeu sinal de parada (-1).\n", args->id);
            break;
        }

        if (ehPrimo(item))
        {
            args->count++;
            printf("%lld eh primo.\n", item);
        };
    }

    pthread_exit(NULL);
}

int main (int argc, char* argv[]) 
{
    if (argc < 4) 
    {
        printf("ERROR: %s <numero_consumidoras> <numero_buffer> <numero_verificados>\n", argv[0]);
        return 1;
    }
    C = atoi(argv[1]);
    M = atoi(argv[2]);
    N = atoi(argv[3]);

    buffer = (long long int*) malloc(sizeof(long long int) * M);
    if (buffer == NULL)
    {
        printf("ERROR: Erro ao alocar buffer\n");
        return 3;
    }
    TCA* cons_array = (TCA*) malloc(sizeof(TCA) * C);
    if (cons_array == NULL) 
    {
        printf("ERROR: Erro ao alocar cons_array\n");
        free(buffer);
        return 4;
    }
    pthread_t tid[P + C];

    sem_init(&mutexGeral, 0, 1);
    sem_init(&slotCheio, 0, 0);
    sem_init(&slotVazio, 0, M);
    for (long int t = 0; t < P; t++) 
    {
        if (pthread_create(&tid[t], NULL, produtor, (void*) t)) 
        {
            printf("ERROR: pthread_create() do produtor\n");
            return 5;
        }
    }
    for (int t = P; t < P + C; t++) 
    {
        cons_array[t].id = t;
        cons_array[t].count = 0; 
        if (pthread_create(&tid[P + t], NULL, consumidor, (void*) &cons_array[t])) 
        {
            printf("ERROR: pthread_create() do consumidor\n");
            return 6;
        }
    }
    for (int t = 0; t < P + C; t++) 
    {
        pthread_join(tid[t], NULL);
    }
    sem_destroy(&mutexGeral);
    sem_destroy(&slotCheio);
    sem_destroy(&slotVazio);

    int id;
    for (int i = P; i < P + C; i++) 
    {
        if (cons_array[i].count > cons_array[id].count) 
        {
            id = i;
        }
    }
    int primo_count = 0;
    for (int i = P; i < P + C; i++) 
    {
        primo_count += cons_array[i].count;
    }
    printf("\n%d primos verificados \n", primo_count);
    printf("Consumidora %d venceu com %d numeros primos verificados\n", id, cons_array[id].count);

    free(buffer);
    free(cons_array);

    return 0;
}

