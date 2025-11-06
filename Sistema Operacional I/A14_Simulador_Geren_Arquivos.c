#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>
#include <ctype.h>

/* ======================
   CONFIGURAÇÃO DE CORES
   ====================== */
#define USE_COLOR 1
#if USE_COLOR
#define C_OK    "\x1b[32m"
#define C_ERR   "\x1b[31m"
#define C_WARN  "\x1b[33m"
#define C_INFO  "\x1b[36m"
#define C_RESET "\x1b[0m"
#else
#define C_OK    ""
#define C_ERR   ""
#define C_WARN  ""
#define C_INFO  ""
#define C_RESET ""
#endif

/* ======================
   PERMISSÕES ESTILO UNIX
   owner/group/others: rwx
   bitmask: usamos 9 bits
   ====================== */
enum {
    O_R = 1<<8, O_W = 1<<7, O_X = 1<<6,   // owner
    G_R = 1<<5, G_W = 1<<4, G_X = 1<<3,   // group
    A_R = 1<<2, A_W = 1<<1, A_X = 1<<0    // others (all)
};

typedef enum { REQ_READ, REQ_WRITE, REQ_EXEC } Req;

/* ======================
   MODELOS DE USUÁRIO/GRUPO
   ====================== */
typedef struct {
    char name[16];
    char group[16];
} User;

typedef struct {
    char name[32];
    char owner[16];
    char group[16];
    int  mode; // 9 bits rwx
} FileMeta;

/* ================
   ACL (simples)
   até 8 entradas
   ================ */
typedef struct {
    char username[16];
    int allow_r; int allow_w; int allow_x; // 1 = concede
    int deny_r;  int deny_w;  int deny_x;  // 1 = nega
} AclEntry;

typedef struct {
    char filename[32];
    AclEntry entries[8];
    int count;
} FileACL;

/* ================
   REGISTROS DE DADOS
   ================ */
typedef struct {
    int   id;
    char  nome[24];
    float nota;
} Registro;

/* índice simples: ID -> posição (offset em registros, não em bytes) */
typedef struct {
    int id;
    long pos; // índice do registro (0..N-1)
} IndexEntry;

/* ======================
   "BANCO" DE EXEMPLO
   ====================== */
static User USERS[] = {
    {"william", "prof"},
    {"ana",     "alunos"},
    {"carlos",  "alunos"},
    {"root",    "admins"},
};
static const int NUSERS = sizeof(USERS)/sizeof(USERS[0]);

/* Um "arquivo" que vamos proteger */
static FileMeta DATA_META = {
    .name  = "registros.dat",
    .owner = "william",
    .group = "prof",
    /*  rwx rwx rwx
        7   4   0    => 740
        owner: rwx, group: r--, others: ---
    */
    .mode  = (O_R|O_W|O_X) | (G_R) | (0)
};

/* ACL acoplada ao arquivo de dados */
static FileACL DATA_ACL = {
    .filename = "registros.dat",
    .count = 0
};

/* ================
   UTILIDADES
   ================ */
static int octal_to_mode(int oct) {
    int o = (oct/100)%10, g = (oct/10)%10, a = oct%10;
    int m = 0;
    // owner
    if (o & 4) m |= O_R;
    if (o & 2) m |= O_W;
    if (o & 1) m |= O_X;
    // group
    if (g & 4) m |= G_R;
    if (g & 2) m |= G_W;
    if (g & 1) m |= G_X;
    // others
    if (a & 4) m |= A_R;
    if (a & 2) m |= A_W;
    if (a & 1) m |= A_X;
    return m;
}

static void mode_to_string(int mode, char out[11], char ftype) {
    // ftype: '-' arquivo, 'd' dir (aqui sempre '-')
    out[0] = ftype;
    out[1] = (mode & O_R) ? 'r' : '-';
    out[2] = (mode & O_W) ? 'w' : '-';
    out[3] = (mode & O_X) ? 'x' : '-';
    out[4] = (mode & G_R) ? 'r' : '-';
    out[5] = (mode & G_W) ? 'w' : '-';
    out[6] = (mode & G_X) ? 'x' : '-';
    out[7] = (mode & A_R) ? 'r' : '-';
    out[8] = (mode & A_W) ? 'w' : '-';
    out[9] = (mode & A_X) ? 'x' : '-';
    out[10]= '\0';
}

