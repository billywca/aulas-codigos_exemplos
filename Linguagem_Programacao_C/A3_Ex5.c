#include <stdio.h>
/* Funções de comprimento em letras (acentos contam como 1 letra). */
int len_1_19(int n) {
    switch (n) {
        case 1:  return 2;  // um
        case 2:  return 4;  // dois
        case 3:  return 4;  // três
        case 4:  return 6;  // quatro
        case 5:  return 5;  // cinco
        case 6:  return 4;  // seis
        case 7:  return 4;  // sete
        case 8:  return 4;  // oito
        case 9:  return 4;  // nove
        case 10: return 3;  // dez
        case 11: return 4;  // onze
        case 12: return 4;  // doze
        case 13: return 5;  // treze
        case 14: return 8;  // quatorze (troque para 7 se usar "catorze")
        case 15: return 6;  // quinze
        case 16: return 9;  // dezesseis
        case 17: return 9;  // dezessete
        case 18: return 7;  // dezoito
        case 19: return 8;  // dezenove
        default: return 0;
    }
}
int len_tens(int t) {
    switch (t) {
        case 20: return 5;  // vinte
        case 30: return 6;  // trinta
        case 40: return 8;  // quarenta
        case 50: return 9;  // cinquenta
        case 60: return 8;  // sessenta
        case 70: return 7;  // setenta
        case 80: return 7;  // oitenta
        case 90: return 7;  // noventa
        default: return 0;
    }
}
int len_hundreds(int h, int resto) {
    switch (h) {
        case 100: return (resto == 0) ? 3 : 5; // "cem" ou "cento"
        case 200: return 8;  // duzentos
        case 300: return 9;  // trezentos
        case 400: return 12; // quatrocentos
        case 500: return 9;  // quinhentos
        case 600: return 9;  // seiscentos
        case 700: return 9;  // setecentos
        case 800: return 9;  // oitocentos
        case 900: return 9;  // novecentos
        default: return 0;
    }
}
int len_number_ptbr(int n) {
    if (n == 1000) return 3; // mil
    int total = 0;
    int centenas = n / 100;
    int dezuni   = n % 100;
    int dezenas  = dezuni / 10;
    int unidades = dezuni % 10;
    if (centenas > 0) {
        int h = centenas * 100;
        total += len_hundreds(h, dezuni);
        if (h != 100 && dezuni != 0) total += 1; // "e"
    }
    if (dezuni > 0 && dezuni < 20) {
        total += len_1_19(dezuni);
        return total;
    }
    if (dezenas > 0) {
        total += len_tens(dezenas * 10);
        if (unidades > 0) total += 1; // "e" entre dezenas e unidades
    }
    if (unidades > 0) total += len_1_19(unidades);
    return total;
}
int main(void) {
    char nome[100], ra[50];
    long long soma = 0;
    printf("Seu nome: ");
    fgets(nome, sizeof(nome), stdin);
    printf("Seu RA: ");
    fgets(ra, sizeof(ra), stdin);
    for (int i = 1; i <= 1000; i++) soma += len_number_ptbr(i);
    printf("\nTotal de letras de 1 a 1000 (sem espacos nem hifens): %lld\n", soma);
    // opcional: checagem 1..5 = 21 (com 'três' = 4 letras)
    long long check = 0;
    for (int i = 1; i <= 5; i++) check += len_number_ptbr(i);
    printf("Checagem 1..5: %lld\n", check);
    return 0;
}

