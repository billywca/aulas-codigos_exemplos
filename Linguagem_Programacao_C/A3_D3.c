
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>

int main() {
    setlocale(LC_ALL, "pt_BR.UTF-8");  // Permite acentuação no terminal
    system("cls"); // ou "clear" no Linux
    // Exemplo 1: Imprimir números de 1 a 5
    int i = 1;
    do {
        printf("%d ", i);
        i++;
    } while (i <= 5);
    printf("\n");
    // Exemplo 2: Pedir um número ao usuário até que ele seja positivo
    int num;
    do {
        printf("Digite um número positivo: ");
        scanf("%d", &num);
    } while (num <= 0);
    printf("Você digitou: %d\n", num);
    // Exemplo 3: Jogo de adivinhação simples
    int secreto = 7;
    int palpite;
    do {
        printf("Adivinhe o número (1-10): ");
        scanf("%d", &palpite);
        if (palpite > secreto) {
            printf("Muito alto!\n");
        } else if (palpite < secreto) {
            printf("Muito baixo!\n");
        } else {
            printf("Você acertou!\n");
        }
    } while (palpite != secreto);
    // Exemplo 4: Menu de opções
    int opcao;
    do {
        printf("\nMenu:\n");
        printf("1. Opção 1\n");
        printf("2. Opção 2\n");
        printf("3. Sair\n");
        printf("Escolha uma opção: ");
        scanf("%d", &opcao);
        switch (opcao) {
            case 1:
                printf("Você escolheu a Opção 1\n");
                break;
            case 2:
                printf("Você escolheu a Opção 2\n");
                break;
            case 3:
                printf("Saindo...\n");
                break;
            default:
                printf("Opção inválida.\n");
        }
    } while (opcao != 3);
    return 0;
}
