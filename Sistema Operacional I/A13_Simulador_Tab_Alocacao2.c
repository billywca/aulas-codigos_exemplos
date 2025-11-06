/*  fat_sim_color.c — Simulador didático de Tabela de Alocação de Arquivos (FAT) com logs coloridos
    Operações: mkfs, create <nome> <bytes>, append <nome> <bytes>, del <nome>, ls, fat, defrag, help, exit
    Conceitos: cadeia de clusters via FAT[cluster] = proximo, com sentinela EOC (-1).
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#if defined(_WIN32)
  #include <windows.h>
#endif

/* ====== CORES ANSI ====== */
#define C_RESET  "\x1b[0m"
#define C_DIM    "\x1b[2m"
#define C_BOLD   "\x1b[1m"
#define C_RED    "\x1b[31m"
#define C_GRN    "\x1b[32m"
#define C_YEL    "\x1b[33m"
#define C_BLU    "\x1b[34m"
#define C_MAG    "\x1b[35m"
#define C_CYN    "\x1b[36m"
#define C_WHT    "\x1b[37m"

#define LOG_INFO(fmt, ...)  printf(C_CYN  "[info] "  C_RESET fmt "\n", ##__VA_ARGS__)
#define LOG_OK(fmt, ...)    printf(C_GRN  "[ ok ] "  C_RESET fmt "\n", ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)  printf(C_YEL  "[warn] "  C_RESET fmt "\n", ##__VA_ARGS__)
#define LOG_ERR(fmt, ...)   printf(C_RED  "[erro] "  C_RESET fmt "\n", ##__VA_ARGS__)

/* ===== Parametrização do "disco" ===== */
#define CLUSTER_COUNT  256
#define CLUSTER_SIZE   1024
#define DIR_CAPACITY   64

#define FAT_FREE       0
#define FAT_EOC       -1

typedef struct {
    char  name[32];
    int   size;       /* bytes */
    int   first;      /* primeiro cluster da cadeia, ou -1 */
    int   in_use;     /* 0 livre | 1 usado */
} DirEntry;

/* ===== Estado global ===== */
static int      FAT[CLUSTER_COUNT];
static DirEntry ROOT[DIR_CAPACITY];
static int      g_formatted = 0;

/* ===== Habilitar ANSI no Windows (quando possível) ===== */
static void enable_vt_mode(void) {
#if defined(_WIN32)
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return;
    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode)) return;
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
#endif
}

/* ===== Utilidades ===== */
static int clusters_needed(int bytes) {
    if (bytes <= 0) return 1;
    int n = bytes / CLUSTER_SIZE;
    if (bytes % CLUSTER_SIZE) n++;
    if (n == 0) n = 1;
    return n;
}

static void mkfs(void) {
    for (int i = 0; i < CLUSTER_COUNT; ++i) FAT[i] = FAT_FREE;
    for (int i = 0; i < DIR_CAPACITY; ++i) {
        ROOT[i].in_use = 0;
        ROOT[i].first  = -1;
        ROOT[i].size   = 0;
        ROOT[i].name[0]= '\0';
    }
    g_formatted = 1;
    LOG_OK("mkfs concluído: %d clusters de %d bytes (%.1f KiB).",
           CLUSTER_COUNT, CLUSTER_SIZE,
           (CLUSTER_COUNT * CLUSTER_SIZE) / 1024.0);
}

static int dir_find_free_slot(void) {
    for (int i = 0; i < DIR_CAPACITY; ++i) if (!ROOT[i].in_use) return i;
    return -1;
}

static int dir_find_by_name(const char *name) {
    for (int i = 0; i < DIR_CAPACITY; ++i) {
        if (ROOT[i].in_use && strcasecmp(ROOT[i].name, name) == 0) return i;
    }
    return -1;
}

static void fs_ls(void) {
    int count = 0;
    printf(C_BOLD "Nome                           Tamanho(B)   1ºCluster\n" C_RESET);
    printf(C_DIM  "-----------------------------------------------------\n" C_RESET);
    for (int i = 0; i < DIR_CAPACITY; ++i) {
        if (ROOT[i].in_use) {
            printf("%-30s %10d   %d\n", ROOT[i].name, ROOT[i].size, ROOT[i].first);
            count++;
        }
    }
    if (count == 0) printf(C_DIM "(diretório vazio)\n" C_RESET);
}

/* imprime apenas entradas ativas e estatísticas, com cores */
static void print_fat(void) {
    int freec = 0, usedc = 0;

    printf(C_BOLD "FAT (cluster: next)\n" C_RESET);
    for (int i = 0; i < CLUSTER_COUNT; ++i) {
        if (FAT[i] == FAT_FREE) { freec++; continue; }
        usedc++;
        if (FAT[i] == FAT_EOC) {
            printf("[%3d] -> " C_MAG "EOC" C_RESET "\n", i);
        } else {
            printf("[%3d] -> " C_BLU "%d" C_RESET "\n", i, FAT[i]);
        }
    }

    printf("Clusters: total=%d, livres=" C_GRN "%d" C_RESET ", usados=" C_YEL "%d" C_RESET "\n",
           CLUSTER_COUNT, freec, usedc);
}

