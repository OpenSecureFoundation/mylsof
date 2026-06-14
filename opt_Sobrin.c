

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "options.h"

/* Sobrin écrit ton code ici — options : -t -a +D -o -K */
}

}
/* ================================================================
**  opt_Sobrin.c — Options : -l  -h  -o  -z
**  Projet : mylsof
**  Auteur : Sobrin
** ================================================================ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/stat.h>
#include "options.h"

/* ----------------------------------------------------------------
**  UTILITAIRE : lire le nom du processus dans /proc/PID/comm
** ---------------------------------------------------------------- */
static void get_comm(int pid, char *buf, size_t size)
{
    char  path[64];
    FILE *f;

    snprintf(path, sizeof(path), "/proc/%d/comm", pid);
    f = fopen(path, "r");
    if (!f) { snprintf(buf, size, "?"); return; }
    if (fgets(buf, (int)size, f))
        buf[strcspn(buf, "\n")] = '\0';
    else
        snprintf(buf, size, "?");
    fclose(f);
}

/* ----------------------------------------------------------------
**  UTILITAIRE : lire l'UID réel du processus dans /proc/PID/status
** ---------------------------------------------------------------- */
static int get_uid(int pid)
{
    char  path[64];
    char  line[256];
    FILE *f;
    int   uid = -1;

    snprintf(path, sizeof(path), "/proc/%d/status", pid);
    f = fopen(path, "r");
    if (!f) return (-1);
    while (fgets(line, sizeof(line), f))
        if (strncmp(line, "Uid:", 4) == 0)
        { sscanf(line + 4, "%d", &uid); break; }
    fclose(f);
    return (uid);
}

/* ----------------------------------------------------------------
**  UTILITAIRE : lire l'offset d'un fd dans /proc/PID/fdinfo/FD
**  L'offset = position actuelle du curseur de lecture (champ pos:)
** ---------------------------------------------------------------- */
static long long get_offset(int pid, int fd)
{
    char       path[128];
    char       line[128];
    FILE      *f;
    long long  offset = -1;

    snprintf(path, sizeof(path), "/proc/%d/fdinfo/%d", pid, fd);
    f = fopen(path, "r");
    if (!f) return (-1);
    while (fgets(line, sizeof(line), f))
        if (strncmp(line, "pos:", 4) == 0)
        { sscanf(line + 4, "%lld", &offset); break; }
    fclose(f);
    return (offset);
}

/* ----------------------------------------------------------------
**  UTILITAIRE : lire le namespace PID dans /proc/PID/ns/pid
**  Le lien symbolique ressemble à : pid:[4026531836]
**  On extrait le numéro d'inode → identifiant unique de la zone
** ---------------------------------------------------------------- */
static void get_zone(int pid, char *buf, size_t size)
{
    char    path[128];
    char    link[256];
    ssize_t len;
    char   *start;
    char   *end;

    snprintf(path, sizeof(path), "/proc/%d/ns/pid", pid);
    len = readlink(path, link, sizeof(link) - 1);
    if (len <= 0) { snprintf(buf, size, "global"); return; }
    link[len] = '\0';
    start = strchr(link, '[');
    end   = strchr(link, ']');
    if (start && end && end > start)
    { *end = '\0'; snprintf(buf, size, "ns:%s", start + 1); }
    else
        snprintf(buf, size, "global");
}

