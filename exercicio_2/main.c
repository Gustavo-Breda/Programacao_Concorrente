#include <stdio.h>
#include <stdlib.h> 
#include <pthread.h>

int devo_esperar_printar = 1;
long int soma = 0;

pthread_cond_t cond;
pthread_mutex_t mutex;

void* executaTarefa (void *arg) {
    long int id = (long int) arg;

    printf("Thread : %ld esta executando...\n", id);
    for (int i = 0; i < 1000; i++) {
        pthread_mutex_lock(&mutex);
        while (!(soma % 1000) && devo_esperar_printar) {
            pthread_cond_wait(&cond, &mutex);
        }
        soma++;
        if (!(soma % 1000)) devo_esperar_printar = 1;
        else devo_esperar_printar = 0;
        pthread_mutex_unlock(&mutex);
    }
    printf("Thread : %ld terminou!\n", id);

    pthread_exit(NULL);
}

void* extra (void *args) {
    static int value = -1;

    printf ("Extra : esta executando...\n");
    while (1) {
        pthread_mutex_lock(&mutex);
        if (!(soma % 1000)) {
            if (value != soma)
                printf("soma = %ld \n", soma);
            value = soma;
            devo_esperar_printar = 0;
            pthread_cond_broadcast(&cond);
        }
        pthread_mutex_unlock(&mutex);
    }
    printf("Extra : terminou!\n");

    pthread_exit(NULL);
}

int main (int argc, char* argv[]) {
    int nthreads;
    pthread_t* tid;

    if (argc < 2) {
        printf("Digite: %s <numero de threads>\n", argv[0]);
        return 1;
    }
    nthreads = atoi(argv[1]);

    tid = (pthread_t*) malloc(sizeof(pthread_t) * (nthreads+1));
    if (tid == NULL) {
        puts("ERROR: malloc");
        return 2;
    }

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);
    for (long int t = 0; t < nthreads; t++) {
        if (pthread_create(&tid[t], NULL, executaTarefa, (void*) t)) {
            printf("ERROR: pthread_create()\n");
            exit(-1);
        }
    }

    if (pthread_create(&tid[nthreads], NULL, extra, NULL)) {
        printf("ERROR: pthread_create()\n");
        exit(-1);
    }

    for (int t = 0; t < nthreads + 1; t++) {
        if (pthread_join(tid[t], NULL)) {
            printf("ERROR: pthread_join() \n"); 
            exit(-1);
        } 
    } 
    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&mutex);
    
    printf("Soma Final = %ld\n", soma);

    return 0;
}