/* Busca usuário pelo nome */
static User* find_user(const char *name) {
    for (int i=0;i<NUSERS;i++) if (strcmp(USERS[i].name, name)==0) return &USERS[i];
    return NULL;
}

/* ACL: encontra entrada por usuário */
static AclEntry* acl_find(FileACL *acl, const char *username) {
    for (int i=0;i<acl->count;i++) if (strcmp(acl->entries[i].username, username)==0) return &acl->entries[i];
    return NULL;
}

/* ACL: adiciona/atualiza */
static void acl_set(FileACL *acl, const char *username, int ar,int aw,int ax,int dr,int dw,int dx) {
    AclEntry *e = acl_find(acl, username);
    if (!e) {
        if (acl->count >= 8) { printf(C_ERR "[ACL] Limite atingido.\n" C_RESET); return; }
        e = &acl->entries[acl->count++];
        memset(e, 0, sizeof(*e));
        strncpy(e->username, username, sizeof(e->username)-1);
    }
    e->allow_r = ar; e->allow_w = aw; e->allow_x = ax;
    e->deny_r  = dr; e->deny_w  = dw; e->deny_x  = dx;
    printf(C_INFO "[ACL] Atualizada para %s (allow r%d w%d x%d / deny r%d w%d x%d)\n" C_RESET,
           username, ar,aw,ax, dr,dw,dx);
}

/* ACL: remove */
static void acl_remove(FileACL *acl, const char *username) {
    for (int i=0;i<acl->count;i++) {
        if (strcmp(acl->entries[i].username, username)==0) {
            for (int j=i+1;j<acl->count;j++) acl->entries[j-1] = acl->entries[j];
            acl->count--;
            printf(C_INFO "[ACL] Removida para %s\n" C_RESET, username);
            return;
        }
    }
    printf(C_WARN "[ACL] Usuário não encontrado.\n" C_RESET);
}

/* Checagem de permissão: retorna 1 se permitido, 0 caso contrário
   Ordem: ACL deny > ACL allow > owner > group > others
*/
static int has_perm(const User *u, const FileMeta *fm, const FileACL *acl, Req req) {
    // Mapeia Req para bits
    int obit=0, gbit=0, abit=0;
    if      (req==REQ_READ)  { obit=O_R; gbit=G_R; abit=A_R; }
    else if (req==REQ_WRITE) { obit=O_W; gbit=G_W; abit=A_W; }
    else                     { obit=O_X; gbit=G_X; abit=A_X; }

    // 1) ACL
    const AclEntry *e = acl_find((FileACL*)acl, u->name);
    if (e) {
        int deny = (req==REQ_READ? e->deny_r : (req==REQ_WRITE? e->deny_w : e->deny_x));
        int allow= (req==REQ_READ? e->allow_r: (req==REQ_WRITE? e->allow_w: e->allow_x));
        if (deny)  return 0;
        if (allow) return 1;
    }
    // 2) owner
    if (strcmp(u->name, fm->owner)==0) return (fm->mode & obit) ? 1 : 0;
    // 3) group
    if (strcmp(u->group, fm->group)==0) return (fm->mode & gbit) ? 1 : 0;
    // 4) others
    return (fm->mode & abit) ? 1 : 0;
}

/* Log amigável */
static const char* req_str(Req r){
    return r==REQ_READ?"ler":(r==REQ_WRITE?"escrever":"executar");
}

/* ======================
   DADOS: criação/semente
   ====================== */
static const Registro SEED[] = {
    {101, "Alice",   8.5f},
    {102, "Bruno",   7.0f},
    {103, "Carla",   9.2f},
    {104, "Diego",   6.7f},
    {105, "Eva",     8.9f},
    {106, "Fernando",5.5f},
    {107, "Gabi",    7.8f},
    {108, "Heitor",  9.5f}
};