/* ================================================================
**  opt_h — Option -h : afficher l'aide et quitter
**
**  POURQUOI exit(0) :
**    -h est une option informative. Le programme NE DOIT PAS
**    continuer à lister les fichiers après l'aide. exit(0)
**    arrête proprement le programme (code 0 = succès).
**
**  DIFFÉRENCE avec les autres fonctions :
**    opt_l / opt_o / opt_z → posent un flag et continuent
**    opt_h                 → affiche et ARRÊTE tout
**
**  EXEMPLE réel dans le terminal :
**    $ ./mylsof -h
**
**    Usage: mylsof [OPTIONS]
**      -l   Affiche l'UID brut au lieu du nom de login
**      -h   Affiche cette aide et quitte
**      -o   Affiche l'offset du fichier (position curseur)
**      -z   Affiche la colonne ZONE (namespace Linux)
**
**    Exemples :
**      ./mylsof -l
**      ./mylsof -o
**      ./mylsof -z
**      ./mylsof -l -o -z
**    $                        ← programme terminé proprement
** ================================================================ */
void opt_h(void)
{
    printf("\n");
    printf("  Usage: mylsof [OPTIONS]\n");
    printf("\n");
    printf("  Options de Sobrin:\n");
    printf("    -l         Affiche l'UID brut au lieu du nom de login\n");
    printf("    -h         Affiche cette aide et quitte\n");
    printf("    -o         Affiche l'offset du fichier\n");
    printf("    -z [zone]  Affiche la colonne ZONE (namespace Linux)\n");
    printf("\n");
    printf("  Exemples:\n");
    printf("    ./mylsof -l\n");
    printf("    ./mylsof -o\n");
    printf("    ./mylsof -z\n");
    printf("    ./mylsof -l -o -z\n");
    printf("\n");
    exit(0);
}

/* ================================================================
**  opt_l — Option -l : inhiber la conversion UID → nom de login
**
**  SANS -l (comportement par défaut) :
**    Le programme appelle getpwuid(uid) pour convertir
**    le numéro 1001 en nom lisible "alice".
**    Colonne USER affiche : alice
**
**  AVEC -l (ce que fait cette fonction) :
**    On pose cfg->show_uid = 1
**    La conversion getpwuid() est ignorée plus bas.
**    Colonne USER affiche : 1001
**
**  EXEMPLE réel dans le terminal :
**    $ ./mylsof          →  USER = root
**    $ ./mylsof -l       →  USER = 0
**
**    COMMAND         PID    USER       FD   TYPE   SIZE/OFF     NAME
**    bash            1234   0          0    REG    0t0          /dev/pts/0
**                           ↑
**                    UID brut affiché (pas "root")
** ================================================================ */
void opt_l(t_lsof *cfg)
{
    cfg->show_uid = 1;
}

/* ================================================================
**  opt_o — Option -o : forcer l'affichage de l'offset du fichier
**
**  SANS -o (comportement par défaut) :
**    La colonne s'appelle SIZE/OFF.
**    Elle affiche la taille du fichier en octets.
**
**  AVEC -o (ce que fait cette fonction) :
**    On pose cfg->show_offset = 1
**    La colonne s'appelle OFFSET.
**    Elle affiche la POSITION ACTUELLE du curseur dans le fichier.
**    Le préfixe "0t" signifie valeur décimale (notation lsof).
**
**  D'où vient l'offset ? → /proc/PID/fdinfo/N, champ "pos:"
**
**  EXEMPLE réel dans le terminal :
**    $ ./mylsof -o
**
**    COMMAND         PID    USER   FD   TYPE   OFFSET       NAME
**    bash            1234   root   0    REG    0t0          /dev/pts/0
**    nginx           5678   www    4    REG    0t12288      /var/log/access.log
**                                              ↑
**                                  12288 octets déjà lus dans ce fichier
** ================================================================ */
void opt_o(t_lsof *cfg)
{
    cfg->show_offset = 1;
}

