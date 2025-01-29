#if defined(_WIN32) || defined(_WIN64)
    #define _CRT_SECURE_NO_WARNINGS 1
    #define _WINSOCK_DEPRECATED_NO_WARNINGS 1
    #pragma comment(lib,"pthreadVC2.lib")
    #define HAVE_STRUCT_TIMESPEC
#endif   

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <pthread.h>

#if !defined(_WIN32) && !defined(_WIN64)
    #include <uv.h>
#endif
 
#define LARGURA 10000
#define ALTURA 10000

/*
    (language of this comment: pt_br)
    UTILIZE TAMANHOS DE LARGURA (COLUNAS) E ALTURA (LINHAS) DE MACROBLOCOS
    QUE DIVIDAM INTEGRALMENTE A MATRIZ (OS MACROBLOCOS SÃO IGUAIS E DEVEM ESTAR
    INSERIDOS NA MATRIZ, SEM SOBRAR ELEMENTOS NALGUM MACROBLOCO OU NA MATRIZ). 
    PARA OBTER OS VERDADEIROS RESULTADOS EXECUTANDO EM PARALELO, USAR SOMENTE
    MACROBLOCOS E MATRIZES QUADRADOS.

    EM LINUX, FOI UTILIZADO A BIBLIOTECA LIBUV, QUE EXIGE INSTALAÇÃO EXTERNA, 
    RELATIVAMENTE COMPLICADA EM WINDOWS — E POR ISTO OPTOU-SE PELAS DIRETIVAS DE
    COMPILAÇÃO ACIMA, PARA UTILIZAR APENAS A BIBLIOTECA TIME PARA AS MEDIÇÕES
    QUANDO ESTIVER EM WINDOWS. PARA INSTALAR ESSA BIBLIOTECA EM UBUNTU, POR
    EXEMPLO, EXECUTE A SEGUINTE LINHA DE COMANDO NO TERMINAL: 
        sudo apt install libuv1-dev

    A BIBLIOTECA LIBUV É UTILIZADA EM NODE.JS, ENTÃO É POSSÍVEL QUE JÁ ESTEJA
    INSTALADA, CASO POSSUA NODE LOCALMENTE. ELA PROVIDENCIA I/O ASSÍNCRONO BASEADO
    EM LAÇOS DE EVENTOS, E POSSUI UMA FUNÇÃO uv_hrtime() QUE RETORNA O ATUAL TIMESTAMP
    EXPRESSO EM NANOSEGUNDOS, RELATIVO A UM TEMPO PASSADO ARBITRÁRIO, NÃO ESTANDO 
    SUJEITO AO TEMPO ATUAL E NEM A CLOCK DRIFTS. TUDO ISSO INDICA PODER CORROBORAR PARA 
    NÃO OCORRER A AGREGAÇÃO DE TEMPO DAS MÚLTIPLAS THREADS, COMO OCORRE COM A BIBLIOTECA
    time.h EM LINUX.
*/

#define MACROB_LARGURA 4
#define MACROB_ALTURA 4 // != MACROB_LARGURA => primeNumbersSerial != primeNumbersParallel
#define NUM_MACROBLOCOS_LARGURA (LARGURA / MACROB_LARGURA)
#define NUM_MACROBLOCOS_ALTURA (ALTURA / MACROB_ALTURA)
#define TOTAL_MACROBLOCOS (NUM_MACROBLOCOS_ALTURA * NUM_MACROBLOCOS_LARGURA)

