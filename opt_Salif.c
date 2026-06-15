#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>    /* pour DIR et struct dirent */
#include <unistd.h>    /* pour readlink() */
#include <sys/stat.h>  /* pour stat() */
#include "options.h"

/* OPTION -P */
/* ========================= */
void option_P() {

    DIR *proc;
    struct dirent *entree;

    char chemin[512];
    char lien[512];
    char nom[512];

    proc = opendir("/proc");

    if (proc == NULL) {
        printf("Erreur /proc\n");
        return;
    }

    while ((entree = readdir(proc)) != NULL) {

        int pid = atoi(entree->d_name);

        if (pid <= 0)
            continue;

        /* lire le nom du processus */
        sprintf(chemin, "/proc/%d/comm", pid);

        FILE *f = fopen(chemin, "r");

        if (f == NULL)
            continue;

        fgets(nom, sizeof(nom), f);

        fclose(f);

        nom[strcspn(nom, "\n")] = 0;

        /* ouvrir les fichiers fd */
        sprintf(chemin, "/proc/%d/fd", pid);

        DIR *fd = opendir(chemin);

        if (fd == NULL)
            continue;

        struct dirent *fd_entree;

        while ((fd_entree = readdir(fd)) != NULL) {

            if (fd_entree->d_name[0] == '.')
                continue;

            sprintf(chemin,
                    "/proc/%d/fd/%s",
                    pid,
                    fd_entree->d_name);

            int len = readlink(chemin,
                               lien,
                               sizeof(lien) - 1);

            if (len == -1)
                continue;

            lien[len] = '\0';

            /* afficher seulement les sockets */
            if (strncmp(lien, "socket:", 7) == 0) {

                printf("%s  %d  %s  %s\n",
                       nom,
                       pid,
                       fd_entree->d_name,
                       lien);
            }
        }

        closedir(fd);
    }

    closedir(proc);
}

/* OPTION -g */
/* ========================= */
void option_g(char *grp) {

    int gid = atoi(grp);

    DIR *proc;
    struct dirent *entree;

    char chemin[512];
    char ligne[512];
    char nom[512];

    proc = opendir("/proc");

    if (proc == NULL) {
        printf("Erreur /proc\n");
        return;
    }

    while ((entree = readdir(proc)) != NULL) {

        int pid = atoi(entree->d_name);

        if (pid <= 0)
            continue;

        /* lire le fichier status */
        sprintf(chemin, "/proc/%d/status", pid);

        FILE *f = fopen(chemin, "r");

        if (f == NULL)
            continue;

        int gid_trouve = -1;

        while (fgets(ligne, sizeof(ligne), f)) {

            if (strncmp(ligne, "Gid:", 4) == 0) {

                sscanf(ligne,
                       "Gid: %*d %d",
                       &gid_trouve);

                break;
            }
        }

        fclose(f);

        if (gid_trouve != gid)
            continue;

        /* lire nom processus */
        sprintf(chemin, "/proc/%d/comm", pid);

        f = fopen(chemin, "r");

        if (f == NULL)
            continue;

        fgets(nom, sizeof(nom), f);

        fclose(f);

        nom[strcspn(nom, "\n")] = 0;

        printf("Processus : %s  PID : %d  GID : %d\n",
               nom,
               pid,
               gid_trouve);
    }

    closedir(proc);
}

/* ========================= */
/* OPTION -U */
/* ========================= */