/* ================================================================
**  opt_z — Option -z : afficher la colonne ZONE (namespace Linux)
**
**  SANS -z (comportement par défaut) :
**    Aucune colonne ZONE n'est affichée.
**
**  AVEC -z seul (ex: ./mylsof -z) :
**    On pose cfg->show_zone = 1  et  cfg->zone_filter = NULL
**    TOUS les processus sont affichés avec leur identifiant de zone.
**
**  AVEC -z + argument (ex: ./mylsof -z ns:4026531836) :
**    On filtre : seuls les processus de CETTE zone sont affichés.
**
**  D'où vient la zone ? → readlink("/proc/PID/ns/pid")
**    retourne : pid:[4026531836]  →  on extrait  ns:4026531836
**
**  EXEMPLE réel dans le terminal :
**    $ ./mylsof -z
**
**    ZONE            COMMAND         PID    USER   FD   NAME
**    ns:4026531836   bash            1234   root   0    /dev/pts/0
**    ns:4026531836   nginx           5678   www    4    /var/log/access.log
**    ns:4026532001   docker-proxy    8901   root   6    /dev/null
**    ↑
**    Chaque processus montre son namespace (zone d'isolation)
** ================================================================ */
void opt_z(t_lsof *cfg, char *zone_name)
{
    cfg->show_zone   = 1;
    cfg->zone_filter = zone_name; /* NULL = tout afficher */
}

/* ================================================================
**  AFFICHAGE D'UNE LIGNE PAR FICHIER OUVERT
**  Appelé pour chaque descripteur de fichier de chaque PID.
**  Applique les 4 flags : show_uid, show_offset, show_zone
** ================================================================ */
static void lister_fds_pid(int pid, t_lsof *cfg)
{
    char            fd_dir[64];
    char            fd_path[128];
    char            link_target[512];
    DIR            *dir;
    struct dirent  *entry;
    ssize_t         len;
    char            comm[64];
    int             uid;
    char            user_str[64];
    char            zone_str[64];
    long long       offset;
    int             fd_num;

    snprintf(fd_dir, sizeof(fd_dir), "/proc/%d/fd", pid);
    dir = opendir(fd_dir);
    if (!dir) return;

    get_comm(pid, comm, sizeof(comm));
    uid = get_uid(pid);

    /* -l : UID brut OU nom de login */
    if (cfg->show_uid || uid < 0)
        snprintf(user_str, sizeof(user_str), "%d", uid);
    else
    {
        struct passwd *pw = getpwuid((uid_t)uid);
        if (pw)
            snprintf(user_str, sizeof(user_str), "%s", pw->pw_name);
        else
            snprintf(user_str, sizeof(user_str), "%d", uid);
    }

    /* -z : lire la zone et filtrer si besoin */
    if (cfg->show_zone)
    {
        get_zone(pid, zone_str, sizeof(zone_str));
        if (cfg->zone_filter && strcmp(zone_str, cfg->zone_filter) != 0)
        { closedir(dir); return; }
    }

    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_name[0] == '.') continue;
        snprintf(fd_path, sizeof(fd_path), "/proc/%d/fd/%s", pid, entry->d_name);
        len = readlink(fd_path, link_target, sizeof(link_target) - 1);
        if (len <= 0) continue;
        link_target[len] = '\0';

        /* -z : colonne ZONE en première position */
        if (cfg->show_zone)
            printf("%-15s ", zone_str);

        printf("%-15s %-6d %-10s %-4s %-6s",
               comm, pid, user_str, entry->d_name, "REG");

        /* -o : colonne OFFSET après TYPE */
        if (cfg->show_offset)
        {
            fd_num = atoi(entry->d_name);
            offset = get_offset(pid, fd_num);
            printf(" 0t%-10lld", (offset >= 0) ? offset : 0LL);
        }

        printf(" %s\n", link_target);
    }
    closedir(dir);
}

