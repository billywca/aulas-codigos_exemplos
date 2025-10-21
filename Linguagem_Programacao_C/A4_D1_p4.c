
#include <stdio.h>
#include <string.h>
int main() {
    char palavra[50];
    printf("Digite uma palavra: ");
    scanf("%s", palavra);
    printf("A palavra '%s' tem %d caracteres.\n", palavra, (int)strlen(palavra));
    return 0;
}
