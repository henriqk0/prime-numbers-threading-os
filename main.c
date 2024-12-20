#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <pthread.h>


#define LARGURA 10000
#define ALTURA 10000


int **matriz;
int numPrimos = 0; 


// add -lm if gcc


int ehPrimo(int n);
void mallocMatriz(int altura, int largura);
void freeMatriz(int altura);
void buscaSerial(int altura, int largura);


int main(int argc, char *argv[]) {
    srand(time(NULL));

    mallocMatriz(ALTURA, LARGURA);
    clock_t timer;

    int opcao;
    printf("Digite:\n1 - busca serial\n2 - busca paralela com o numero de threads definido no código\n");

  
    scanf(" %d", &opcao);
    if (opcao <= 2 && opcao > 0) {
        timer = clock();
      
        if (opcao == 1) buscaSerial(ALTURA, LARGURA); 
        else if (opcao == 2) printf("a");
        
        timer = clock() - timer;
        printf("Código executado em  : %.3f segundos\n", ((double)timer) / (CLOCKS_PER_SEC));
        printf("Quantidade de primos na matriz: %d\n", numPrimos);

    }
    else { 
        printf("Opcão inválida.\n");
        return 1;
    }
    freeMatriz(ALTURA);
    
    return 0;
}


int ehPrimo(int n) {
    int raizInteira, i; 
    double raiz;
    
    if (n <= 1) return 0;

    raizInteira = (int)(round(sqrt(n))); // AKS?
    for (i = 2; i <= raizInteira; i++) {
        if (n % i == 0) return 0;
    }
    return 1;
}


void mallocMatriz(int altura, int largura) {
    matriz = (int **)malloc( altura * sizeof(int *) );
    if (matriz == NULL) {
        printf("ERRO. Não foi possível alocar memória na matriz\n");
        exit(1);   
    }
    
    int i, j;
    for (int i = 0; i < altura; i++) {
        matriz[i] = (int *)malloc(largura * sizeof(int)); // invalid write size of 8 (?)
        if (matriz[i] == NULL) {
            printf("ERRO. Não foi possível alocar memória para a linha %d da matriz\n", i);
            exit(1);   
        }
    }

    for (i = 0; i < altura; i++) {
        for (j = 0; j < largura; j++) {
            matriz[i][j] = 0 + rand() % (31999 - 0 + 1);
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


void buscaSerial(int altura, int largura) {
    int i, j; 

    printf("\nIniciando buscaSerial...\n");

    for (i = 0; i < altura; i++) {
        for (j = 0; j < largura; j++) {
            if (ehPrimo(matriz[i][j]) == 1) numPrimos += 1;
        }
    }
}

