#include <stdio.h>
int main(void) {
    char nome[100], ra[50];
    double chico = 1.50; // metros
    double ze    = 1.10; // metros
    int anos = 0;
    printf("Seu nome: ");
    fgets(nome, sizeof(nome), stdin);
    printf("Seu RA: ");
    fgets(ra, sizeof(ra), stdin);
    // Crescimentos: +0.02 m/ano e +0.03 m/ano
    while (ze <= chico) {
        chico += 0.02;
        ze    += 0.03;
        anos++;
        if (anos > 200) break; // segurança
    }
    printf("\nAnos necessarios para Ze ser maior que Chico: %d\n", anos);
    printf("Altura final Chico: %.2f m\n", chico);
    printf("Altura final Ze   : %.2f m\n", ze);
    return 0;
}
