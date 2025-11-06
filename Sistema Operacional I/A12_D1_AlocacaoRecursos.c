/* alocacao.c – Simulador randômico de alocação de recursos com Banqueiro e detecção de deadlock
   Autor: você :)
   Objetivo: demonstrar alocação/liberação, estado seguro/inseguro e deadlock de forma dinâmica
*/

#include <stdio.h>      // printf
#include <stdlib.h>     // rand, srand
#include <locale.h>
#include <time.h>       // time
#include <stdbool.h>    // bool
#include <string.h>     // memset

/* ===================== Parâmetros do cenário ===================== */
#define P 5             // nº de processos
#define R 3             // nº de tipos de recursos
#define PASSOS 200      // iterações máximas de simulação
#define LOG_CADA 1      // imprime a cada N passos (1 = cada passo)
#define PEDIDO_MAX_UM   1 // pede no máx. 1 unidade por recurso por iteração (mais realista)

/* Ative para imprimir ainda mais detalhes */
 #define MODO_TAGARELA

/* ===================== Estado global ===================== */
int Total[R];           // quantidade total por recurso
int Disponivel[R];      // vetor Available
int Max[P][R];          // demanda máxima de cada processo
int Alocado[P][R];      // matriz Allocation
int Need[P][R];         // Need = Max - Allocation
bool Finalizado[P];     // processo já terminou?
int  Esperas[P];        // quantas vezes um processo ficou bloqueado

/* ===================== Utilidades ===================== */
static int min(int a, int b) { return a < b ? a : b; }

static void copia_vetor(const int *src, int *dst, int n) {
    for (int i = 0; i < n; i++) dst[i] = src[i];
}

static bool leq(const int *a, const int *b, int n) {
    for (int i = 0; i < n; i++) if (a[i] > b[i]) return false;
    return true;
}

static void soma(int *a, const int *b, int n) {
    for (int i = 0; i < n; i++) a[i] += b[i];
}

static void sub(int *a, const int *b, int n) {
    for (int i = 0; i < n; i++) a[i] -= b[i];
}

/* Impressão de tabelas (legível) */
static void imprime_estado(int passo, const char *rotulo) {
    if (passo % LOG_CADA != 0) return;

    printf("\n===== PASSO %d: %s =====\n", passo, rotulo);
    printf("Total:       ");
    for (int j = 0; j < R; j++) printf(" %d", Total[j]);
    printf("\nDisponivel:  ");
    for (int j = 0; j < R; j++) printf(" %d", Disponivel[j]);

    printf("\n\n%-6s | %-7s | %-8s | %-9s | %-5s | Esperas\n", "Proc", "Max", "Alocado", "Need", "Fim?");
    printf("---------------------------------------------------------------\n");
    for (int i = 0; i < P; i++) {
        printf("P%-5d | ", i);
        for (int j = 0; j < R; j++) printf("%d", Max[i][j]), j < R-1 ? printf(",") : printf("   | ");
        for (int j = 0; j < R; j++) printf("%d", Alocado[i][j]), j < R-1 ? printf(",") : printf("    | ");
        for (int j = 0; j < R; j++) printf("%d", Need[i][j]), j < R-1 ? printf(",") : printf("     | ");
        printf(" %s  | %d\n", Finalizado[i] ? "Sim" : "Nao", Esperas[i]);
    }
}

/* ===================== Banqueiro (checagem de segurança) ===================== */
/* Retorna true se existir uma sequência segura (ninguém travado no fim) */
static bool estado_e_seguro(void) {
    bool Finish[P];
    int Work[R];
    for (int i = 0; i < P; i++) Finish[i] = Finalizado[i]; // os que já terminaram contam como "finalizáveis"
    copia_vetor(Disponivel, Work, R);

    while (1) {
        bool progresso = false;
        for (int i = 0; i < P; i++) {
            if (Finish[i]) continue;
            if (leq(Need[i], Work, R)) {
                // P_i pode finalizar
                soma(Work, Alocado[i], R); // libera tudo ao terminar
                Finish[i] = true;
                progresso = true;
            }
        }
        if (!progresso) break;
    }

    // Se algum não finalizou, não há sequência segura
    for (int i = 0; i < P; i++) if (!Finish[i]) return false;
    return true;
}

