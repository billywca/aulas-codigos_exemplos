/*  fat_sim.c — Simulador didático de Tabela de Alocação de Arquivos (FAT)
    Autor: você :)
    Objetivo: demonstrar como a FAT encadeia clusters, como criar/apagar arquivos,
              e como a fragmentação acontece.

    Modelo simplificado:
    - Disco dividido em CLUSTERS de tamanho fixo (CLUSTER_SIZE).
    - FAT[cluster] guarda o próximo cluster da corrente; EOC (-1) indica fim de arquivo.
    - 0 significa cluster livre.
    - Diretório raiz simples (lista fixa de entradas).
    - Operações: mkfs, create <nome> <bytes>, append <nome> <bytes>, del <nome>, ls, fat, help, exit
*/

#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

/* ==== Parametrização do "disco" ==== */
#define CLUSTER_COUNT  256          /* quantidade total de clusters do disco */
#define CLUSTER_SIZE   1024         /* tamanho de cada cluster em bytes      */
#define DIR_CAPACITY   64           /* entradas máximas do diretório raiz    */

#define FAT_FREE       0            /* cluster livre                         */
#define FAT_EOC       -1            /* end-of-chain                          */

/* ==== Estruturas ==== */
typedef struct {
    char  name[32];     /* nome do arquivo (simplificado, sem 8.3) */
    int   size;         /* tamanho em bytes                         */
    int   first;        /* primeiro cluster da corrente (ou -1)     */
    int   in_use;       /* 0 livre | 1 usado                        */
} DirEntry;

/* ==== "Disco" em memória ==== */
static int      FAT[CLUSTER_COUNT];          /* tabela de alocação           */
static DirEntry ROOT[DIR_CAPACITY];          /* diretório raiz               */
static int      g_formatted = 0;             /* flag: está formatado?        */

/* ==== Utilidades ==== */

/* arredonda para cima: quantos clusters são necessários p/ 'bytes' */
static int clusters_needed(int bytes) {
    if (bytes <= 0) return 0;
    int n = bytes / CLUSTER_SIZE;
    if (bytes % CLUSTER_SIZE) n++;
    if (n == 0) n = 1;
    return n;
}

/* zera FAT e diretório */
static void mkfs(void) {
    for (int i = 0; i < CLUSTER_COUNT; ++i) FAT[i] = FAT_FREE;
    for (int i = 0; i < DIR_CAPACITY; ++i) {
        ROOT[i].in_use = 0;
        ROOT[i].first  = -1;
        ROOT[i].size   = 0;
        ROOT[i].name[0]= '\0';
    }
    g_formatted = 1;
    printf("[mkfs] Disco inicializado: %d clusters de %d bytes (%.1f KiB).\n",
           CLUSTER_COUNT, CLUSTER_SIZE,
           (CLUSTER_COUNT * CLUSTER_SIZE) / 1024.0);
}

/* procura um slot livre no diretório */
static int dir_find_free_slot(void) {
    for (int i = 0; i < DIR_CAPACITY; ++i) if (!ROOT[i].in_use) return i;
    return -1;
}

/* procura arquivo por nome (case-insensitive) */
static int dir_find_by_name(const char *name) {
    for (int i = 0; i < DIR_CAPACITY; ++i) {
        if (ROOT[i].in_use) {
            if (strcasecmp(ROOT[i].name, name) == 0) return i;
        }
    }
    return -1;
}

/* imprime lista de arquivos (nome, tamanho, 1º cluster) */
static void fs_ls(void) {
    int count = 0;
    printf("Nome                           Tamanho(B)   1ºCluster\n");
    printf("-----------------------------------------------------\n");
    for (int i = 0; i < DIR_CAPACITY; ++i) {
        if (ROOT[i].in_use) {
            printf("%-30s %10d   %d\n", ROOT[i].name, ROOT[i].size, ROOT[i].first);
            count++;
        }
    }
    if (count == 0) puts("(diretório vazio)");
}

/* imprime FAT (índice -> valor) em blocos para visualização */
static void print_fat(void) {
    printf("FAT (cluster: next)\n");
    for (int i = 0; i < CLUSTER_COUNT; ++i) {
        if (FAT[i] == FAT_FREE) {
            /* para reduzir saída, mostramos só linhas “ativas” e alguns livres */
            continue;
        }
        printf("[%3d] -> %d\n", i, FAT[i]);
    }
    /* estatística de livres/ocupados */
    int freec = 0, usedc = 0;
    for (int i = 0; i < CLUSTER_COUNT; ++i) {
        if (FAT[i] == FAT_FREE) freec++; else usedc++;
    }
    printf("Clusters: total=%d, livres=%d, usados=%d\n", CLUSTER_COUNT, freec, usedc);
}

