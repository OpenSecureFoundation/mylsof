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
void option_U()            { /* TODO */ }
void option_s(char *proto) { /* TODO */ }
void option_X()            { /* TODO */ }

int gerer_options_Salif(int argc, char *argv[]) {
    /* Salif ajoute tes if ici */
    return 0;
}