/* Deadlock “de fato”: impossível montar sequência segura no estado atual */
static bool ha_deadlock(void) {
    return !estado_e_seguro();
}

/* ===================== Inicialização aleatória plausível ===================== */
static void inicializa(unsigned seed) {
    srand(seed);

    // Totais “realistas” (pequenos para gerar contenção)
    for (int j = 0; j < R; j++) {
        Total[j] = 2 + rand() % 4; // entre 2 e 5
        Disponivel[j] = Total[j];
    }

    memset(Alocado, 0, sizeof(Alocado));
    memset(Need, 0, sizeof(Need));
    memset(Finalizado, 0, sizeof(Finalizado));
    memset(Esperas, 0, sizeof(Esperas));

    // Max[i][j] aleatório, mas não excede Total[j]; garante que cada processo precise de algo
    for (int i = 0; i < P; i++) {
        int somaMax = 0;
        for (int j = 0; j < R; j++) {
            Max[i][j] = rand() % (Total[j] + 1);   // pode ser 0..Total[j]
            somaMax += Max[i][j];
        }
        // força cada processo a ter alguma necessidade > 0
        if (somaMax == 0) {
            int j = rand() % R;
            Max[i][j] = 1;
        }
        // Need inicial = Max
        for (int j = 0; j < R; j++) Need[i][j] = Max[i][j];
    }
}

/* ===================== Execução de um pedido ===================== */
/* Tenta conceder pedido Req de processo i; usa Banqueiro.
   Retorna: 1 = concedido, 0 = negado (inseguro ou indisponível)
*/
static int tenta_conceder(int i, const int Req[R]) {
    // 1) Requisicao coerente: Req <= Need
    if (!leq(Req, Need[i], R)) return 0;
    // 2) Disponibilidade: Req <= Disponivel
    if (!leq(Req, Disponivel, R)) return 0;

    // 3) Teste de segurança (fase de simulação)
    int backupDisp[R], backupAlloc[R], backupNeed[R];
    copia_vetor(Disponivel, backupDisp, R);
    copia_vetor(Alocado[i], backupAlloc, R);
    copia_vetor(Need[i],     backupNeed, R);

    sub(Disponivel, Req, R);
    soma(Alocado[i], Req, R);
    sub(Need[i],     Req, R);

    bool seguro = estado_e_seguro();

    // Reverte se inseguro
    if (!seguro) {
        //printf("\n--> Não é seguro pegar o P%d pegar o R%d, revertendo...\n\n", i, R);
        copia_vetor(backupDisp, Disponivel, R);
        copia_vetor(backupAlloc, Alocado[i], R);
        copia_vetor(backupNeed,  Need[i], R);
        return 0;
    }
    return 1;
}

/* Finaliza processo i (Need==0), liberando sua alocação */
static void finalizar_processo(int i) {
    soma(Disponivel, Alocado[i], R);
    for (int j = 0; j < R; j++) {
        Alocado[i][j] = 0;
        Need[i][j] = 0;
    }
    Finalizado[i] = true;
}

/* Gera um pedido randômico válido (no máx. 1 por recurso e não maior que Need) */
static void gera_pedido(int i, int Req[R]) {
    int solicitados = 0;
    for (int j = 0; j < R; j++) {
        if (Need[i][j] <= 0) {
            Req[j] = 0;
            continue;
        }
        // 50% de chance de pedir 1 unidade (se ainda precisa), senão 0
        int pede = (rand() % 2 == 0) ? 1 : 0;
        // Respeitar PEDIDO_MAX_UM e Need
        Req[j] = min(pede, min(PEDIDO_MAX_UM, Need[i][j]));
        solicitados += Req[j];
    }
    // Garante que peça ao menos algo, senão força um recurso aleatório que ainda precisa
    if (solicitados == 0) {
        int tentativas = 10;
        while (tentativas--) {
            int j = rand() % R;
            if (Need[i][j] > 0) { Req[j] = 1; break; }
        }
    }
}