/* obtém tamanho (em clusters) da corrente a partir de 'first' */
static int chain_length(int first) {
    if (first < 0) return 0;
    int len = 0;
    int c = first;
    while (c >= 0 && c < CLUSTER_COUNT) {
        len++;
        if (FAT[c] == FAT_EOC) break;
        c = FAT[c];
    }
    return len;
}

/* percorre corrente e grava índices em 'out' (até max_out). retorna n preenchidos */
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

/* aloca N clusters livres (first-fit), encadeia na FAT e retorna o 1º cluster, ou -1 */
static int alloc_chain(int n) {
    if (n <= 0) return -1;

    /* 1) colete índices livres suficientes */
    int *buf = (int*)malloc(sizeof(int) * n);
    if (!buf) return -1;

    int found = 0;
    for (int i = 0; i < CLUSTER_COUNT && found < n; ++i) {
        if (FAT[i] == FAT_FREE) buf[found++] = i;
    }
    if (found < n) {
        free(buf);
        return -1; /* sem espaço */
    }

    /* 2) encadeie: c0->c1->...->c(n-1)->EOC */
    for (int i = 0; i < n - 1; ++i) {
        FAT[buf[i]] = buf[i + 1];
    }
    FAT[buf[n - 1]] = FAT_EOC;

    int first = buf[0];
    free(buf);
    return first;
}

/* anexa 'n' clusters ao final de uma corrente existente (retorna 0 ok / -1 erro) */
static int append_clusters(int *pfirst, int n) {
    if (n <= 0) return 0;
    int new_first = alloc_chain(n);
    if (new_first < 0) return -1;

    /* se arquivo ainda não tinha corrente: vira a corrente */
    if (*pfirst < 0) {
        *pfirst = new_first;
        return 0;
    }
    /* caso contrário, navegue até o último e encadeie a nova corrente */
    int c = *pfirst;
    while (FAT[c] != FAT_EOC) c = FAT[c];
    FAT[c] = new_first;
    return 0;
}

/* libera todos os clusters a partir de 'first' */
static void free_chain(int first) {
    int c = first;
    while (c >= 0 && c < CLUSTER_COUNT) {
        int next = FAT[c];
        FAT[c] = FAT_FREE;
        if (next == FAT_EOC) break;
        c = next;
    }
}

/* cria arquivo com 'bytes' (aloca clusters) */
static int fs_create(const char *name, int bytes) {
    if (!g_formatted) { puts("Erro: mkfs antes de usar."); return -1; }
    if (!name || !*name) { puts("Erro: nome inválido."); return -1; }
    if (dir_find_by_name(name) >= 0) { puts("Erro: arquivo já existe."); return -1; }

    int slot = dir_find_free_slot();
    if (slot < 0) { puts("Erro: diretório cheio."); return -1; }

    int need = clusters_needed(bytes <= 0 ? 1 : bytes);
    int first = alloc_chain(need);
    if (first < 0) { puts("Erro: sem espaço em disco."); return -1; }

    /* grava entrada no diretório */
    ROOT[slot].in_use = 1;
    ROOT[slot].first  = first;
    ROOT[slot].size   = (bytes <= 0 ? 1 : bytes);
    strncpy(ROOT[slot].name, name, sizeof(ROOT[slot].name) - 1);
    ROOT[slot].name[sizeof(ROOT[slot].name) - 1] = '\0';

    /* log */
    int vec[CLUSTER_COUNT];
    int n = chain_to_array(first, vec, CLUSTER_COUNT);
    printf("[create] '%s' %d bytes -> clusters: ", ROOT[slot].name, ROOT[slot].size);
    for (int i = 0; i < n; ++i) printf("%d%s", vec[i], (i+1<n?",":""));
    printf("\n");
    return 0;
}

/* aumenta arquivo 'name' em 'add_bytes' (pode exigir novos clusters) */
static int fs_append(const char *name, int add_bytes) {
    if (add_bytes <= 0) { puts("Nada a fazer."); return -1; }
    int i = dir_find_by_name(name);
    if (i < 0) { puts("Arquivo não encontrado."); return -1; }

    DirEntry *e = &ROOT[i];

    /* clusters atuais e novos necessários após o crescimento */
    int old_clusters = clusters_needed(e->size);
    int new_size     = e->size + add_bytes;
    int new_clusters = clusters_needed(new_size);

    int delta = new_clusters - old_clusters;
    if (delta > 0) {
        if (append_clusters(&e->first, delta) != 0) {
            puts("Sem espaço para append.");
            return -1;
        }
    }

    e->size = new_size;

    /* log */
    int vec[CLUSTER_COUNT];
    int n = chain_to_array(e->first, vec, CLUSTER_COUNT);
    printf("[append] '%s' +%d bytes -> novo tamanho=%d; clusters: ",
           e->name, add_bytes, e->size);
    for (int k = 0; k < n; ++k) printf("%d%s", vec[k], (k+1<n?",":""));
    printf("\n");
    return 0;
}