static int chain_to_array(int first, int *out, int max_out) {
    int count = 0;
    int c = first;
    while (c >= 0 && c < CLUSTER_COUNT && count < max_out) {
        out[count++] = c;
        if (FAT[c] == FAT_EOC) break;
        c = FAT[c];
    }
    return count;
}

static int alloc_chain(int n) {
    if (n <= 0) return -1;

    int *buf = (int*)malloc(sizeof(int) * n);
    if (!buf) return -1;

    int found = 0;
    for (int i = 0; i < CLUSTER_COUNT && found < n; ++i) {
        if (FAT[i] == FAT_FREE) buf[found++] = i;
    }
    if (found < n) { free(buf); return -1; }

    for (int i = 0; i < n - 1; ++i) FAT[buf[i]] = buf[i + 1];
    FAT[buf[n - 1]] = FAT_EOC;

    int first = buf[0];
    free(buf);
    return first;
}

static int append_clusters(int *pfirst, int n) {
    if (n <= 0) return 0;
    int new_first = alloc_chain(n);
    if (new_first < 0) return -1;

    if (*pfirst < 0) { *pfirst = new_first; return 0; }

    int c = *pfirst;
    while (FAT[c] != FAT_EOC) c = FAT[c];
    FAT[c] = new_first;
    return 0;
}

static void free_chain(int first) {
    int c = first;
    while (c >= 0 && c < CLUSTER_COUNT) {
        int next = FAT[c];
        FAT[c] = FAT_FREE;
        if (next == FAT_EOC) break;
        c = next;
    }
}

static int fs_create(const char *name, int bytes) {
    if (!g_formatted) { LOG_ERR("mkfs antes de usar."); return -1; }
    if (!name || !*name) { LOG_ERR("nome inválido."); return -1; }
    if (dir_find_by_name(name) >= 0) { LOG_ERR("arquivo já existe."); return -1; }

    int slot = dir_find_free_slot();
    if (slot < 0) { LOG_ERR("diretório cheio."); return -1; }

    int need  = clusters_needed(bytes);
    int first = alloc_chain(need);
    if (first < 0) { LOG_ERR("sem espaço em disco."); return -1; }

    ROOT[slot].in_use = 1;
    ROOT[slot].first  = first;
    ROOT[slot].size   = (bytes <= 0 ? 1 : bytes);
    strncpy(ROOT[slot].name, name, sizeof(ROOT[slot].name) - 1);
    ROOT[slot].name[sizeof(ROOT[slot].name) - 1] = '\0';

    int vec[CLUSTER_COUNT];
    int n = chain_to_array(first, vec, CLUSTER_COUNT);

    printf(C_GRN "[ ok ] " C_RESET "create '%s' %d bytes -> clusters: ",
           ROOT[slot].name, ROOT[slot].size);
    for (int i = 0; i < n; ++i) {
        printf(C_BLU "%d" C_RESET "%s", vec[i], (i+1<n?",":""));
    }
    printf("\n");
    return 0;
}

static int fs_append(const char *name, int add_bytes) {
    if (add_bytes <= 0) { LOG_WARN("nada a fazer."); return -1; }
    int i = dir_find_by_name(name);
    if (i < 0) { LOG_ERR("arquivo não encontrado."); return -1; }

    DirEntry *e = &ROOT[i];
    int old_clusters = clusters_needed(e->size);
    int new_size     = e->size + add_bytes;
    int new_clusters = clusters_needed(new_size);
    int delta        = new_clusters - old_clusters;

    if (delta > 0) {
        if (append_clusters(&e->first, delta) != 0) {
            LOG_ERR("sem espaço para append.");
            return -1;
        }
        LOG_INFO("append exigiu %d novo(s) cluster(s).", delta);
    } else {
        LOG_INFO("append coube no último cluster (sem novos clusters).");
    }

    e->size = new_size;

    int vec[CLUSTER_COUNT], n = chain_to_array(e->first, vec, CLUSTER_COUNT);
    printf(C_GRN "[ ok ] " C_RESET "append '%s' +%d bytes -> tamanho=%d; clusters: ",
           e->name, add_bytes, e->size);
    for (int k = 0; k < n; ++k) {
        printf(C_BLU "%d" C_RESET "%s", vec[k], (k+1<n?",":""));
    }
    printf("\n");
    return 0;
}