void option_U() {

    printf("Affichage des sockets Unix...\n");

    DIR *dossier_proc;
    struct dirent *entree;

    char chemin[512];
    char lien[512];
    char nom_process[512];

    dossier_proc = opendir("/proc");

    if (dossier_proc == NULL) {
        printf("Erreur ouverture /proc\n");
        return;
    }

    while ((entree = readdir(dossier_proc)) != NULL) {

        int pid = atoi(entree->d_name);

        if (pid <= 0)
            continue;

        snprintf(chemin,
                 sizeof(chemin),
                 "/proc/%d/comm",
                 pid);

        FILE *f = fopen(chemin, "r");

        if (f == NULL)
            continue;

        fgets(nom_process,
              sizeof(nom_process),
              f);

        fclose(f);

        nom_process[strcspn(nom_process, "\n")] = 0;

        snprintf(chemin,
                 sizeof(chemin),
                 "/proc/%d/fd",
                 pid);

        DIR *dossier_fd = opendir(chemin);

        if (dossier_fd == NULL)
            continue;

        struct dirent *fd_entree;

        while ((fd_entree = readdir(dossier_fd)) != NULL) {

            if (fd_entree->d_name[0] == '.')
                continue;

            snprintf(chemin,
                     sizeof(chemin),
                     "/proc/%d/fd/%s",
                     pid,
                     fd_entree->d_name);

            int len = readlink(chemin,
                               lien,
                               sizeof(lien) - 1);

            if (len == -1)
                continue;

            lien[len] = '\0';

            if (strncmp(lien, "socket:", 7) == 0) {

                printf("%-20s %-6d %-6s %-40s\n",
                       nom_process,
                       pid,
                       fd_entree->d_name,
                       lien);
            }
        }

        closedir(dossier_fd);
    }

    closedir(dossier_proc);
}



/*---------- OPTION -s ----------*/


void option_s(char *proto) {

    (void)proto;

    DIR *proc;
    struct dirent *entree;

    char chemin[512];
    char lien[512];
    char nom[512];

    struct stat info;

    proc = opendir("/proc");

    if (proc == NULL) {
        printf("Erreur /proc\n");
        return;
    }

    while ((entree = readdir(proc)) != NULL) {

        int pid = atoi(entree->d_name);

        if (pid <= 0)
            continue;

        /* nom processus */
        sprintf(chemin, "/proc/%d/comm", pid);

        FILE *f = fopen(chemin, "r");

        if (f == NULL)
            continue;

        fgets(nom, sizeof(nom), f);

        fclose(f);

        nom[strcspn(nom, "\n")] = 0;

        /* dossier fd */
        sprintf(chemin, "/proc/%d/fd", pid);

        DIR *fd = opendir(chemin);

        if (fd == NULL)
            continue;

        struct dirent *fd_entree;

        while ((fd_entree = readdir(fd)) != NULL) {

            if (fd_entree->d_name[0] == '.')
                continue;

            sprintf(chemin,
                    "/proc/%d/fd/%s",
                    pid,
                    fd_entree->d_name);

            int len = readlink(chemin,
                               lien,
                               sizeof(lien) - 1);

            if (len == -1)
                continue;

            lien[len] = '\0';

            if (stat(lien, &info) == 0) {

                printf("%s  %d  %ld octets  %s\n",
                       nom,
                       pid,
                       (long)info.st_size,
                       lien);
            }
        }

        closedir(fd);
    }

    closedir(proc);
}




/* ----------OPTION -X----------*/


void option_X() {

    printf("Affichage des fichiers mappés en mémoire...\n");

    DIR *dossier_proc;
    struct dirent *entree;

    char chemin[512];
    char ligne[1024];
    char nom_process[512];

    dossier_proc = opendir("/proc");

    if (dossier_proc == NULL) {
        printf("Erreur ouverture /proc\n");
        return;
    }

    while ((entree = readdir(dossier_proc)) != NULL) {

        int pid = atoi(entree->d_name);

        if (pid <= 0)
            continue;

        snprintf(chemin,
                 sizeof(chemin),
                 "/proc/%d/comm",
                 pid);

        FILE *f = fopen(chemin, "r");

        if (f == NULL)
            continue;

        fgets(nom_process,
              sizeof(nom_process),
              f);

        fclose(f);

        nom_process[strcspn(nom_process, "\n")] = 0;

        snprintf(chemin,
                 sizeof(chemin),
                 "/proc/%d/maps",
                 pid);

        FILE *f_maps = fopen(chemin, "r");

        if (f_maps == NULL)
            continue;

        while (fgets(ligne,
                      sizeof(ligne),
                      f_maps)) {

            char adresse[64];
            char fichier[512];

            fichier[0] = '\0';

            sscanf(ligne,
                   "%63s %*s %*s %*s %*s %511[^\n]",
                   adresse,
                   fichier);

            if (fichier[0] == '/') {

                printf("%-20s %-6d %-20s %-40s\n",
                       nom_process,
                       pid,
                       adresse,
                       fichier);
            }
        }

        fclose(f_maps);
    }

    closedir(dossier_proc);
}



