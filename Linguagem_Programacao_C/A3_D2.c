
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>
#include <math.h>
void pulaLinha(char texto[]){
    printf("\n\n\n");
    for (int i=0; i<50; i++) printf("=");
    printf("\n %s \n", texto);
}
int main() {
    setlocale(LC_ALL, "pt_BR.UTF-8");  // Permite acentuação no terminal
    system("cls"); // ou "clear" no Linux
    pulaLinha("Exemplo 1: Números de 1 a 10");
    
    int i = 1; 
    while (i <= 10) {
        printf("%d ", i);
        i++;
    }
    pulaLinha("Exemplo 2: Soma numeros digitados pelo usuário");
    int num, soma = 0;
    printf("Digite números para somar (0 encerra):\n");
    scanf("%d", &num);
    while (num != 0) {
        soma += num;
        printf("Soma parcial: %d\n", soma);
        scanf("%d", &num);
    }
    printf("Soma final: %d\n", soma);  
 
    pulaLinha("Exemplo 2: Entrando com senha");
    char senha[20];
    printf("Digite a senha (dica: só tem a noite):\n");
    scanf("%s", senha);
    while (strcmp(senha, "luar") != 0) {
        printf("Senha incorreta. Tente novamente:\n");
        scanf("%s", senha);
    }
    printf("Acesso liberado!\n");
    pulaLinha("Exemplo 3: Aproximação do número π com série infinita (Leibniz)");
    printf("\nA aproximação de π pela série de Leibniz é dada pela fórmula: ");
    printf("\n π/4 = 1 - 1/3 + 1/5 - 1/7 + 1/9 - ..., onde os termos são o inverso dos números ímpares, alternando entre soma e subtração. ");
    printf("\nEsta série infinita converge lentamente, exigindo muitos termos para alcançar alta precisão, e foi descoberta por Madhava de Sangamagrama antes de Leibniz, ");
    printf("\nsendo um caso particular da série de Taylor da função arctan(x). \n");
    i = 0;
    double pi = 0.0, termo;
    int limite;
    printf("Digite o número de termos da série de Leibniz: ");
    scanf("%d", &limite);
    while (i < limite) {
        termo = pow(-1, i) / (2 * i + 1);
        pi += termo;
        i++;
    }
    pi *= 4;
    printf("Valor aproximado de pi com %d termos: %.10f\n", limite, pi);
    return 0;
}