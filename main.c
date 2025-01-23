/* Uncomment below if WINDOWS + VS 2022 */
/*
#pragma once
#define _CRT_SECURE_NO_WARNINGS 1
#define _WINSOCK_DEPRECATED_NO_WARNINGS 1
#pragma comment(lib,"pthreadVC2.lib")
#define HAVE_STRUCT_TIMESPEC
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <pthread.h>

#include <uv.h>

#define LARGURA 10000
#define ALTURA 10000

/*
    (language of this comment: pt_br)
    UTILIZE TAMANHOS DE LARGURA (COLUNAS) E ALTURA (LINHAS) DE MACROBLOCOS
    QUE DIVIDAM INTEGRALMENTE A MATRIZ (OS MACROBLOCOS SÃO IGUAIS E DEVEM ESTAR
    INSERIDOS NA MATRIZ, SEM SOBRAR ELEMENTOS NALGUM MACROBLOCO OU NA MATRIZ). 
    PARA OBTER OS VERDADEIROS RESULTADOS EXECUTANDO EM PARALELO, USAR SOMENTE
    MACROBLOCOS E MATRIZES QUADRADOS.

    EM LINUX, FOI UTILIZADO A BIBLIOTECA LIBUV, QUE EXIGE INSTALAÇÃO EXTERNA, QUE
    É RELATIVAMENTE COMPLICADA EM WINDOWS. BASTA COMENTAR O INCLUDE DE UV.H E AS
    VARIÁVEIS DE TIPO uint64_t E A VARIÁVEL tempoExecSerial, UTILIZANDO APENAS A
    BIBLIOTECA TIME APÓS DESCOMENTAR OS TRECHOS QUE MANIPULAM AS VARIÁVEIS DE TIPO
    clock_t. 

    A BIBLIOTECA LIBUV É UTILIZADA EM NODE.JS, ENTÃO É POSSÍVEL QUE JÁ ESTEJA
    INSTALADA, CASO POSSUA NODE LOCALMENTE. POR PROVIDENCIAR I/O ASSÍNCRONO BASEADO
    EM LAÇOS DE EVENTOS, ACABA POR TER UMA MÉTRICA DE TEMPORIZAÇÃO, EM LINUX, QUE
    ESCAPA DA AGREGAÇÃO DE TEMPO DAS MÚLTIPLAS THREADS, COMO OCORRE COM A BIBLIOTECA
    time.h EM LINUX.
*/

#define MACROB_LARGURA 1000
#define MACROB_ALTURA 1000 // != MACROB_LARGURA => primeNumbersSerial != primeNumbersParallel
#define NUM_MACROBLOCOS_LARGURA (LARGURA / MACROB_LARGURA)
#define NUM_MACROBLOCOS_ALTURA (ALTURA / MACROB_ALTURA)
#define TOTAL_MACROBLOCOS (NUM_MACROBLOCOS_ALTURA * NUM_MACROBLOCOS_LARGURA)

#define MULTIPLICADOR_I(num_macrobloco) ((num_macrobloco) / NUM_MACROBLOCOS_ALTURA)
#define MULTIPLICADOR_J(num_macrobloco) ((num_macrobloco) % NUM_MACROBLOCOS_ALTURA)
/*  obtains the coords (x, y) (think in small matrix, like 4x4 divided by 2x2; we have 4 macroblocks (0, 1, 2, 3))
    like:   (the operations (integer div and mod) give distinct results, used with NUM_MACROBLOCOS_ALTURA in order to obtain a order like bellow)
    0   1           -> pick the 1, e.g.: 1 / 2 => 0 and 1 % 2 => 1; 2 / 2 => 1 and 2 % 2 = 0 (...)
    2   3   (location of all 'macroblocos' inside matrix, each containing 4 elements)
    This directive can converts the coords of a 'macrobloco' to the cords of the real matrix, by adding x or y with the multiplying of their inverse (x * mblockcolumns, y * mblockheight, like bellow)
*/
#define LOOP_I_TO_GLOBAL_I(num_macrobloco, loop_i) (MULTIPLICADOR_I(num_macrobloco) * MACROB_ALTURA + (loop_i))
#define LOOP_J_TO_GLOBAL_J(num_macrobloco, loop_j) (MULTIPLICADOR_J(num_macrobloco) * MACROB_LARGURA + (loop_j)) 
#define NUMTHREADS 2


int** matriz;
int numPrimos = 0;
int proxMacrobloco = 0;
pthread_mutex_t numPrimosMutex;
pthread_mutex_t macroblocoMutex;

// add -lm -luv if gcc (-lm to math.h and -luv to uv.h (needs install libuv))

int ehPrimo(int n);
int** mallocMatriz(int altura, int largura);
int** freeMatriz(int altura, int largura);

void buscaSerial(int altura, int largura);
void* buscaParalela();