#define NREG ( (int)(sizeof(SEED)/sizeof(SEED[0])) )

/* Índice (ID -> pos) */
static IndexEntry INDEX_TBL[NREG];

static int file_exists(const char* p) {
    FILE* f = fopen(p, "rb");
    if (!f) return 0;
    fclose(f);
    return 1;
}

static void seed_file(const char *path) {
    FILE* f = fopen(path, "wb");
    if (!f) { printf(C_ERR "Falha ao criar %s\n" C_RESET, path); exit(1); }
    fwrite(SEED, sizeof(Registro), NREG, f);
    fclose(f);
    printf(C_OK "[OK] Arquivo de dados semeado com %d registros.\n" C_RESET, NREG);
}

static void build_index(void) {
    for (int i=0;i<NREG;i++){ INDEX_TBL[i].id = SEED[i].id; INDEX_TBL[i].pos = i; }
}

/* Busca posição por ID no índice (linear simples para didática) */
static long index_find_pos(int id) {
    for (int i=0;i<NREG;i++) if (INDEX_TBL[i].id==id) return INDEX_TBL[i].pos;
    return -1;
}

/* ======================
   OPERAÇÕES DE ACESSO
   ====================== */
static void show_meta(const FileMeta *fm) {
    char buf[11]; mode_to_string(fm->mode, buf, '-');
    printf(C_INFO "%s %s:%s  %s\n" C_RESET, buf, fm->owner, fm->group, fm->name);
}

static void show_acl(const FileACL *acl) {
    if (acl->count==0) { printf("(sem ACLs específicas)\n"); return; }
    for (int i=0;i<acl->count;i++){
        const AclEntry* e=&acl->entries[i];
        printf(" - %-10s allow(r%d w%d x%d) deny(r%d w%d x%d)\n",
               e->username, e->allow_r,e->allow_w,e->allow_x, e->deny_r,e->deny_w,e->deny_x);
    }
}

/* Leitura sequencial */
static void read_sequential(const User *u, const FileMeta *fm, const FileACL *acl) {
    printf(C_INFO "[SEQUENCIAL] Usuário '%s' tenta %s %s\n" C_RESET, u->name, req_str(REQ_READ), fm->name);
    if (!has_perm(u,fm,acl,REQ_READ)) { printf(C_ERR "Acesso NEGADO (read)\n" C_RESET); return; }

    FILE* f = fopen(fm->name, "rb");
    if (!f) { printf(C_ERR "Falha ao abrir.\n" C_RESET); return; }
    Registro r;
    int i=0;
    while (fread(&r,sizeof(r),1,f)==1) {
        printf(C_OK "#%d  ID:%d  Nome:%-10s Nota:%.1f\n" C_RESET, i, r.id, r.nome, r.nota);
        i++;
    }
    fclose(f);
}

/* Acesso direto: por posição (0..N-1) */
static void read_direct_pos(const User *u, const FileMeta *fm, const FileACL *acl, long pos) {
    printf(C_INFO "[DIRETO] Usuário '%s' tenta %s pos=%ld de %s\n" C_RESET, u->name, req_str(REQ_READ), pos, fm->name);
    if (!has_perm(u,fm,acl,REQ_READ)) { printf(C_ERR "Acesso NEGADO (read)\n" C_RESET); return; }
    if (pos<0 || pos>=NREG) { printf(C_WARN "Posição inválida.\n" C_RESET); return; }

    FILE* f = fopen(fm->name, "rb");
    if (!f) { printf(C_ERR "Falha ao abrir.\n" C_RESET); return; }
    fseek(f, pos*sizeof(Registro), SEEK_SET);
    Registro r;
    if (fread(&r,sizeof(r),1,f)==1) {
        printf(C_OK "[pos=%ld] ID:%d Nome:%s Nota:%.1f\n" C_RESET, pos, r.id, r.nome, r.nota);
    } else {
        printf(C_WARN "Leitura falhou.\n" C_RESET);
    }
    fclose(f);
}

