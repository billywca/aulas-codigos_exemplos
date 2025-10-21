#include <stdio.h>
int main() {
    char frase[50];
    printf("Digite 3 frases (uma por linha):\n");
    for (int i = 0; i < 3; i++) {
        fgets(frase, 50, stdin);
        printf("Frase %d: %s", i + 1, frase);
    }
    return 0;
}