int main(int argc, char* argv[]) {
    srand(time(NULL));
  
    // clock_t timer, timerSerial;

    uint64_t comeco, fim;
    double tempoExecSerial;

    int opcao;
    printf("\nDigite:\n1 - busca serial\n2 - busca paralela com o numero de threads definido no código\n3 - opcao 1 seguida da opcao 2\nOutro caractere - sair\n");
    scanf(" %d", &opcao);

    while (opcao <= 3 && opcao > 0) {
        matriz = mallocMatriz(ALTURA, LARGURA);
        //timer = clock();

        if (opcao == 1) {
            // timer = clock();

            comeco = uv_hrtime();

            buscaSerial(ALTURA, LARGURA);
        }
        else if (opcao == 2 || opcao == 3) {

            if (opcao == 3) {
                //timerSerial = clock();

                uint64_t comecoSerial, fimSerial;
                comecoSerial = uv_hrtime();

                buscaSerial(ALTURA, LARGURA);

                /*
                timerSerial = clock() - timerSerial;
                printf("Código serial executado em  : %.3f segundos\n", ((double)timerSerial) / (CLOCKS_PER_SEC));
                */

                fimSerial = uv_hrtime();
                tempoExecSerial = (fimSerial - comecoSerial) / 1e9;
                printf("Código serial executado em  : %.3f segundos\n", tempoExecSerial);

                printf("Quantidade de primos na matriz: %d\n", numPrimos);

                // restore primeNum conter to parallel search
                numPrimos = 0;
            }
            printf("\nIniciando busca paralela com %d threads...\n", NUMTHREADS);

            pthread_t threads[NUMTHREADS];
            // mutexes to critical regions
            pthread_mutex_init(&macroblocoMutex, NULL);
            pthread_mutex_init(&numPrimosMutex, NULL);

            //timer = clock();
            comeco = uv_hrtime();

            int i;
            for (i = 0; i < NUMTHREADS; i++) {
                pthread_create(&threads[i], NULL, buscaParalela, NULL);
            }
            // join outside create loop to take advantage of parallelism
            for (i = 0; i < NUMTHREADS; i++) {
                pthread_join(threads[i], NULL);
            }

            // destroy mutexes after all
            pthread_mutex_destroy(&macroblocoMutex);
            pthread_mutex_destroy(&numPrimosMutex);
        };

        //timer = clock() - timer;

        fim = uv_hrtime();
        double tempoExec = (fim - comeco) / 1e9;
        printf("Código executado em  : %.3f segundos\n", tempoExec);

        //printf("Código executado em  : %.3f segundos\n", ((double)timer) / (CLOCKS_PER_SEC));
        printf("Quantidade de primos na matriz: %d\n", numPrimos);
        if (opcao == 3) {
            //printf("Speedup: %.3f\n", (((double)timerSerial) / (CLOCKS_PER_SEC)) / (((double)timer) / (CLOCKS_PER_SEC)));
            printf("Speedup: %.3f\n", (tempoExecSerial / tempoExec));
        }
        // restore variables to next iteration
        matriz = freeMatriz(ALTURA, LARGURA);
        numPrimos = 0;
        proxMacrobloco = 0;

        printf("\n=============================================================\n");
        printf("Digite:\n1 - busca serial\n2 - busca paralela com o numero de threads definido no código\n3 - opcao 1 seguida da opcao 2\nOutro caractere - sair\n");
        scanf(" %d", &opcao);
    }

    return 0;
}


int ehPrimo(int n) {
    int raizInteira, i;
    double raiz;

    if (n <= 1) return 0;

    raizInteira = (int)(round(sqrt(n))); // we use this method
    for (i = 2; i <= raizInteira; i++) {
        if (n % i == 0) return 0;
    }
    return 1;
}


int** mallocMatriz(int altura, int largura) {
    if (altura < 1 || largura < 1) {
        printf("ERRO. Parametro invalido\n");
        return (NULL);
    }

    matriz = (int**)malloc(altura * sizeof(int*));
    if (matriz == NULL) {
        printf("ERRO. Memoria insuficiente\n");
        return (NULL);
    }

    int i, j;
    for (int i = 0; i < altura; i++) {
        matriz[i] = (int*)malloc(largura * sizeof(int));
        if (matriz[i] == NULL) {
            printf("ERRO. Memoria insuficiente para a linha %d da matriz\n", i);
            return (NULL);
        }
    }

    for (i = 0; i < altura; i++) {
        for (j = 0; j < largura; j++) {
            matriz[i][j] = 0 + rand() % (31999 - 0 + 1);
        }
    }

    return matriz;
}


int** freeMatriz(int altura, int largura) {
    if (matriz == NULL) return (NULL);

    if (altura < 1 || largura < 1) {
        printf("ERRO. Parametro invalido\n");
        return matriz;
    }

    int i;
    for (i = 0; i < altura; i++) {
        free(matriz[i]);
        matriz[i] = NULL; // null for the lines
    }
    free(matriz);   // free matrix 
    return (NULL);  // return matrix
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
    int numPrimosLocal = 0;

    for (int proxMacroblocoLocal = 0; proxMacroblocoLocal < TOTAL_MACROBLOCOS; ) {
        // critical region (proxMacrobloco, etc.)
        pthread_mutex_lock(&macroblocoMutex);
        if (proxMacrobloco >= TOTAL_MACROBLOCOS) {
            pthread_mutex_unlock(&macroblocoMutex);
            break;
        }
        proxMacroblocoLocal = proxMacrobloco;
        ++proxMacrobloco;                           // update global var to other iterations 
        pthread_mutex_unlock(&macroblocoMutex);     // unlock after it has been updated or all macroblocks have been analyzed

        // search for primeNumbers (inside 'macrobloco') outside critical region
        int matrizX, matrizY;
        for (int i = 0; i < MACROB_ALTURA; i++) {
            for (int j = 0; j < MACROB_LARGURA; j++) {
                matrizX = LOOP_I_TO_GLOBAL_I(proxMacroblocoLocal, i);
                matrizY = LOOP_J_TO_GLOBAL_J(proxMacroblocoLocal, j);

                if (ehPrimo(matriz[matrizX][matrizY]) == 1) numPrimosLocal += 1;
            }
        }
    }

    // critical region (global primeNumbers counter)
    pthread_mutex_lock(&numPrimosMutex);
    numPrimos += numPrimosLocal;
    pthread_mutex_unlock(&numPrimosMutex);

    pthread_exit(0);
}