/* ----------OPTION -Z----------*/

#include "options.h"

void option_Z(void) {

    DIR *dossier_proc;
    struct dirent *entree;
    char chemin[512];
    char lien[512];
    char nom_process[512];
    char contexte[256];         /* stocke le label SELinux */

    printf("Contextes SELinux des processus\n\n");
    printf("%-20s %-6s %-6s %-30s %-40s\n",
           "COMMANDE", "PID", "FD", "CONTEXTE SELinux", "FICHIER");
    printf("%-20s %-6s %-6s %-30s %-40s\n",
           "--------", "---", "--", "----------------", "-------");

    dossier_proc = opendir("/proc");
    if (dossier_proc == NULL) {
        printf("Erreur : impossible d'ouvrir /proc\n");
        return;
    }

    while ((entree = readdir(dossier_proc)) != NULL) {

        int pid = atoi(entree->d_name);
        if (pid <= 0) continue;

        /* Lire le nom du processus */
        snprintf(chemin, sizeof(chemin), "/proc/%d/comm", pid);
        FILE *f_comm = fopen(chemin, "r");
        if (f_comm == NULL) continue;
        fgets(nom_process, sizeof(nom_process), f_comm);
        fclose(f_comm);
        nom_process[strcspn(nom_process, "\n")] = 0;

        /* Lire le contexte SELinux dans /proc/PID/attr/current */
        /* Ce fichier contient le label de sécurité du processus */
        snprintf(chemin, sizeof(chemin), "/proc/%d/attr/current", pid);
        FILE *f_attr = fopen(chemin, "r");

        /* Si le fichier n'existe pas, SELinux n'est pas actif */
        strcpy(contexte, "(SELinux inactif)");
        if (f_attr != NULL) {
            fgets(contexte, sizeof(contexte), f_attr);
            fclose(f_attr);
            contexte[strcspn(contexte, "\n")] = 0; /* enlever le \n */
        }

        /* Parcourir les fichiers ouverts du processus */
        snprintf(chemin, sizeof(chemin), "/proc/%d/fd", pid);
        DIR *dossier_fd = opendir(chemin);
        if (dossier_fd == NULL) continue;

        struct dirent *fd_entree;
        while ((fd_entree = readdir(dossier_fd)) != NULL) {

            if (fd_entree->d_name[0] == '.') continue;

            snprintf(chemin, sizeof(chemin), "/proc/%d/fd/%s", pid, fd_entree->d_name);
            int len = readlink(chemin, lien, sizeof(lien) - 1);
            if (len == -1) continue;
            lien[len] = '\0';

            printf("%-20s %-6d %-6s %-30s %-40s\n",
                   nom_process, pid,
                   fd_entree->d_name,
                   contexte,
                   lien);
        }
        closedir(dossier_fd);
    }
    closedir(dossier_proc);
}


/* ----------OPTION -N ----------*/


#include "options.h"

