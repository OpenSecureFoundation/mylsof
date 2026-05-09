#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/stat.h>
#include "options.h"

int main(int argc, char *argv[]) {

    if (argc >= 2) {
        if (gerer_options_Ange(argc, argv))      return 0;
        if (gerer_options_Cassandra(argc, argv)) return 0;
        if (gerer_options_Naomy(argc, argv))     return 0;
        if (gerer_options_Salif(argc, argv))     return 0;
        if (gerer_options_Sobrin(argc, argv))    return 0;
        printf("Option inconnue : %s\n", argv[1]);
        return 1;
    }

    DIR *proc_dir = opendir("/proc");
    if (!proc_dir) { perror("Erreur /proc"); return 1; }

    printf("%-15s %-6s %-10s %-4s %-6s %s\n",
           "COMMAND","PID","USER","FD","TYPE","NAME");

    struct dirent *entry;
    while ((entry = readdir(proc_dir)) != NULL) {
        if (entry->d_type != DT_DIR) continue;
        int pid = atoi(entry->d_name);
        if (pid <= 0) continue;
        afficher_fichiers_pid(pid);
    }
    closedir(proc_dir);
    return 0;
}
