#include <stdio.h>
int main(void) {
    char nome[100], ra[50];
    int anoAlvo;
    double salario = 2000.00;
    double aumento1996 = 1.5; // em %
    printf("Seu nome: ");
    fgets(nome, sizeof(nome), stdin);
    printf("Seu RA: ");
    fgets(ra, sizeof(ra), stdin);
    printf("\nCalcular salario para qual ano (>= 2008)? ");
    if (scanf("%d", &anoAlvo) != 1) return 0;
    if (anoAlvo < 2008) anoAlvo = 2008;
    // 1995: contratado (sem aumento)
    // 1996: +1.5%
    // a partir de 1997, o % de aumento dobra em relacao ao ano anterior
    if (anoAlvo >= 2008) {
        salario *= (1.0 + aumento1996 / 100.0);
        double taxa = aumento1996 * 2.0; // taxa para 1997
        for (int ano = 2009; ano <= anoAlvo; ano++) {
            printf("Ano %d: taxa %.2f%%\n", ano, taxa);
            salario *= (1.0 + taxa / 100.0);
            taxa *= 2.0; // dobra a cada ano
            // cuidado: isso cresce muito rápido; em cenários reais caparia
        }
    }
    printf("\nSalario em %d: R$ %.2f\n", anoAlvo, salario);
    return 0;
}