/* ================================================================
**  GERER_OPTIONS_SOBRIN — Point d'entrée appelé depuis main.c
**
**  Retourne 0 : l'option n'est pas la nôtre → main essaie le suivant
**  Retourne 1 : option traitée → main s'arrête
** ================================================================ */
int gerer_options_Sobrin(int argc, char *argv[])
{
    t_lsof         cfg;
    DIR           *proc_dir;
    struct dirent *entry;
    int            i;

    /* L'option argv[1] nous appartient-elle ? */
    if (strcmp(argv[1], "-h") != 0 &&
        strcmp(argv[1], "-l") != 0 &&
        strcmp(argv[1], "-o") != 0 &&
        strcmp(argv[1], "-z") != 0)
        return (0);

    /* -h : cas spécial → afficher l'aide et quitter */
    if (strcmp(argv[1], "-h") == 0)
    {
        opt_h(); /* exit(0) à l'intérieur */
        return (1);
    }

    /* Initialiser la config */
    memset(&cfg, 0, sizeof(t_lsof));

    /* Parcourir tous les arguments et activer les flags */
    for (i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-l") == 0)
            opt_l(&cfg);
        else if (strcmp(argv[i], "-o") == 0)
            opt_o(&cfg);
        else if (strcmp(argv[i], "-z") == 0)
        {
            char *zone_arg = (i + 1 < argc && argv[i + 1][0] != '-')
                             ? argv[++i] : NULL;
            opt_z(&cfg, zone_arg);
        }
    }

    /* En-tête des colonnes */
    if (cfg.show_zone)  printf("%-15s ", "ZONE");
    printf("%-15s %-6s %-10s %-4s %-6s",
           "COMMAND", "PID", "USER", "FD", "TYPE");
    if (cfg.show_offset) printf(" %-12s", "OFFSET");
    else                 printf(" %-12s", "SIZE/OFF");
    printf(" %s\n", "NAME");

    /* Séparateur */
    if (cfg.show_zone) printf("%-15s ", "---------------");
    printf("%-15s %-6s %-10s %-4s %-6s %-12s %s\n",
           "---------------","------","----------","----","------","------------","----");

    /* Parcourir /proc et lister */
    proc_dir = opendir("/proc");
    if (!proc_dir) { perror("Erreur /proc"); return (1); }
    while ((entry = readdir(proc_dir)) != NULL)
    {
        if (entry->d_type != DT_DIR) continue;
        int pid = atoi(entry->d_name);
        if (pid <= 0) continue;
        lister_fds_pid(pid, &cfg);
    }
    closedir(proc_dir);
    return (1);
}
/* ================================================================
**  opt_Sobrin.c — Options : -l  -h  -o  -z
**  Projet : mylsof
**  Auteur : Sobrin
** ================================================================ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/stat.h>
#include "options.h"

/* ----------------------------------------------------------------
**  UTILITAIRE : lire le nom du processus dans /proc/PID/comm
** ---------------------------------------------------------------- */
static void get_comm(int pid, char *buf, size_t size)
{
    char  path[64];
    FILE *f;

    snprintf(path, sizeof(path), "/proc/%d/comm", pid);
    f = fopen(path, "r");
    if (!f) { snprintf(buf, size, "?"); return; }
    if (fgets(buf, (int)size, f))
        buf[strcspn(buf, "\n")] = '\0';
    else
        snprintf(buf, size, "?");
    fclose(f);
}

/* ----------------------------------------------------------------
**  UTILITAIRE : lire l'UID réel du processus dans /proc/PID/status
** ---------------------------------------------------------------- */
static int get_uid(int pid)
{
    char  path[64];
    char  line[256];
    FILE *f;
    int   uid = -1;

    snprintf(path, sizeof(path), "/proc/%d/status", pid);
    f = fopen(path, "r");
    if (!f) return (-1);
    while (fgets(line, sizeof(line), f))
        if (strncmp(line, "Uid:", 4) == 0)
        { sscanf(line + 4, "%d", &uid); break; }
    fclose(f);
    return (uid);
}

/* ----------------------------------------------------------------
**  UTILITAIRE : lire l'offset d'un fd dans /proc/PID/fdinfo/FD
**  L'offset = position actuelle du curseur de lecture (champ pos:)
** ---------------------------------------------------------------- */
static long long get_offset(int pid, int fd)
{
    char       path[128];
    char       line[128];
    FILE      *f;
    long long  offset = -1;

    snprintf(path, sizeof(path), "/proc/%d/fdinfo/%d", pid, fd);
    f = fopen(path, "r");
    if (!f) return (-1);
    while (fgets(line, sizeof(line), f))
        if (strncmp(line, "pos:", 4) == 0)
        { sscanf(line + 4, "%lld", &offset); break; }
    fclose(f);
    return (offset);
}

