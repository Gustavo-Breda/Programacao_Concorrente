#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define N 12

float seq_inner_product (float* vec1, float* vec2) {
    float sum = 0;

    for (int i = 0; i < N; i++) {
        sum += vec1[i] * vec2[i];
    }

    return sum;
}

int main (int argc, char* argv[]) {
    FILE* file;

    int n = N;
    float* vec1;
    float* vec2;

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

    // vetores com valores randomicos entre 0 e 100
    srand(time(NULL));
    for (int i = 0; i < N; i++) {
        vec1[i] = ((float) rand() / RAND_MAX) * 100;
        vec2[i] = ((float) rand() / RAND_MAX) * 100;
    }

    // resultado do produto interno sequencial
    float res = seq_inner_product(vec1, vec2);

    // escrevendo os vetores
    file = fopen("vetores.bin", "wb");
    fwrite(&n, sizeof(long int), 1, file);
    fwrite(vec1, sizeof(float), N, file);
    fwrite(vec2, sizeof(float), N, file);
    fwrite(&res, sizeof(float), 1, file);
    
    // fim da escrita
    fclose(file);

    // liberando memoria
    free(vec1);
    free(vec2);

    return 0;
}