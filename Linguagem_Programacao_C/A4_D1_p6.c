#include <stdio.h>
#include <string.h>
int main() {
    char senha[20] = "12345";
    char tentativa[20];
    printf("Digite a senha: ");
    scanf("%s", tentativa);
    if (strcmp(senha, tentativa) == 0)
        printf("Acesso permitido!\n");
    else
        printf("Senha incorreta.\n");
    return 0;
}
