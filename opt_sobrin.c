/* ================================================================
**  opt_Sobrin.c — Implémentation des options -l, -h, -o, -z
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

/* ================================================================
**  FONCTIONS UTILITAIRES INTERNES (privées à ce fichier)
** ================================================================ */

/*
** get_comm — lit le nom de la commande depuis /proc/PID/comm
** Retourne le nom dans le buffer fourni, ou "?" si introuvable.
*/
static void get_comm(int pid, char *buf, size_t size)
{
    char    path[64];
    FILE    *f;

    snprintf(path, sizeof(path), "/proc/%d/comm", pid);
    f = fopen(path, "r");
    if (!f) { snprintf(buf, size, "?"); return; }
    if (fgets(buf, (int)size, f))
    {
        /* supprimer le '\n' final */
        buf[strcspn(buf, "\n")] = '\0';
    }
    else
        snprintf(buf, size, "?");
    fclose(f);
}

/*
** get_uid — lit l'UID du propriétaire depuis /proc/PID/status
** Retourne l'UID ou -1 en cas d'erreur.
*/
static int get_uid(int pid)
{
    char    path[64];
    char    line[256];
    FILE    *f;
    int     uid = -1;

    snprintf(path, sizeof(path), "/proc/%d/status", pid);
    f = fopen(path, "r");
    if (!f) return (-1);
    while (fgets(line, sizeof(line), f))
    {
        if (strncmp(line, "Uid:", 4) == 0)
        {
            sscanf(line + 4, "%d", &uid);
            break;
        }
    }
    fclose(f);
    return (uid);
}

/*
** get_offset — lit l'offset du descripteur fd depuis /proc/PID/fdinfo/FD
** Retourne l'offset en long long, ou -1 si introuvable.
*/
static long long get_offset(int pid, int fd)
{
    char        path[128];
    char        line[128];
    FILE        *f;
    long long   offset = -1;

    snprintf(path, sizeof(path), "/proc/%d/fdinfo/%d", pid, fd);
    f = fopen(path, "r");
    if (!f) return (-1);
    while (fgets(line, sizeof(line), f))
    {
        if (strncmp(line, "pos:", 4) == 0)
        {
            sscanf(line + 4, "%lld", &offset);
            break;
        }
    }
    fclose(f);
    return (offset);
}

/*
** get_zone — lit le namespace PID depuis /proc/PID/ns/pid
** Retourne un identifiant de zone simplifié dans buf.
** Sur Linux, on affiche l'inode du namespace comme identifiant de zone.
*/
static void get_zone(int pid, char *buf, size_t size)
{
    char    path[128];
    char    link[256];
    ssize_t len;

    snprintf(path, sizeof(path), "/proc/%d/ns/pid", pid);
    len = readlink(path, link, sizeof(link) - 1);
    if (len <= 0)
    {
        snprintf(buf, size, "global");
        return;
    }
    link[len] = '\0';
    /*
    ** Le lien ressemble à "pid:[4026531836]"
    ** On extrait le numéro entre crochets comme identifiant de zone.
    */
    char *start = strchr(link, '[');
    char *end   = strchr(link, ']');
    if (start && end && end > start)
    {
        *end = '\0';
        snprintf(buf, size, "ns:%s", start + 1);
    }
    else
        snprintf(buf, size, "global");
}