void option_N(void) {

    DIR *dossier_proc;       /* pour ouvrir /proc */
    struct dirent *entree;   /* pour lire chaque élément de /proc */
    char chemin[512];        /* stocke un chemin de fichier */
    char lien[512];          /* stocke la destination d'un lien */
    char nom_process[256];   /* stocke le nom du processus */

    printf("Fichiers NFS ouverts\n\n");
    printf("%-20s %-6s %-6s %-50s\n", "COMMANDE", "PID", "FD", "FICHIER NFS");
    printf("%-20s %-6s %-6s %-50s\n", "--------", "---", "--", "-----------");

    /* Ouvrir /proc pour parcourir tous les processus */
    dossier_proc = opendir("/proc");
    if (dossier_proc == NULL) {
        printf("Erreur : impossible d'ouvrir /proc\n");
        return;
    }

    while ((entree = readdir(dossier_proc)) != NULL) {

        /* Vérifier si c'est un numéro de PID */
        int pid = atoi(entree->d_name);
        if (pid <= 0) continue;

        /* Lire le nom du processus dans /proc/PID/comm */
        snprintf(chemin, sizeof(chemin), "/proc/%d/comm", pid);
        FILE *f = fopen(chemin, "r");
        if (f == NULL) continue;
        fgets(nom_process, sizeof(nom_process), f);
        fclose(f);
        nom_process[strcspn(nom_process, "\n")] = 0; /* enlever le \n */

        /* Ouvrir /proc/PID/fd pour lister les fichiers ouverts */
        snprintf(chemin, sizeof(chemin), "/proc/%d/fd", pid);
        DIR *dossier_fd = opendir(chemin);
        if (dossier_fd == NULL) continue;

        struct dirent *fd_entree;
        while ((fd_entree = readdir(dossier_fd)) != NULL) {

            if (fd_entree->d_name[0] == '.') continue;

            /* Lire où pointe le lien symbolique */
            snprintf(chemin, sizeof(chemin), "/proc/%d/fd/%s", pid, fd_entree->d_name);
            int len = readlink(chemin, lien, sizeof(lien) - 1);
            if (len == -1) continue;
            lien[len] = '\0';

            /* Un fichier NFS est monté dans /net/ ou /nfs/ ou contient "nfs" */
            /* On vérifie si le chemin commence par /net/ ou /nfs/ */
            if (strncmp(lien, "/net/", 5) == 0 ||
                strncmp(lien, "/nfs/", 5) == 0 ||
                strstr(lien, "nfs") != NULL) {

                printf("%-20s %-6d %-6s %-50s\n",
                       nom_process, pid,
                       fd_entree->d_name,
                       lien);
            }
        }
        closedir(dossier_fd);
    }
    closedir(dossier_proc);
}



/* ----------OPTION -j ----------*/

#include "options.h"

void option_j(void) {

    DIR *dossier_proc;
    struct dirent *entree;
    char chemin[512];
    char lien[512];
    char nom_process[256];
    char ns_lien[256];     /* stocke le namespace du processus */

    printf("Namespaces des processus\n\n");
    printf("%-20s %-6s %-6s %-20s %-40s\n",
           "COMMANDE", "PID", "FD", "NAMESPACE", "FICHIER");
    printf("%-20s %-6s %-6s %-20s %-40s\n",
           "--------", "---", "--", "---------", "-------");

    dossier_proc = opendir("/proc");
    if (dossier_proc == NULL) {
        printf("Erreur : impossible d'ouvrir /proc\n");
        return;
    }

    while ((entree = readdir(dossier_proc)) != NULL) {

        int pid = atoi(entree->d_name);
        if (pid <= 0) continue;

        /* Lire le nom du processus */
        snprintf(chemin, sizeof(chemin), "/proc/%d/comm", pid);
        FILE *f = fopen(chemin, "r");
        if (f == NULL) continue;
        fgets(nom_process, sizeof(nom_process), f);
        fclose(f);
        nom_process[strcspn(nom_process, "\n")] = 0;

        /* Lire le namespace dans /proc/PID/ns/mnt */
        /* Ce lien pointe vers quelque chose comme : mnt:[4026531840] */
        /* Le numéro entre crochets identifie le namespace */
        snprintf(chemin, sizeof(chemin), "/proc/%d/ns/mnt", pid);
        int ns_len = readlink(chemin, ns_lien, sizeof(ns_lien) - 1);

        /* Si on ne peut pas lire le namespace */
        strcpy(ns_lien, "(inconnu)");
        if (ns_len > 0) {
            ns_lien[ns_len] = '\0';
        }

        /* Parcourir les fichiers ouverts */
        snprintf(chemin, sizeof(chemin), "/proc/%d/fd", pid);
        DIR *dossier_fd = opendir(chemin);
        if (dossier_fd == NULL) continue;

        struct dirent *fd_entree;
        while ((fd_entree = readdir(dossier_fd)) != NULL) {

            if (fd_entree->d_name[0] == '.') continue;

            snprintf(chemin, sizeof(chemin), "/proc/%d/fd/%s", pid, fd_entree->d_name);
            int len = readlink(chemin, lien, sizeof(lien) - 1);
            if (len == -1) continue;
            lien[len] = '\0';

            printf("%-20s %-6d %-6s %-20s %-40s\n",
                   nom_process, pid,
                   fd_entree->d_name,
                   ns_lien,
                   lien);
        }
        closedir(dossier_fd);
    }
    closedir(dossier_proc);
}


