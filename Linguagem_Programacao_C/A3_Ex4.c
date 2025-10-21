#include <stdio.h>
int main(void) {
    char nome[100], ra[50];
    int saque;
    int notas[] = {100, 50, 20, 5, 2, 1};
    int qtd[6] = {0};
    printf("Seu nome: ");
    fgets(nome, sizeof(nome), stdin);
    printf("Seu RA: ");
    fgets(ra, sizeof(ra), stdin);
    printf("\nValor do saque (inteiro em reais): ");
    if (scanf("%d", &saque) != 1 || saque < 0) {
        printf("Valor invalido.\n");
        return 0;
    }
    int resto = saque;
    for (int i = 0; i < 6; i++) {
        qtd[i] = resto / notas[i];
        resto  = resto % notas[i];
    }
    printf("\nSaque: R$ %d\n", saque);
    for (int i = 0; i < 6; i++) {
        printf("Notas de %3d: %d\n", notas[i], qtd[i]);
    }
    return 0;
}