/* ----------------------------------------------------------------
**  UTILITAIRE : lire le namespace PID dans /proc/PID/ns/pid
**  Le lien symbolique ressemble à : pid:[4026531836]
**  On extrait le numéro d'inode → identifiant unique de la zone
** ---------------------------------------------------------------- */
static void get_zone(int pid, char *buf, size_t size)
{
    char    path[128];
    char    link[256];
    ssize_t len;
    char   *start;
    char   *end;

    snprintf(path, sizeof(path), "/proc/%d/ns/pid", pid);
    len = readlink(path, link, sizeof(link) - 1);
    if (len <= 0) { snprintf(buf, size, "global"); return; }
    link[len] = '\0';
    start = strchr(link, '[');
    end   = strchr(link, ']');
    if (start && end && end > start)
    { *end = '\0'; snprintf(buf, size, "ns:%s", start + 1); }
    else
        snprintf(buf, size, "global");
}

/* ================================================================
**  opt_h — Option -h : afficher l'aide et quitter
**
**  POURQUOI exit(0) :
**    -h est une option informative. Le programme NE DOIT PAS
**    continuer à lister les fichiers après l'aide. exit(0)
**    arrête proprement le programme (code 0 = succès).
**
**  DIFFÉRENCE avec les autres fonctions :
**    opt_l / opt_o / opt_z → posent un flag et continuent
**    opt_h                 → affiche et ARRÊTE tout
**
**  EXEMPLE réel dans le terminal :
**    $ ./mylsof -h
**
**    Usage: mylsof [OPTIONS]
**      -l   Affiche l'UID brut au lieu du nom de login
**      -h   Affiche cette aide et quitte
**      -o   Affiche l'offset du fichier (position curseur)
**      -z   Affiche la colonne ZONE (namespace Linux)

**
**    Exemples :
**      ./mylsof -l
**      ./mylsof -o
**      ./mylsof -z
**      ./mylsof -l -o -z
**    $                        ← programme terminé proprement
** ================================================================ */
void opt_h(void)
{
    printf("\n");
    printf("  Usage: mylsof [OPTIONS]\n");
    printf("\n");
    printf("  Options de Sobrin:\n");
    printf("    -l         Affiche l'UID brut au lieu du nom de login\n");
    printf("    -h         Affiche cette aide et quitte\n");
    printf("    -o         Affiche l'offset du fichier\n");
    printf("    -z [zone]  Affiche la colonne ZONE (namespace Linux)\n");
    printf("\n");
    printf("  Exemples:\n");
    printf("    ./mylsof -l\n");
    printf("    ./mylsof -o\n");
    printf("    ./mylsof -z\n");
    printf("    ./mylsof -l -o -z\n");
    printf("\n");
    exit(0);
}

/* ================================================================
**  opt_l — Option -l : inhiber la conversion UID → nom de login
**
**  SANS -l (comportement par défaut) :
**    Le programme appelle getpwuid(uid) pour convertir
**    le numéro 1001 en nom lisible "alice".
**    Colonne USER affiche : alice
**
**  AVEC -l (ce que fait cette fonction) :
**    On pose cfg->show_uid = 1
**    La conversion getpwuid() est ignorée plus bas.
**    Colonne USER affiche : 1001
**
**  EXEMPLE réel dans le terminal :
**    $ ./mylsof          →  USER = root
**    $ ./mylsof -l       →  USER = 0
**
**    COMMAND         PID    USER       FD   TYPE   SIZE/OFF     NAME
**    bash            1234   0          0    REG    0t0          /dev/pts/0
**                           ↑
**                    UID brut affiché (pas "root")
** ================================================================ */
void opt_l(t_lsof *cfg)
{
    cfg->show_uid = 1;
}

