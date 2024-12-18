#include <stdio.h>
#include <stdlib.h>
#include <math.h>//permitido?
#include <time.h>


#define LARGURA 10000
#define ALTURA 10000
#define MATRIZ 
#define NUM_PRIMOS 


int ehPrimo(int );


int main(int argc, char *argv[]) {
    srand(time(NULL));

    int num_rando = 0 + rand() % (31999 - 0 + 1)
    
    
    return 0;
}


int ehPrimo(int n) {
    int raizInteira; 
    raizInteira = (int)(round(squareRoot(n)));
    for (int i = 2; i <= raizInteira; i++) {
        if (n % i == 0) return 0;
    }
    return 1;
}


