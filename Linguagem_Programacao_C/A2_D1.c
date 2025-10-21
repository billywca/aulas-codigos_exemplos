#include <stdio.h>
int main() {
    int idade;
    float altura;
    char nome[30];
    printf("Digite seu nome: ");
    scanf("%s", nome);
    printf("Digite sua idade: ");
    scanf("%d", &idade);
    printf("Digite sua altura: ");
    scanf("%f", &altura);
    printf("Olá %s, você tem %d anos e mede %.2f m.\n",
            nome, idade, altura);
    return 0;
}