/* ================================================================
**  opt_o — Option -o : forcer l'affichage de l'offset du fichier
**
**  SANS -o (comportement par défaut) :
**    La colonne s'appelle SIZE/OFF.
**    Elle affiche la taille du fichier en octets.
**
**  AVEC -o (ce que fait cette fonction) :
**    On pose cfg->show_offset = 1
**    La colonne s'appelle OFFSET.
**    Elle affiche la POSITION ACTUELLE du curseur dans le fichier.
**    Le préfixe "0t" signifie valeur décimale (notation lsof).
**
**  D'où vient l'offset ? → /proc/PID/fdinfo/N, champ "pos:"
**
**  EXEMPLE réel dans le terminal :
**    $ ./mylsof -o
**
**    COMMAND         PID    USER   FD   TYPE   OFFSET       NAME
**    bash            1234   root   0    REG    0t0          /dev/pts/0
**    nginx           5678   www    4    REG    0t12288      /var/log/access.log
**                                              ↑
**                                  12288 octets déjà lus dans ce fichier
** ================================================================ */
void opt_o(t_lsof *cfg)
{
    cfg->show_offset = 1;
}

/* ================================================================
**  opt_z — Option -z : afficher la colonne ZONE (namespace Linux)
**
**  SANS -z (comportement par défaut) :
**    Aucune colonne ZONE n'est affichée.
**
**  AVEC -z seul (ex: ./mylsof -z) :
**    On pose cfg->show_zone = 1  et  cfg->zone_filter = NULL
**    TOUS les processus sont affichés avec leur identifiant de zone.
**

**  AVEC -z + argument (ex: ./mylsof -z ns:4026531836) :
**    On filtre : seuls les processus de CETTE zone sont affichés.
**
**  D'où vient la zone ? → readlink("/proc/PID/ns/pid")
**    retourne : pid:[4026531836]  →  on extrait  ns:4026531836
**
**  EXEMPLE réel dans le terminal :
**    $ ./mylsof -z
**
**    ZONE            COMMAND         PID    USER   FD   NAME
**    ns:4026531836   bash            1234   root   0    /dev/pts/0
**    ns:4026531836   nginx           5678   www    4    /var/log/access.log
**    ns:4026532001   docker-proxy    8901   root   6    /dev/null
**    ↑
**    Chaque processus montre son namespace (zone d'isolation)
** ================================================================ */
void opt_z(t_lsof *cfg, char *zone_name)
{
    cfg->show_zone   = 1;
    cfg->zone_filter = zone_name; /* NULL = tout afficher */
}

