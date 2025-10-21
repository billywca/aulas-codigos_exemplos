
#include <stdio.h>
int main() {

    // Exemplo 1: Imprimir números de 1 a 10
    printf("Exemplo 1: Números de 1 a 10\n");
    for (int i = 1; i <= 10; i++) {
        printf("%d ", i);
    }

    printf("\n\n");
    // Exemplo 2: Imprimir números pares de 2 a 20
    printf("Exemplo 2: Números pares de 2 a 20\n");
    for (int i = 2; i <= 20; i += 2) {
        printf("%d ", i);
    }

    printf("\n\n");
    // Exemplo 3: Calcular a soma dos números de 1 a 100
    printf("Exemplo 3: Soma dos números de 1 a 100\n");
    int soma = 0;
    for (int i = 1; i <= 100; i++) {
        soma += i;
    }

    printf("A soma é: %d\n\n", soma);
    // Exemplo 4: Imprimir uma tabuada (do 5, por exemplo)
    printf("Exemplo 4: Tabuada do 5\n");
    int numero = 5;
    for (int i = 1; i <= 10; i++) {
        printf("%d x %d = %d\n", numero, i, numero * i);
    }
    printf("\n");

    // Exemplo 5: Imprimir um triângulo de asteriscos
    printf("Exemplo 5: Triângulo de asteriscos\n");
    int linhas = 5;
    for (int i = 1; i <= linhas; i++) {
        for (int j = 1; j <= i; j++) {
            printf("*");
        }
        printf("\n");
    }
    printf("\n");

    // Exemplo 6: Verificar se um número é primo
    printf("Exemplo 6: Verificar se um número é primo (ex: 29)\n");
    int num = 29;
    int ehPrimo = 1; // Assumimos que é primo inicialmente
    if (num <= 1) {
        ehPrimo = 0; // Números menores ou iguais a 1 não são primos
    } else {
        for (int i = 2; i * i <= num; i++) {
            if (num % i == 0) {
                ehPrimo = 0; // Encontrou um divisor, não é primo
                break;
            }
        }
    }
    if (ehPrimo) {
        printf("%d é um número primo.\n", num);
    } else {
        printf("%d não é um número primo.\n", num);
    }
    printf("\n");

    // Exemplo 7: Calcular o fatorial de um número
    printf("Exemplo 7: Calcular o fatorial de um número (ex: 5)\n");
    int n = 5;
    long long fatorial = 1; // Usar long long para evitar overflow
    if (n < 0) {
            printf("Fatorial não está definido para números negativos.\n");
    } else {
            for (int i = 1; i <= n; i++) {
                     fatorial *= i;
            }
            printf("O fatorial de %d é %lld\n", n, fatorial);
     }
    printf("\n");
    return 0;
}