/*
** lister_fds_pid — parcourt /proc/PID/fd et affiche chaque fichier ouvert
** Prend en compte les flags de la structure t_lsof (show_uid, show_offset, show_zone)
*/
static void lister_fds_pid(int pid, t_lsof *cfg)
{
    char            fd_dir[64];
    char            fd_path[128];
    char            link_target[512];
    DIR             *dir;
    struct dirent   *entry;
    ssize_t         len;
    char            comm[64];
    int             uid;
    char            user_str[64];
    char            zone_str[64];
    long long       offset;

    /* Construire le chemin /proc/PID/fd */
    snprintf(fd_dir, sizeof(fd_dir), "/proc/%d/fd", pid);
    dir = opendir(fd_dir);
    if (!dir) return;

    /* Récupérer le nom de la commande */
    get_comm(pid, comm, sizeof(comm));

    /* Récupérer l'UID */
    uid = get_uid(pid);

    /* --------------------------------------------------------
    ** -l : afficher l'UID brut OU le nom de login
    ** -------------------------------------------------------- */
    if (cfg->show_uid || uid < 0)
    {
        /* Option -l active : afficher le numéro brut */
        snprintf(user_str, sizeof(user_str), "%d", uid);
    }
    else
    {
        /* Comportement par défaut : convertir UID → nom login */
        struct passwd *pw = getpwuid((uid_t)uid);
        if (pw)
            snprintf(user_str, sizeof(user_str), "%s", pw->pw_name);
        else
            snprintf(user_str, sizeof(user_str), "%d", uid);
    }

    /* --------------------------------------------------------
    ** -z : récupérer le nom de zone si demandé
    ** -------------------------------------------------------- */
    if (cfg->show_zone)
    {
        get_zone(pid, zone_str, sizeof(zone_str));
        /* Filtre de zone : ignorer les PIDs hors de la zone voulue */
        if (cfg->zone_filter && strcmp(zone_str, cfg->zone_filter) != 0)
        {
            closedir(dir);
            return;
        }
    }

    /* Parcourir tous les descripteurs de fichiers du PID */
    while ((entry = readdir(dir)) != NULL)
    {
        int fd_num = atoi(entry->d_name);
        if (fd_num < 0 && strcmp(entry->d_name, "0") != 0) continue;
        if (entry->d_name[0] == '.') continue;

        /* Lire la cible du lien symbolique /proc/PID/fd/N */
        snprintf(fd_path, sizeof(fd_path), "/proc/%d/fd/%s", pid, entry->d_name);
        len = readlink(fd_path, link_target, sizeof(link_target) - 1);
        if (len <= 0) continue;
        link_target[len] = '\0';

        /* --------------------------------------------------------
        ** -z : afficher la colonne ZONE en premier
        ** -------------------------------------------------------- */
        if (cfg->show_zone)
            printf("%-15s ", zone_str);

        /* Colonnes fixes : COMMAND  PID  USER  FD  TYPE  NAME */
        printf("%-15s %-6d %-10s %-4s %-6s",
               comm, pid, user_str, entry->d_name, "REG");

        /* --------------------------------------------------------
        ** -o : afficher l'OFFSET à la place ou en plus de SIZE
        ** -------------------------------------------------------- */
        if (cfg->show_offset)
        {
            fd_num = atoi(entry->d_name);
            offset = get_offset(pid, fd_num);
            if (offset >= 0)
                printf(" 0t%-10lld", offset);
            else
                printf(" %-12s", "0t0");
        }

        /* Afficher le nom du fichier (cible du lien symbolique) */
        printf(" %s\n", link_target);
    }
    closedir(dir);
}

/* ================================================================
**  OPT_H — Affiche l'aide et quitte
**
**  Comportement :
**    ./mylsof -h  →  affiche toutes les options  →  exit(0)
**
**  Exemple de sortie :
**    Usage: mylsof [OPTIONS]
**      -l   Inhiber la conversion UID → nom de login
**      -h   Afficher cette aide et quitter
**      -o   Afficher l'offset du fichier (colonne OFFSET)
**      -z   Afficher les noms de zone (namespace Linux)
** ================================================================ */
void opt_h(void)
{
    printf("\n");
    printf("  Usage: mylsof [OPTIONS]\n");
    printf("\n");
    printf("  Description:\n");
    printf("    mylsof liste tous les fichiers ouverts par les processus\n");
    printf("    actifs sur le systeme, en lisant /proc/PID/fd\n");
    printf("\n");
    printf("  Options de Sobrin:\n");
    printf("    -l         Inhiber la conversion UID vers nom de login\n");
    printf("               (affiche le numero d'utilisateur brut)\n");
    printf("\n");
    printf("    -h         Afficher cette aide et quitter le programme\n");
    printf("\n");
    printf("    -o         Afficher l'offset du fichier a la place de sa taille\n");
    printf("               (position actuelle du curseur de lecture/ecriture)\n");
    printf("               Prefixe 0t = decimal,  0x = hexadecimal\n");
    printf("\n");
    printf("    -z [zone]  Afficher la colonne ZONE (namespace Linux)\n");
    printf("               Sans argument : affiche tous les processus avec leur zone\n");
    printf("               Avec argument : filtre uniquement la zone specifiee\n");
    printf("\n");
    printf("  Exemples:\n");
    printf("    ./mylsof          Liste tous les fichiers ouverts\n");
    printf("    ./mylsof -l       Affiche les UID bruts (pas les noms)\n");
    printf("    ./mylsof -o       Affiche les offsets de lecture\n");
    printf("    ./mylsof -z       Affiche les zones (namespaces)\n");
    printf("    ./mylsof -z ns:4026531836   Filtre par zone specifique\n");
    printf("\n");
    exit(0);
}