/* apaga arquivo (libera corrente e slot do diretório) */
static int fs_delete(const char *name) {
    int i = dir_find_by_name(name);
    if (i < 0) { puts("Arquivo não encontrado."); return -1; }
    if (ROOT[i].first >= 0) free_chain(ROOT[i].first);
    printf("[del] '%s' removido. %d bytes liberados.\n", ROOT[i].name, ROOT[i].size);
    ROOT[i].in_use = 0;
    ROOT[i].first  = -1;
    ROOT[i].size   = 0;
    ROOT[i].name[0]= '\0';
    return 0;
}

/* tentativa de “defrag” simples: reembala correntes a partir do cluster 2
   (1) varre diretório em ordem, (2) recolhe todos os clusters do arquivo,
   (3) realoca blocos contíguos desde 'write', ajusta FAT e encadeia contiguamente.
   ATENÇÃO: é um defrag didático, não robusto como um real! */
static void fs_defrag(void) {
    int write = 2; /* reserve 0 e 1 só para mostrar que podem ter áreas “sist.” */

    for (int i = 0; i < DIR_CAPACITY; ++i) {
        if (!ROOT[i].in_use) continue;

        /* copie corrente antiga para vetor */
        int old_vec[CLUSTER_COUNT];
        int n = chain_to_array(ROOT[i].first, old_vec, CLUSTER_COUNT);
        if (n == 0) continue;

        /* libere corrente antiga na FAT (ficarão livres antes de recolocar) */
        for (int k = 0; k < n; ++k) FAT[old_vec[k]] = FAT_FREE;

        /* realoque n clusters contíguos a partir de 'write' (se possível) */
        int ok = 1;
        for (int k = 0; k < n; ++k) {
            if (write + k >= CLUSTER_COUNT || FAT[write + k] != FAT_FREE) { ok = 0; break; }
        }

        if (ok) {
            /* encadeie contíguo: write -> write+1 -> ... -> EOC */
            for (int k = 0; k < n - 1; ++k) FAT[write + k] = write + k + 1;
            FAT[write + (n - 1)] = FAT_EOC;
            ROOT[i].first = write;
            write += n;
        } else {
            /* fallback: se não couber contíguo, apenas realoque onde houver espaço */
            int first = alloc_chain(n);
            if (first < 0) {
                puts("[defrag] Falhou por falta de espaço inesperada.");
                return;
            }
            ROOT[i].first = first;
        }
    }
    puts("[defrag] Feito (didático). Verifique 'fat' e 'ls'.");
}

/* ajuda */
static void help(void) {
    puts("Comandos:");
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

/* ==== Shell simples ==== */
int main(void) {
    system("cls");
    setlocale(LC_ALL,"pt_BR.UTF-8");
    
    setbuf(stdout, NULL);
    srand((unsigned)time(NULL));

    printf("FAT Simulator (didático) — clusters=%d, cluster_size=%d bytes\n",
           CLUSTER_COUNT, CLUSTER_SIZE);
    printf("Digite 'help' para comandos.\n");

    char line[256];
    char cmd[32], a1[64], a2[64];

    /* comece sem formatar para reforçar a necessidade do mkfs */
    while (1) {
        printf("> ");
        if (!fgets(line, sizeof(line), stdin)) break;

        /* reseta tokens */
        cmd[0]=a1[0]=a2[0]='\0';

        /* parse básico: cmd arg1 arg2 */
        int n = sscanf(line, "%31s %63s %63s", cmd, a1, a2);
        if (n <= 0) continue;

        if (strcasecmp(cmd, "exit") == 0 || strcasecmp(cmd, "quit") == 0) break;
        else if (strcasecmp(cmd, "help") == 0) help();
        else if (strcasecmp(cmd, "mkfs") == 0) mkfs();
        else if (strcasecmp(cmd, "ls") == 0) fs_ls();
        else if (strcasecmp(cmd, "fat") == 0) print_fat();
        else if (strcasecmp(cmd, "defrag") == 0) fs_defrag();
        else if (strcasecmp(cmd, "create") == 0) {
            if (n < 3) { puts("Uso: create <nome> <bytes>"); continue; }
            int bytes = atoi(a2);
            fs_create(a1, bytes);
        }
        else if (strcasecmp(cmd, "append") == 0) {
            if (n < 3) { puts("Uso: append <nome> <bytes>"); continue; }
            int bytes = atoi(a2);
            fs_append(a1, bytes);
        }
        else if (strcasecmp(cmd, "del") == 0 || strcasecmp(cmd, "rm") == 0) {
            if (n < 2) { puts("Uso: del <nome>"); continue; }
            fs_delete(a1);
        }
        else {
            puts("Comando não reconhecido. Use 'help'.");
        }
    }

    puts("bye!");
    return 0;
}
