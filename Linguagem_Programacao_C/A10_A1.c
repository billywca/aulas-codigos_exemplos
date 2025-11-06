#include <stdio.h>
#include <stdlib.h>

#define TAMANHO 5

// Função para inicializar a matriz com zeros
void inicializarMatriz(int matriz[][TAMANHO], int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            matriz[i][j] = 0;
        }
    }
}

// Função para exibir a matriz
void exibirMatriz(int matriz[][TAMANHO], int n) {
    printf("\n   ");
    for (int j = 0; j < n; j++) {
        printf(" %d ", j);
    }
    printf("\n");
    
    for (int i = 0; i < n; i++) {
        printf("%d: ", i);
        for (int j = 0; j < n; j++) {
            if (matriz[i][j] == 1) {
                printf(" X ");
            } else {
                printf(" . ");
            }
        }
        printf("\n");
    }
    printf("\n");
}

// Função para marcar a posição específica
void marcarPosicao(int matriz[][TAMANHO], int linha, int coluna) {
    matriz[linha][coluna] = 1;
}

// Função para marcar toda a linha
void marcarLinha(int matriz[][TAMANHO], int linha, int n) {
    for (int j = 0; j < n; j++) {
        matriz[linha][j] = 1;
    }
}

// Função para marcar toda a coluna
void marcarColuna(int matriz[][TAMANHO], int coluna, int n) {
    for (int i = 0; i < n; i++) {
        matriz[i][coluna] = 1;
    }
}

// Função para marcar as diagonais que passam pela posição
void marcarDiagonais(int matriz[][TAMANHO], int linha, int coluna, int n) {
    // Diagonal principal (\ - de cima-esquerda para baixo-direita)
    // Encontrar o início da diagonal
    int i = linha, j = coluna;
    while (i > 0 && j > 0) {
        i--;
        j--;
    }
    // Marcar toda a diagonal
    while (i < n && j < n) {
        matriz[i][j] = 1;
        i++;
        j++;
    }
    
    // Diagonal secundária (/ - de cima-direita para baixo-esquerda)
    // Encontrar o início da diagonal
    i = linha;
    j = coluna;
    while (i > 0 && j < n - 1) {
        i--;
        j++;
    }
    // Marcar toda a diagonal
    while (i < n && j >= 0) {
        matriz[i][j] = 1;
        i++;
        j--;
    }
}

int main() {
    int matriz[TAMANHO][TAMANHO];
    int linha, coluna;
    
    // Inicializar a matriz
    inicializarMatriz(matriz, TAMANHO);
    
    printf("=== PROGRAMA DE MARCACAO DE MATRIZ ===\n");
    printf("Tamanho da matriz: %dx%d\n", TAMANHO, TAMANHO);
    
    // Exibir matriz inicial
    printf("\nMatriz inicial:");
    exibirMatriz(matriz, TAMANHO);
    
    // Pedir posição ao usuário
    printf("Digite a linha (0 a %d): ", TAMANHO - 1);
    scanf("%d", &linha);
    
    printf("Digite a coluna (0 a %d): ", TAMANHO - 1);
    scanf("%d", &coluna);
    
    // Validar entrada
    if (linha < 0 || linha >= TAMANHO || coluna < 0 || coluna >= TAMANHO) {
        printf("\nPosicao invalida!\n");
        return 1;
    }
    
    // Marcar a posição escolhida
    printf("\n--- Marcando posicao [%d][%d] ---", linha, coluna);
    marcarPosicao(matriz, linha, coluna);
    exibirMatriz(matriz, TAMANHO);
    
    // Marcar a linha
    printf("--- Marcando linha %d ---", linha);
    marcarLinha(matriz, linha, TAMANHO);
    exibirMatriz(matriz, TAMANHO);
    
    // Marcar a coluna
    printf("--- Marcando coluna %d ---", coluna);
    marcarColuna(matriz, coluna, TAMANHO);
    exibirMatriz(matriz, TAMANHO);
    
    // Marcar as diagonais
    printf("--- Marcando diagonais que passam por [%d][%d] ---", linha, coluna);
    marcarDiagonais(matriz, linha, coluna, TAMANHO);
    exibirMatriz(matriz, TAMANHO);
    
    printf("Marcacao completa!\n");
    
    return 0;
}