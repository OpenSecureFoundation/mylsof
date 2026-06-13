#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "options.h"

/* OPTION -P */
/* ========================= */
void option_P() {

    DIR *proc;
    struct dirent *entree;

    char chemin[256];
    char lien[256];
    char nom[256];

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

    char chemin[256];
    char ligne[256];
    char nom[256];

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
    char nom_process[256];

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

void option_s(char *proto) { /* TODO */ }
void option_X()            { /* TODO */ }

int gerer_options_Salif(int argc, char *argv[]) {
    /* Salif ajoute tes if ici */
    return 0;
}