#define MULTIPLICADOR_I(num_macrobloco) ((num_macrobloco) / NUM_MACROBLOCOS_ALTURA) // no difference using NUM_MACROBLOCOS_ALTURA or NUM_MACROBLOCOS_LARGURA in square blocks and matrix
#define MULTIPLICADOR_J(num_macrobloco) ((num_macrobloco) % NUM_MACROBLOCOS_ALTURA)
/*  obtains the coords (x, y) (think in small matrix, like 4x4 divided by 2x2; we have 4 macroblocks (0, 1, 2, 3))
    like:   (the operations (integer div and mod) give distinct results, used with NUM_MACROBLOCOS_ALTURA in order to obtain a order like bellow)
    0   1           -> pick the 1, e.g.: 1 / 2 => 0 and 1 % 2 => 1; 2 / 2 => 1 and 2 % 2 = 0 (...)
    2   3   (location of all 'macroblocos' inside matrix, each containing 4 elements)
    This directive can converts the coords of a macroblock to the cords of the real matrix, by adding i or j with its
    horizontal or vertical multiplier multiplied by the horizontal or vertical dimension of the macroblock, respectively  

    This works because the integer divisor of each identifier, starting at 0, of macroblocks that are on the same 
    horizontal, will cause the macroblock multiplier on the horizontal axis to remain the same, since the last macroblock 
    on this axis will be a value that, if incremented and divided by that integer divisor, results in a value 1 unit higher; 
    while the rest of the integer division of this macroblock representative will cause the multipliers of the macroblock 
    columns to change from 0 (which marks the start of a new horizontal occupied by macroblocks, since the result is 
    obtained in an integer division), varying by 1, to the number of macroblocks per macroblock vertical subtracted by 1, 
    which, if incremented again, would return to the first column. With the multiplier in hand, simply multiply it by 
    the number of columns and rows, respectively, to obtain the position of the element that is at that index in another 
    macroblock. As in macroblock 1 in the example given, where the first element of its first row and first column will 
    be aij such that i is the same and j is real, with the multiplier incremented by 1 with respect to macroblock 0, it
     is multiplied by the vertical dimension of the macroblock to obtain the start of the macroblock, as if it were 
     “shifted” to the right.
*/
#define LOOP_I_TO_GLOBAL_I(num_macrobloco, loop_i) (MULTIPLICADOR_I(num_macrobloco) * MACROB_ALTURA + (loop_i))
#define LOOP_J_TO_GLOBAL_J(num_macrobloco, loop_j) (MULTIPLICADOR_J(num_macrobloco) * MACROB_LARGURA + (loop_j)) 
#define NUMTHREADS 8


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
  
    #if !defined( _WIN32) || !defined(_WIN64)
        uint64_t comeco, fim;
        double tempoExecSerial;
    #else
        clock_t timer, timerSerial;
    #endif
    
    int opcao;
    printf("\nDigite:\n1 - busca serial\n2 - busca paralela com o numero de threads definido no código\n3 - opcao 1 seguida da opcao 2\nOutro número inteiro - sair\n");
    scanf(" %d", &opcao);

    while (opcao <= 3 && opcao > 0) {
        matriz = mallocMatriz(ALTURA, LARGURA);

        if (opcao == 1) {
            #if !defined(_WIN32) && !defined(_WIN64)
                comeco = uv_hrtime();
            #else
                timer = clock();
            #endif

            buscaSerial(ALTURA, LARGURA);
        }
        else if (opcao == 2 || opcao == 3) {

            if (opcao == 3) {
                #if !defined(_WIN32) && !defined(_WIN64)
                    uint64_t comecoSerial, fimSerial;

                    comecoSerial = uv_hrtime();
                    buscaSerial(ALTURA, LARGURA);
                    fimSerial = uv_hrtime();
                    tempoExecSerial = (fimSerial - comecoSerial) / 1e9;

                    printf("Código serial executado em  : %.3f segundos\n", tempoExecSerial);
                #else
                    timerSerial = clock();
                    buscaSerial(ALTURA, LARGURA);
                    timerSerial = clock() - timerSerial;

                    printf("Código serial executado em  : %.3f segundos\n", ((double)timerSerial) / (CLOCKS_PER_SEC));
                #endif

                printf("Quantidade de primos na matriz: %d\n", numPrimos);

                // restore primeNum conter to parallel search
                numPrimos = 0;
            }
            printf("\nIniciando busca paralela com %d threads...\n", NUMTHREADS);

            pthread_t threads[NUMTHREADS];
            // mutexes to critical regions
            pthread_mutex_init(&macroblocoMutex, NULL);
            pthread_mutex_init(&numPrimosMutex, NULL);

            #if !defined(_WIN32) && !defined(_WIN64)
                comeco = uv_hrtime();
            #else
                timer = clock();
            #endif

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

        #if !defined(_WIN32) && !defined(_WIN64)
            fim = uv_hrtime();
            double tempoExec = (fim - comeco) / 1e9;
            printf("Código executado em  : %.3f segundos\n", tempoExec);
        #else
            timer = clock() - timer;
            printf("Código exe e ecutado em  : %.3f segundos\n", ((double)timer) / (CLOCKS_PER_SEC));
        #endif

        printf("Quantidade de primos na matriz: %d\n", numPrimos);
        if (opcao == 3) {
            #if !defined(_WIN32) && !defined(_WIN64)
                printf("Speedup: %.3f\n", (tempoExecSerial / tempoExec));
            #else
                printf("Speedup: %.3f\n", (((double)timerSerial) / (CLOCKS_PER_SEC)) / (((double)timer) / (CLOCKS_PER_SEC)));
            #endif
        }
        // restore variables to next iteration
        matriz = freeMatriz(ALTURA, LARGURA);
        numPrimos = 0;
        proxMacrobloco = 0;

        printf("\n=============================================================\n");
        printf("Digite:\n1 - busca serial\n2 - busca paralela com o numero de threads definido no código\n3 - opcao 1 seguida da opcao 2\nOutro número inteiro - sair\n");
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
