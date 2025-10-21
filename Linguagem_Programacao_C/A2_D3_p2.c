#include <stdio.h>
int main(void) {
    int nota;
    printf("nota: ");
    if (scanf("%f", &nota) != 1) return 1;
    if (nota < 0 || nota > 10) {
        puts("Nota fora do intervalo [0,10].");
        return 1; // ou repete leitura
    }
    return 0;
}