/* Acesso indexado: por ID -> consulta índice -> fseek */
static void read_indexed_id(const User *u, const FileMeta *fm, const FileACL *acl, int id) {
    printf(C_INFO "[INDEXADO] Usuário '%s' tenta %s ID=%d em %s\n" C_RESET, u->name, req_str(REQ_READ), id, fm->name);
    if (!has_perm(u,fm,acl,REQ_READ)) { printf(C_ERR "Acesso NEGADO (read)\n" C_RESET); return; }

    long pos = index_find_pos(id);
    if (pos<0){ printf(C_WARN "ID não encontrado no índice.\n" C_RESET); return; }

    FILE* f = fopen(fm->name, "rb");
    if (!f) { printf(C_ERR "Falha ao abrir.\n" C_RESET); return; }
    fseek(f, pos*sizeof(Registro), SEEK_SET);
    Registro r;
    if (fread(&r,sizeof(r),1,f)==1) {
        printf(C_OK "[id=%d pos=%ld] Nome:%s Nota:%.1f\n" C_RESET, r.id, pos, r.nome, r.nota);
    } else {
        printf(C_WARN "Leitura falhou.\n" C_RESET);
    }
    fclose(f);
}

/* Escrita direta (editar nota) – exige WRITE */
static void write_update_nota(const User *u, const FileMeta *fm, const FileACL *acl, int id, float nova) {
    printf(C_INFO "[WRITE] Usuário '%s' tenta %s (id=%d) em %s\n" C_RESET, u->name, req_str(REQ_WRITE), id, fm->name);
    if (!has_perm(u,fm,acl,REQ_WRITE)) { printf(C_ERR "Acesso NEGADO (write)\n" C_RESET); return; }

    long pos = index_find_pos(id);
    if (pos<0){ printf(C_WARN "ID não encontrado.\n" C_RESET); return; }

    FILE* f = fopen(fm->name, "r+b");
    if (!f) { printf(C_ERR "Falha ao abrir.\n" C_RESET); return; }

    fseek(f, pos*sizeof(Registro), SEEK_SET);
    Registro r;
    if (fread(&r,sizeof(r),1,f)!=1){ printf(C_ERR "Falha leitura.\n" C_RESET); fclose(f); return; }

    printf(" Antes: ID:%d Nome:%s Nota:%.1f\n", r.id, r.nome, r.nota);
    r.nota = nova;

    fseek(f, pos*sizeof(Registro), SEEK_SET);
    if (fwrite(&r,sizeof(r),1,f)==1) {
        printf(C_OK " Depois: ID:%d Nome:%s Nota:%.1f (SALVO)\n" C_RESET, r.id, r.nome, r.nota);
    } else {
        printf(C_ERR "Falha na escrita.\n" C_RESET);
    }
    fclose(f);
}

/* ======================
   MENU / INTERAÇÃO
   ====================== */
static void menu_help(void) {
    printf(C_INFO "\n--- MENU ---\n" C_RESET
           "1) Mostrar meta/permissões\n"
           "2) Mostrar ACLs\n"
           "3) Leitura SEQUENCIAL\n"
           "4) Leitura DIRETA por posição (0..%d)\n"
           "5) Leitura INDEXADA por ID\n"
           "6) Alterar nota (WRITE) por ID\n"
           "7) chmod (octal, ex: 740)\n"
           "8) chown (owner group)\n"
           "9) ACL set (usuario allow[rwx] deny[rwx])\n"
           "10) ACL remove (usuario)\n"
           "0) Sair\n", NREG-1);
}

/* Lê string segura */
static void read_token(char *buf, int max) {
    if (fgets(buf, max, stdin)==NULL){ buf[0]='\0'; return; }
    buf[strcspn(buf, "\r\n")] = 0;
}

/* ======================
   MAIN
   ====================== */
