#include <stdio.h>
#include <string.h>
int main() {
    int idade;
    float altura;
    char nome[30];
    printf("Digite seu nome: ");
    fgets(nome, sizeof(nome), stdin);
     // Fazer com e sem
     // Remove o '\n' que o fgets adiciona no final 
    nome[strcspn(nome, "\n")] = '\0';
    printf("Digite sua idade: ");
    scanf("%d", &idade);
    printf("Digite sua altura: ");
    scanf("%f", &altura);
    printf("Olá %s, você tem %d anos e mede %.2f m.\n",
            nome, idade, altura);
    return 0;
}