static int fs_delete(const char *name) {
    int i = dir_find_by_name(name);
    if (i < 0) { LOG_ERR("arquivo não encontrado."); return -1; }
    if (ROOT[i].first >= 0) free_chain(ROOT[i].first);
    LOG_OK("del '%s' — %d bytes liberados.", ROOT[i].name, ROOT[i].size);
    ROOT[i].in_use = 0;
    ROOT[i].first  = -1;
    ROOT[i].size   = 0;
    ROOT[i].name[0]= '\0';
    return 0;
}

/* Defrag didático: tenta realocar cada arquivo contiguamente a partir do cluster 2 */
static void fs_defrag(void) {
    int write = 2;

    for (int i = 0; i < DIR_CAPACITY; ++i) {
        if (!ROOT[i].in_use) continue;

        int old_vec[CLUSTER_COUNT];
        int n = chain_to_array(ROOT[i].first, old_vec, CLUSTER_COUNT);
        if (n == 0) continue;

        for (int k = 0; k < n; ++k) FAT[old_vec[k]] = FAT_FREE;

        int ok = 1;
        for (int k = 0; k < n; ++k) {
            if (write + k >= CLUSTER_COUNT || FAT[write + k] != FAT_FREE) { ok = 0; break; }
        }

        if (ok) {
            for (int k = 0; k < n - 1; ++k) FAT[write + k] = write + k + 1;
            FAT[write + (n - 1)] = FAT_EOC;
            ROOT[i].first = write;
            LOG_OK("arquivo '%s' realocado contiguamente [%d..%d].",
                  ROOT[i].name, write, write + n - 1);
            write += n;
        } else {
            int first = alloc_chain(n);
            if (first < 0) {
                LOG_ERR("defrag falhou por falta de espaço inesperada.");
                return;
            }
            ROOT[i].first = first;
            LOG_WARN("arquivo '%s' realocado de forma não contígua (fallback).", ROOT[i].name);
        }
    }
    LOG_INFO("defrag concluído. Use 'fat' e 'ls' para inspecionar.");
}

static void help(void) {
    puts(C_BOLD "Comandos:" C_RESET);
    puts("  mkfs                          -> formata/zera disco");
    puts("  create <nome> <bytes>         -> cria arquivo com tamanho inicial");
    puts("  append <nome> <bytes>         -> acrescenta bytes ao arquivo");
    puts("  del <nome>                    -> apaga arquivo");
    puts("  ls                            -> lista arquivos");
    puts("  fat                           -> imprime a FAT (entradas ativas)");
    puts("  defrag                        -> tentativa de desfragmentacao simples");
    puts("  help                          -> esta ajuda");
    puts("  exit                          -> sair");
}

/* ===== Shell ===== */
int main(void) {
    enable_vt_mode();
    setbuf(stdout, NULL);
    srand((unsigned)time(NULL));

    printf(C_BOLD "FAT Simulator (didático) " C_RESET "— clusters=%d, cluster_size=%d bytes\n",
           CLUSTER_COUNT, CLUSTER_SIZE);
    printf("Digite '" C_BOLD "help" C_RESET "' para comandos.\n");

    char line[256], cmd[32], a1[64], a2[64];

    while (1) {
        printf(C_DIM "> " C_RESET);
        if (!fgets(line, sizeof(line), stdin)) break;

        cmd[0]=a1[0]=a2[0]='\0';
        int n = sscanf(line, "%31s %63s %63s", cmd, a1, a2);
        if (n <= 0) continue;

        if      (strcasecmp(cmd, "exit")   == 0 || strcasecmp(cmd, "quit")==0) break;
        else if (strcasecmp(cmd, "help")   == 0) help();
        else if (strcasecmp(cmd, "mkfs")   == 0) mkfs();
        else if (strcasecmp(cmd, "ls")     == 0) fs_ls();
        else if (strcasecmp(cmd, "fat")    == 0) print_fat();
        else if (strcasecmp(cmd, "defrag") == 0) fs_defrag();
        else if (strcasecmp(cmd, "create") == 0) {
            if (n < 3) { LOG_ERR("uso: create <nome> <bytes>"); continue; }
            int bytes = atoi(a2);
            fs_create(a1, bytes);
        }
        else if (strcasecmp(cmd, "append") == 0) {
            if (n < 3) { LOG_ERR("uso: append <nome> <bytes>"); continue; }
            int bytes = atoi(a2);
            fs_append(a1, bytes);
        }
        else if (strcasecmp(cmd, "del") == 0 || strcasecmp(cmd, "rm") == 0) {
            if (n < 2) { LOG_ERR("uso: del <nome>"); continue; }
            fs_delete(a1);
        }
        else {
            LOG_ERR("comando não reconhecido. Use 'help'.");
        }
    }

    puts("bye!");
    return 0;
}
