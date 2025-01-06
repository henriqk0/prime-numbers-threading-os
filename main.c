#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <pthread.h>


#define LARGURA 10000
#define ALTURA 10000
#define NUMTHREADS 8


int **matriz;
int numPrimos = 0; 
pthread_mutex_t qtdPrimosMutex;
pthread_mutex_t macroblocoMutex;

// add -lm if gcc (math.h)

int ehPrimo(int n);
void mallocMatriz(int altura, int largura);
void freeMatriz(int altura);

void buscaSerial(int altura, int largura);
void* buscaParalela();

int main(int argc, char *argv[]) {
    srand(time(NULL));

    clock_t timer;

    int opcao;
    printf("Digite:\n1 - busca serial\n2 - busca paralela com o numero de threads definido no código\nOutro caractere - sair\n");
    scanf(" %d", &opcao);

    while (opcao <= 2 && opcao > 0) {
        mallocMatriz(ALTURA, LARGURA);
        timer = clock();
      
        if (opcao == 1) buscaSerial(ALTURA, LARGURA); 
        else if (opcao == 2) {
            printf("\nInciando busca paralela com %d threads...\n", NUMTHREADS);
            
            pthread_t threads[NUMTHREADS];
            pthread_mutex_init(&macroblocoMutex, NULL);
            
            int i;
            for (i = 0; i < NUMTHREADS; i++) {
                pthread_create(&threads[i], NULL, buscaParalela(), NULL);
                pthread_join(threads[i], NULL);
            }

            pthread_mutex_destroy(&macroblocoMutex);
            pthread_mutex_destroy(&qtdPrimosMutex);
        };

        timer = clock() - timer;
        printf("Código executado em  : %.3f segundos\n", ((double)timer) / (CLOCKS_PER_SEC));
        printf("Quantidade de primos na matriz: %d\n", numPrimos);

        freeMatriz(ALTURA);
        numPrimos = 0;

        printf("\n\n\nDigite:\n1 - busca serial\n2 - busca paralela com o numero de threads definido no código\nOutro caractere - sair\n");
        scanf(" %d", &opcao);   
    }
    
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
        matriz[i] = NULL;
    }
    free(matriz);
    matriz = NULL;
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


void* buscaParalela() {
    pthread_exit(0);    
}
