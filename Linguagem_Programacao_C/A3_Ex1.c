#include <stdio.h>
int main(void) {
    char nome[100], ra[50];
    double salarioCarlos, taxaCarlos, taxaJoao;
    double valC, valJ;
    int meses = 0;
    printf("Seu nome: ");
    fgets(nome, sizeof(nome), stdin);
    printf("Seu RA: ");
    fgets(ra, sizeof(ra), stdin);
    printf("\n=== Carlos x Joao ===\n");
    printf("Salario de Carlos (ex: 3000): ");
    if (scanf("%lf", &salarioCarlos) != 1) return 0;
    printf("Taxa mensal Carlos em %% (ex: 2 para 2%%): ");
    if (scanf("%lf", &taxaCarlos) != 1) return 0;
    printf("Taxa mensal Joao em %% (ex: 5 para 5%%): ");
    if (scanf("%lf", &taxaJoao) != 1) return 0;
    valC = salarioCarlos;       // aplica 100% do salário
    valJ = salarioCarlos / 3.0; // João recebe 1/3 do salário de Carlos
    double rc = 1.0 + taxaCarlos / 100.0;
    double rj = 1.0 + taxaJoao   / 100.0;
    while (valJ < valC) {
        valC *= rc;
        valJ *= rj;
        meses++;
        // segurança contra laços infinitos, caso taxas estranhas:
        if (meses > 1000000) break;
    }
    printf("\nMeses necessarios: %d\n", meses);
    printf("Montante Carlos: R$ %.2f\n", valC);
    printf("Montante Joao  : R$ %.2f\n", valJ);
    return 0;
}