/* ----------OPTION +c ----------*/
#include "options.h"

void option_c_plus(char *nom_recherche) {

    DIR *dossier_proc;
    struct dirent *entree;
    char chemin[512];
    char lien[512];
    char nom_process[256];

    printf("Fichiers ouverts par les processus commençant par : %s\n\n",
           nom_recherche);
    printf("%-20s %-6s %-6s %-50s\n", "COMMANDE", "PID", "FD", "FICHIER");
    printf("%-20s %-6s %-6s %-50s\n", "--------", "---", "--", "-------");

    dossier_proc = opendir("/proc");
    if (dossier_proc == NULL) {
        printf("Erreur : impossible d'ouvrir /proc\n");
        return;
    }

    while ((entree = readdir(dossier_proc)) != NULL) {

        int pid = atoi(entree->d_name);
        if (pid <= 0) continue;

        /* Lire le nom du processus dans /proc/PID/comm */
        snprintf(chemin, sizeof(chemin), "/proc/%d/comm", pid);
        FILE *f = fopen(chemin, "r");
        if (f == NULL) continue;
        fgets(nom_process, sizeof(nom_process), f);
        fclose(f);
        nom_process[strcspn(nom_process, "\n")] = 0;

        /* Comparer le début du nom du processus avec le nom recherché */
        /* strncmp compare les N premiers caractères */
        /* N = longueur du nom recherché */
        int longueur = strlen(nom_recherche);
        if (strncmp(nom_process, nom_recherche, longueur) != 0) {
            continue; /* le nom ne correspond pas, on passe */
        }

        /* Le nom correspond ! On liste ses fichiers ouverts */
        snprintf(chemin, sizeof(chemin), "/proc/%d/fd", pid);
        DIR *dossier_fd = opendir(chemin);
        if (dossier_fd == NULL) continue;

        struct dirent *fd_entree;
        while ((fd_entree = readdir(dossier_fd)) != NULL) {

            if (fd_entree->d_name[0] == '.') continue;

            snprintf(chemin, sizeof(chemin), "/proc/%d/fd/%s", pid, fd_entree->d_name);
            int len = readlink(chemin, lien, sizeof(lien) - 1);
            if (len == -1) continue;
            lien[len] = '\0';

            printf("%-20s %-6d %-6s %-50s\n",
                   nom_process, pid,
                   fd_entree->d_name,
                   lien);
        }
        closedir(dossier_fd);
    }
    closedir(dossier_proc);
}

/*appel des fonctions*/

int gerer_options_Salif(int argc, char *argv[]) {

    int i;
    for (i = 1; i < argc; i++) {

        if (strcmp(argv[i], "-P") == 0) {
            option_P();
        }
        else if (strcmp(argv[i], "-g") == 0 && i + 1 < argc) {
            option_g(argv[i + 1]);
            i++;
        }
        else if (strcmp(argv[i], "-U") == 0) {
            option_U();
        }
        else if (strcmp(argv[i], "-s") == 0) {
            option_s(NULL);
        }
        else if (strcmp(argv[i], "-X") == 0) {
            option_X();
        }
        else if (strcmp(argv[i], "-N") == 0) {
            option_N();
        }
        else if (strcmp(argv[i], "-Z") == 0) {
            option_Z();
        }
        else if (strcmp(argv[i], "-j") == 0) {
            option_j();
        }
        else if (strcmp(argv[i], "+c") == 0 && i + 1 < argc) {
            option_c_plus(argv[i + 1]);
            i++;
        }
    }
    return 0;
}
