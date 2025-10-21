
#include <stdio.h>
int main() {
    char texto[50];
    int i, contador = 0;
    printf("Digite um texto: ");
    fgets(texto, 50, stdin);
    for (i = 0; texto[i] != '\0'; i++) {
        if (texto[i] != '\n') { // ignora o ENTER
            contador++;
        }
    }
    printf("O texto digitado tem %d caracteres.\n", contador);
    return 0;
}
