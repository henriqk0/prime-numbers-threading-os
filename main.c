#include <stdio.h>
#include <stdlib.h>
#include <math.h>//permitido?
#include <time.h>
#include <pthread.h>

#define LARGURA 10000
#define ALTURA 10000


int **matriz;
int numPrimos = 0; 


int ehPrimo(int n);
void mallocMatriz(int altura, int largura);
void freeMatriz(int altura);

int main(int argc, char *argv[]) {
    srand(time(NULL));

    mallocMatriz(ALTURA, LARGURA);
    int num_rando = 0 + rand() % (31999 - 0 + 1);
    
    
return 0;
}


int ehPrimo(int n) {
    int raizInteira, i; 
    raizInteira = (int)(round(squareRoot(n)));
    for (i = 2; i <= raizInteira; i++) {
        if (n % i == 0) return 0;

    }
    return 1;
}


void mallocMatriz(int altura, int largura) {
    matriz = (int **)malloc(sizeof( altura * sizeof(int *) ));
    if matriz == NULL {
        printf("ERRO. Não foi possível alocar memória na matriz\n")
        exit(1);   
    }
    
    int i;
    for (i = 0; i < altura; i++) {
        matriz[i] = (int *)malloc(sizeof( largura * sizeof(int *) ));
        if matriz[i] == NULL {
            printf("ERRO. Não foi possível alocar memória na matriz\n")
            exit(1);   
        }
    }
}


void freeMatriz(int altura) {
    int i;
    for (i = 0; i < altura; i++){
        free(matriz[i]);
    }
    free(matriz);
}