/* ================================================================
**  OPT_L — Inhiber la conversion UID → nom de login
**
**  Comportement :
**    Active le flag show_uid dans la config.
**    La colonne USER affichera "1001" au lieu de "alice".
**
**  Source Linux : getpwuid() est IGNORÉ quand ce flag est actif.
** ================================================================ */
void opt_l(t_lsof *cfg)
{
    cfg->show_uid = 1;
}

/* ================================================================
**  OPT_O — Forcer l'affichage de l'offset du fichier
**
**  Comportement :
**    Active le flag show_offset dans la config.
**    L'offset se lit dans /proc/PID/fdinfo/N (champ "pos:").
**    Affiché avec le préfixe "0t" (décimal) devant le nombre.
**
**  Exemple : pos: 4096  →  affiche "0t4096"
** ================================================================ */
void opt_o(t_lsof *cfg)
{
    cfg->show_offset = 1;
}

/* ================================================================
**  OPT_Z — Afficher les noms de zone (namespaces Linux)
**
**  Comportement :
**    Active le flag show_zone dans la config.
**    La zone est lue via readlink("/proc/PID/ns/pid").
**    Si zone_name est fourni : filtre uniquement cette zone.
**    Si zone_name est NULL   : affiche toutes les zones.
**
**  Exemple de zone : "ns:4026531836" (inode du namespace)
** ================================================================ */
void opt_z(t_lsof *cfg, char *zone_name)
{
    cfg->show_zone = 1;
    if (zone_name)
        cfg->zone_filter = zone_name;
    else
        cfg->zone_filter = NULL;
}

/* ================================================================
**  GERER_OPTIONS_SOBRIN — Point d'entrée appelé depuis main.c
**
**  Détecte si argv[1] est une option de Sobrin (-l, -h, -o, -z).
**  Si oui : configure et exécute, retourne 1 (main s'arrête).
**  Si non : retourne 0 (main essaie le membre suivant).
** ================================================================ */
int gerer_options_Sobrin(int argc, char *argv[])
{
    t_lsof  cfg;
    DIR     *proc_dir;
    struct dirent *entry;

    /* Vérifier si l'option appartient à Sobrin */
    if (strcmp(argv[1], "-h") != 0 &&
        strcmp(argv[1], "-l") != 0 &&
        strcmp(argv[1], "-o") != 0 &&
        strcmp(argv[1], "-z") != 0)
        return (0); /* pas notre option */

    /* Initialiser la configuration à zéro */
    memset(&cfg, 0, sizeof(t_lsof));

    /* --------------------------------------------------------
    ** -h : cas spécial — affiche l'aide et quitte immédiatement
    ** -------------------------------------------------------- */
    if (strcmp(argv[1], "-h") == 0)
    {
        opt_h();
        return (1); /* jamais atteint (exit dans opt_h) */
    }

    /* --------------------------------------------------------
    ** Appliquer toutes les options présentes sur la ligne
    ** (ex: ./mylsof -l -o  active les deux flags)
    ** -------------------------------------------------------- */
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-l") == 0)
            opt_l(&cfg);
        else if (strcmp(argv[i], "-o") == 0)
            opt_o(&cfg);
        else if (strcmp(argv[i], "-z") == 0)
        {
            /* -z peut avoir un argument optionnel : -z nom_zone */
            char *zone_arg = (i + 1 < argc && argv[i+1][0] != '-')
                             ? argv[++i] : NULL;
            opt_z(&cfg, zone_arg);
        }
    }

    /* --------------------------------------------------------
    ** Afficher l'en-tête des colonnes
    ** -------------------------------------------------------- */
    if (cfg.show_zone)
        printf("%-15s ", "ZONE");

    printf("%-15s %-6s %-10s %-4s %-6s",
           "COMMAND", "PID", "USER", "FD", "TYPE");

    if (cfg.show_offset)
        printf(" %-12s", "OFFSET");
    else
        printf(" %-12s", "SIZE/OFF");

    printf(" %s\n", "NAME");

    /* Ligne séparatrice */
    if (cfg.show_zone) printf("%-15s ", "-------");
    printf("%-15s %-6s %-10s %-4s %-6s %-12s %s\n",
           "-------","------","----------","----","------","------------","----");

    /* --------------------------------------------------------
    ** Parcourir /proc et afficher les fichiers ouverts
    ** -------------------------------------------------------- */
    proc_dir = opendir("/proc");
    if (!proc_dir) { perror("Erreur ouverture /proc"); return (1); }

    while ((entry = readdir(proc_dir)) != NULL)
    {
        if (entry->d_type != DT_DIR) continue;
        int pid = atoi(entry->d_name);
        if (pid <= 0) continue;
        lister_fds_pid(pid, &cfg);
    }
    closedir(proc_dir);
    return (1); /* option traitée */
}
