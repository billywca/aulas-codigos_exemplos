#include <stdio.h>
#include <ctype.h>
int main(void) {
    char grau;
    printf("Grau (A/B/C/D/F): ");
    if (scanf(" %c", &grau) != 1) return 1;
    grau = toupper((unsigned char)grau);
    switch (grau) {
        case 'A': puts("Excelente"); break;
        case 'B': puts("Bom");       break;
        case 'C': puts("Regular");   break;
        case 'D': puts("Ruim");      break;
        case 'F': puts("Reprovado"); break;
        default:  puts("Grau invalido");
    }
    return 0;
}
