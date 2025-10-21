
#include <stdio.h>
int main(void) {
    int op; double a, b;
    printf("1-Somar  2-Subtrair  3-Multiplicar  4-Dividir  0-Sair\n");
    printf("Opcao: "); if (scanf("%d", &op) != 1) return 1;
    if (op == 0) { puts("Saindo..."); return 0; }
    printf("Digite dois numeros: ");
    if (scanf("%lf %lf", &a, &b) != 2) return 1;
    switch (op) {
        case 1: printf("= %.2f\n", a + b); break;
        case 2: printf("= %.2f\n", a - b); break;
        case 3: printf("= %.2f\n", a * b); break;
        case 4:
            if (b == 0) puts("Divisao por zero!");
            else printf("= %.2f\n", a / b);
            break;
        default: puts("Opcao invalida.");
    }
    return 0;
}
