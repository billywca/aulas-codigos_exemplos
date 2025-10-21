#include <stdio.h>
#include <string.h>
int main() {
    char nome[20], sobrenome[20], completo[50];
    printf("Digite o nome: ");
    scanf("%s", nome);
    printf("Digite o sobrenome: ");
    scanf("%s", sobrenome);
    strcpy(completo, nome);          // copia nome para completo
    strcat(completo, " ");           // adiciona espaço
    strcat(completo, sobrenome);     // concatena sobrenome
    printf("Nome completo: %s\n", completo);
    return 0;
}