/* ===================== Loop de simulação ===================== */
int main(void) {
    system("cls");
    setlocale(LC_ALL, "pt_BR.UTF-8");

    unsigned seed = (unsigned)time(NULL);  // mude para número fixo p/ reprodução
    inicializa(seed);

    printf("Simulador de Alocacao de Recursos (Banqueiro) – seed=%u\n", seed);
    imprime_estado(0, "Estado inicial");

    int passos_sem_progresso = 0;

    for (int passo = 1; passo <= PASSOS; passo++) {
        bool houve_progresso = false;

        // Escolhe um processo candidato que ainda não terminou e ainda precisa de algo
        int candidatos[P], nc = 0;
        for (int i = 0; i < P; i++) {
            if (!Finalizado[i]) {
                bool precisa = false;
                for (int j = 0; j < R; j++) if (Need[i][j] > 0) { precisa = true; break; }
                if (precisa) candidatos[nc++] = i;
            }
        }

        if (nc == 0) {
            printf("\nTodos os processos finalizaram! :)\n");
            imprime_estado(passo, "Fim");
            break;
        }

        // Tenta alguns pedidos por passo para aumentar a dinâmica
        int tentativas = 1 + (rand() % nc); // 1..nc pedidos no passo
        while (tentativas--) {
            int i = candidatos[rand() % nc];

            // Gera um pedido randômico coerente com Need
            int Req[R] = {0};
            gera_pedido(i, Req);

            // Se o pedido é zerado (pode acontecer), pula
            bool zero = true; for (int j = 0; j < R; j++) if (Req[j] > 0) { zero = false; break; }
            if (zero) continue;

            // Tenta conceder via Banqueiro
            if (tenta_conceder(i, Req)) {
                houve_progresso = true;
#ifdef MODO_TAGARELA
                printf("PASSO %d: CONCEDIDO a P%d: Req=[", passo, i);
                for (int j = 0; j < R; j++) printf("%d%s", Req[j], j<R-1?",":"");
                printf("]\n");
#endif
                // Se o processo alcançou Need==0, finaliza e libera
                bool acabou = true;
                for (int j = 0; j < R; j++) if (Need[i][j] > 0) { acabou = false; break; }
                if (acabou) {
                    finalizar_processo(i);
                    printf("PASSO %d: P%d FINALIZOU e liberou seus recursos.\n", passo, i);
                }
            } else {
                Esperas[i]++;
#ifdef MODO_TAGARELA
                printf("PASSO %d: NEGADO (inseguro/indisponivel) para P%d. Req ignorado.\n", passo, i);
#endif
            }
        }

        // Log do estado
        imprime_estado(passo, houve_progresso ? "Progresso" : "Sem progresso");

        // Checa deadlock
        if (ha_deadlock()) {
            printf("\n*** ALERTA: ESTADO INSEGURO/DEADLOCK DETECTADO NO PASSO %d ***\n", passo);
            printf("Nenhuma sequência segura existe a partir do estado atual.\n");
            imprime_estado(passo, "Deadlock");
            break;
        }

        // Controle de progresso para evitar loop “parado”
        if (houve_progresso) passos_sem_progresso = 0;
        else {
            passos_sem_progresso++;
            // Se ficar muito tempo sem progresso, é sinal de saturação/estrelações severas
            if (passos_sem_progresso >= 10) {
                printf("\n*** Saturacao: 10 passos sem progresso. Estado pode estar perto de deadlock.\n");
                if (ha_deadlock()) {
                    printf("*** Confirmado: DEADLOCK ao verificar sequência segura.\n");
                } else {
                    printf("*** Ainda ha sequencia segura, mas pedidos atuais nao permitiram avancar.\n");
                }
                imprime_estado(passo, "Saturacao");
                passos_sem_progresso = 0; // continua tentando
            }
        }
    }

    printf("\nSimulacao encerrada.\n");
    return 0;
}