int main(void) {
    system("cls");
    setlocale(LC_ALL,"pt_BR.UTF-8");

    /* prepara dados */
    if (!file_exists(DATA_META.name)) seed_file(DATA_META.name);
    build_index();

    printf(C_INFO "=== Simulador de Métodos de Acesso + Permissões (SO I) ===\n" C_RESET);
    printf("Usuários disponíveis:\n");
    for (int i=0;i<NUSERS;i++) printf(" - %s (grupo=%s)\n", USERS[i].name, USERS[i].group);

    char userbuf[32];
    printf(C_INFO "Faça login como: " C_RESET);
    read_token(userbuf, sizeof(userbuf));
    User *U = find_user(userbuf);
    if (!U) { printf(C_ERR "Usuário inválido.\n" C_RESET); return 0; }
    printf(C_OK "Logado como %s (grupo=%s)\n" C_RESET, U->name, U->group);

    int op;
    do {
        menu_help();
        printf(C_INFO "Opção: " C_RESET);
        if (scanf("%d", &op)!=1){ op=0; }
        while (getchar()!='\n'); // limpa buffer

        if (op==1) {
            show_meta(&DATA_META);
        } else if (op==2) {
            show_acl(&DATA_ACL);
        } else if (op==3) {
            read_sequential(U, &DATA_META, &DATA_ACL);
        } else if (op==4) {
            long pos; printf("Posição (0..%d): ", NREG-1);
            scanf("%ld",&pos); while (getchar()!='\n');
            read_direct_pos(U, &DATA_META, &DATA_ACL, pos);
        } else if (op==5) {
            int id; printf("ID (ex: 101..108): ");
            scanf("%d",&id); while (getchar()!='\n');
            read_indexed_id(U, &DATA_META, &DATA_ACL, id);
        } else if (op==6) {
            int id; float nova;
            printf("ID: "); scanf("%d",&id);
            printf("Nova nota: "); scanf("%f",&nova);
            while (getchar()!='\n');
            write_update_nota(U,&DATA_META,&DATA_ACL,id,nova);
        } else if (op==7) {
            int oct; printf("novo modo (octal, ex 740): ");
            scanf("%d",&oct); while (getchar()!='\n');
            DATA_META.mode = octal_to_mode(oct);
            printf(C_OK "chmod aplicado. "); show_meta(&DATA_META);
        } else if (op==8) {
            char own[16], grp[16];
            printf("novo owner: "); read_token(own,sizeof(own));
            printf("novo group: "); read_token(grp,sizeof(grp));
            strncpy(DATA_META.owner, own, sizeof(DATA_META.owner)-1);
            strncpy(DATA_META.group, grp, sizeof(DATA_META.group)-1);
            printf(C_OK "chown aplicado. "); show_meta(&DATA_META);
        } else if (op==9) {
            char u[16]; char allow[4]; char deny[4];
            printf("usuario: "); read_token(u,sizeof(u));
            printf("allow (ex: rw, r, x, rwx, -): "); read_token(allow,sizeof(allow));
            printf("deny  (ex: r, w, x, rwx, -): ");  read_token(deny,sizeof(deny));
            int ar=0,aw=0,ax=0, dr=0,dw=0,dx=0;
            for (int i=0;i<(int)strlen(allow);i++){
                if (allow[i]=='r') ar=1; else if (allow[i]=='w') aw=1; else if (allow[i]=='x') ax=1;
            }
            for (int i=0;i<(int)strlen(deny);i++){
                if (deny[i]=='r') dr=1; else if (deny[i]=='w') dw=1; else if (deny[i]=='x') dx=1;
            }
            acl_set(&DATA_ACL, u, ar,aw,ax, dr,dw,dx);
        } else if (op==10) {
            char u[16]; printf("usuario: "); read_token(u,sizeof(u));
            acl_remove(&DATA_ACL, u);
        } else if (op==0) {
            puts("Saindo...");
        } else {
            puts("Opção inválida.");
        }
    } while (op!=0);

    return 0;
}
