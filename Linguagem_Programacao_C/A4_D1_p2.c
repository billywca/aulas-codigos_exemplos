#include <stdio.h>
int main() {
    char frase[50];
    printf("Digite uma frase: ");
    fgets(frase, 50, stdin);
    printf("Voce digitou: %s", frase);
    return 0;
}