/* ================================================================
**  AFFICHAGE D'UNE LIGNE PAR FICHIER OUVERT
**  Appelé pour chaque descripteur de fichier de chaque PID.
**  Applique les 4 flags : show_uid, show_offset, show_zone
** ================================================================ */
static void lister_fds_pid(int pid, t_lsof *cfg)
{
    char            fd_dir[64];
    char            fd_path[128];
    char            link_target[512];
    DIR            *dir;
    struct dirent  *entry;
    ssize_t         len;
    char            comm[64];
    int             uid;
    char            user_str[64];
    char            zone_str[64];
    long long       offset;
    int             fd_num;

    snprintf(fd_dir, sizeof(fd_dir), "/proc/%d/fd", pid);
    dir = opendir(fd_dir);
    if (!dir) return;

    get_comm(pid, comm, sizeof(comm));
    uid = get_uid(pid);

    /* -l : UID brut OU nom de login */
    if (cfg->show_uid || uid < 0)
        snprintf(user_str, sizeof(user_str), "%d", uid);
    else
    {
        struct passwd *pw = getpwuid((uid_t)uid);
        if (pw)
            snprintf(user_str, sizeof(user_str), "%s", pw->pw_name);
        else
            snprintf(user_str, sizeof(user_str), "%d", uid);
    }

    /* -z : lire la zone et filtrer si besoin */
    if (cfg->show_zone)
    {
        get_zone(pid, zone_str, sizeof(zone_str));
        if (cfg->zone_filter && strcmp(zone_str, cfg->zone_filter) != 0)
        { closedir(dir); return; }
    }

    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_name[0] == '.') continue;
        snprintf(fd_path, sizeof(fd_path), "/proc/%d/fd/%s", pid, entry->d_name);
        len = readlink(fd_path, link_target, sizeof(link_target) - 1);
        if (len <= 0) continue;
        link_target[len] = '\0';

        /* -z : colonne ZONE en première position */
        if (cfg->show_zone)
            printf("%-15s ", zone_str);

        printf("%-15s %-6d %-10s %-4s %-6s",
               comm, pid, user_str, entry->d_name, "REG");

        /* -o : colonne OFFSET après TYPE */
        if (cfg->show_offset)
        {
            fd_num = atoi(entry->d_name);
            offset = get_offset(pid, fd_num);
            printf(" 0t%-10lld", (offset >= 0) ? offset : 0LL);
        }

        printf(" %s\n", link_target);
    }
    closedir(dir);
}

/* ================================================================
**  GERER_OPTIONS_SOBRIN — Point d'entrée appelé depuis main.c
**
**  Retourne 0 : l'option n'est pas la nôtre → main essaie le suivant
**  Retourne 1 : option traitée → main s'arrête
** ================================================================ */
int gerer_options_Sobrin(int argc, char *argv[])
{
    t_lsof         cfg;
    DIR           *proc_dir;
    struct dirent *entry;
    int            i;

    /* L'option argv[1] nous appartient-elle ? */
    if (strcmp(argv[1], "-h") != 0 &&
        strcmp(argv[1], "-l") != 0 &&
        strcmp(argv[1], "-o") != 0 &&
        strcmp(argv[1], "-z") != 0)
        return (0);

    /* -h : cas spécial → afficher l'aide et quitter */
    if (strcmp(argv[1], "-h") == 0)
    {
        opt_h(); /* exit(0) à l'intérieur */
        return (1);
    }

    /* Initialiser la config */
    memset(&cfg, 0, sizeof(t_lsof));

    /* Parcourir tous les arguments et activer les flags */
    for (i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-l") == 0)
            opt_l(&cfg);
        else if (strcmp(argv[i], "-o") == 0)
            opt_o(&cfg);
        else if (strcmp(argv[i], "-z") == 0)
        {
            char *zone_arg = (i + 1 < argc && argv[i + 1][0] != '-')
                             ? argv[++i] : NULL;
            opt_z(&cfg, zone_arg);
        }
    }

    /* En-tête des colonnes */
    if (cfg.show_zone)  printf("%-15s ", "ZONE");
    printf("%-15s %-6s %-10s %-4s %-6s",
           "COMMAND", "PID", "USER", "FD", "TYPE");
    if (cfg.show_offset) printf(" %-12s", "OFFSET");
    else                 printf(" %-12s", "SIZE/OFF");
    printf(" %s\n", "NAME");

    /* Séparateur */
    if (cfg.show_zone) printf("%-15s ", "---------------");
    printf("%-15s %-6s %-10s %-4s %-6s %-12s %s\n",
           "---------------","------","----------","----","------","------------","----");

    /* Parcourir /proc et lister */
    proc_dir = opendir("/proc");
    if (!proc_dir) { perror("Erreur /proc"); return (1); }
    while ((entry = readdir(proc_dir)) != NULL)
    {
        if (entry->d_type != DT_DIR) continue;
        int pid = atoi(entry->d_name);
        if (pid <= 0) continue;
        lister_fds_pid(pid, &cfg);
    }
    closedir(proc_dir);
    return (1);
}

