
/* operadores_demo.c */
#include <stdio.h>
#include <stdbool.h>
int main(void) {
    puts("==== OPERADORES EM C ====\n");
    /* ---------------------- *
     * 1) ARITMÉTICOS         *
     * ---------------------- */
    int a = 10, b = 3;
    printf("[Aritméticos]\n");
    printf("a = %d, b = %d\n", a, b);
    printf("a + b = %d\n", a + b);
    printf("a - b = %d\n", a - b);
    printf("a * b = %d\n", a * b);
    printf("a / b = %d  (divisão inteira)\n", a / b);
    printf("a %% b = %d  (resto/módulo)\n", a % b);
    /* Precedência e parênteses */
    printf("3 + 2 * 5 = %d   (precedência: * antes de +)\n", 3 + 2 * 5);
    printf("(3 + 2) * 5 = %d (forçando ordem pelos parênteses)\n\n", (3 + 2) * 5);
    /* Divisão real (casting) */
    printf("Divisão real (double)a / b = %.2f\n\n", (double)a / b);
    /* ---------------------- *
     * 2) ATRIBUIÇÃO          *
     * ---------------------- */
    printf("[Atribuição]\n");
    int c = a;               // atribuição simples
    printf("c = a  -> c = %d\n", c);
    c += 5;  printf("c += 5 -> c = %d\n", c);
    c -= 2;  printf("c -= 2 -> c = %d\n", c);
    c *= 3;  printf("c *= 3 -> c = %d\n", c);
    c /= 4;  printf("c /= 4 -> c = %d\n", c);
    c %= 5;  printf("c %%= 5 -> c = %d\n\n", c);
    /* ---------------------- *
     * 3) ++ e --             *
     * ---------------------- */
    printf("[Incremento/Decremento]\n");
    int i = 5;
    printf("i inicial = %d\n", i);
    printf("i++ (pos-incremento) retorna %d, i vira %d\n", i++, i);
    printf("++i (pre-incremento) agora retorna %d, i = %d\n", ++i, i);
    printf("i-- (pos-decremento) retorna %d, i vira %d\n", i--, i);
    printf("--i (pre-decremento) agora retorna %d, i = %d\n\n", --i, i);
    /* ---------------------- *
     * 4) RELACIONAIS         *
     * ---------------------- */
    printf("[Relacionais]\n");
    printf("a == b -> %d\n", a == b);
    printf("a != b -> %d\n", a != b);
    printf("a >  b -> %d\n", a >  b);
    printf("a <  b -> %d\n", a <  b);
    printf("a >= b -> %d\n", a >= b);
    printf("a <= b -> %d\n\n", a <= b);
    /* ---------------------- *
     * 5) LÓGICOS             *
     * ---------------------- */
    printf("[Lógicos]\n");
    bool cond1 = (a > 5);      // true
    bool cond2 = (b == 3);     // true
    bool cond3 = (a % 2 == 0); // true se a for par
    printf("cond1 (a>5)  = %d\n", cond1);
    printf("cond2 (b==3) = %d\n", cond2);
    printf("cond3 (a par)= %d\n", cond3);
    printf("cond1 && cond2 -> %d\n", cond1 && cond2);
    printf("cond1 || (b>10) -> %d\n", cond1 || (b > 10));
    printf("!cond3 -> %d\n\n", !cond3);
    puts("\n==== FIM ====");
    return 0;
}
