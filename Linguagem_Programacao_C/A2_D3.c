
/* operadores_demo.c */
#include <stdio.h>
int main(void) {
    int idade;
    printf("Idade: ");
    if (scanf("%d", &idade) != 1) return 1;
    if (idade < 0) {
        puts("Idade inválida.");
    } else if (idade < 12) {
        puts("Criança");
    } else if (idade < 18) {
        puts("Adolescente");
    } else if (idade < 60) {
        puts("Adulto");
    } else {
        puts("Idoso");
    }
    return 0;
}

