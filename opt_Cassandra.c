#include "options.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <dirent.h>
#include <unistd.h>

void    opt_d(t_lsof *data)
{
    DIR             *proc_dir;
    DIR             *fd_dir;
    struct dirent   *proc_entry;
    struct dirent   *fd_entry;
    char            fd_path[256];
    char            full_fd[512];
    char            target[512];
    ssize_t         len;

    proc_dir = opendir("/proc");
    if (!proc_dir)
    {
        perror("opendir -d");
        return ;
    }
    while ((proc_entry = readdir(proc_dir)) != NULL)
    {
        if (atoi(proc_entry->d_name) <= 0)
            continue ;
        snprintf(fd_path, sizeof(fd_path), "/proc/%s/fd", proc_entry->d_name);
        fd_dir = opendir(fd_path);
        if (!fd_dir)
            continue ;
        while ((fd_entry = readdir(fd_dir)) != NULL)
        {
            if (atoi(fd_entry->d_name) != data->fd_filter)
                continue ;
            snprintf(full_fd, sizeof(full_fd), "%s/%s", fd_path, fd_entry->d_name);
            len = readlink(full_fd, target, sizeof(target) - 1);
            if (len == -1)
                continue ;
            target[len] = '\0';
            printf("PID %s -> fd %s -> %s\n",
                proc_entry->d_name, fd_entry->d_name, target);
        }
        closedir(fd_dir);
    }
    closedir(proc_dir);
}

void    opt_u(t_lsof *data)
{
    struct passwd   *pw;
    DIR             *proc_dir;
    DIR             *fd_dir;
    struct dirent   *proc_entry;
    struct dirent   *fd_entry;
    char            status_path[256];
    char            fd_path[256];
    char            full_fd[512];
    char            target[512];
    FILE            *f;
    char            line[256];
    int             uid;
    ssize_t         len;

    pw = getpwnam(data->username);
    if (!pw)
    {
        fprintf(stderr, "Utilisateur introuvable : %s\n", data->username);
        return ;
    }
    proc_dir = opendir("/proc");
    if (!proc_dir)
    {
        perror("opendir -u");
        return ;
    }
    while ((proc_entry = readdir(proc_dir)) != NULL)
    {
        if (atoi(proc_entry->d_name) <= 0)
            continue ;
        snprintf(status_path, sizeof(status_path),
            "/proc/%s/status", proc_entry->d_name);
        f = fopen(status_path, "r");
        if (!f)
            continue ;
        uid = -1;
        while (fgets(line, sizeof(line), f))
        {
            if (strncmp(line, "Uid:", 4) == 0)
            {
                sscanf(line, "Uid:\t%d", &uid);
                break ;
            }
        }
        fclose(f);
        if (uid != (int)pw->pw_uid)
            continue ;
        snprintf(fd_path, sizeof(fd_path), "/proc/%s/fd", proc_entry->d_name);
        fd_dir = opendir(fd_path);
        if (!fd_dir)
            continue ;
        while ((fd_entry = readdir(fd_dir)) != NULL)
        {
            snprintf(full_fd, sizeof(full_fd), "%s/%s", fd_path, fd_entry->d_name);
            len = readlink(full_fd, target, sizeof(target) - 1);
            if (len == -1)
                continue ;
            target[len] = '\0';
            printf("PID %s -> fd %s -> %s\n",
                proc_entry->d_name, fd_entry->d_name, target);
        }
        closedir(fd_dir);
    }
    closedir(proc_dir);
}

void    opt_l(t_lsof *data)
{
    DIR             *dir;
    struct dirent   *entry;
    char            status_path[256];
    FILE            *f;
    char            line[256];

    (void)data;
    dir = opendir("/proc");
    if (!dir)
    {
        perror("opendir -l");
        return ;
    }
    while ((entry = readdir(dir)) != NULL)
    {
        if (atoi(entry->d_name) > 0)
        {
            snprintf(status_path, sizeof(status_path),
                "/proc/%s/status", entry->d_name);
            f = fopen(status_path, "r");
            if (!f)
                continue ;
            while (fgets(line, sizeof(line), f))
            {
                if (strncmp(line, "Uid:", 4) == 0)
                {
                    printf("PID %s -> %s", entry->d_name, line);
                    break ;
                }
            }
            fclose(f);
        }
    }
    closedir(dir);
}

void    opt_F(t_lsof *data)
{
    DIR             *proc_dir;
    DIR             *fd_dir;
    struct dirent   *proc_entry;
    struct dirent   *fd_entry;
    char            fd_path[256];
    char            full_fd[512];
    char            target[512];
    ssize_t         len;

    (void)data;
    proc_dir = opendir("/proc");
    if (!proc_dir)
    {
        perror("opendir -F");
        return ;
    }
    while ((proc_entry = readdir(proc_dir)) != NULL)
    {
        if (atoi(proc_entry->d_name) <= 0)
            continue ;
        printf("p%s\n", proc_entry->d_name);
        snprintf(fd_path, sizeof(fd_path), "/proc/%s/fd", proc_entry->d_name);
        fd_dir = opendir(fd_path);
        if (!fd_dir)
            continue ;
        while ((fd_entry = readdir(fd_dir)) != NULL)
        {
            if (atoi(fd_entry->d_name) < 0)
                continue ;
            snprintf(full_fd, sizeof(full_fd), "%s/%s", fd_path, fd_entry->d_name);
            len = readlink(full_fd, target, sizeof(target) - 1);
            if (len == -1)
                continue ;
            target[len] = '\0';
            printf("f%s\n", fd_entry->d_name);
            printf("n%s\n", target);
        }
        closedir(fd_dir);
    }
    closedir(proc_dir);
}

void    opt_S(t_lsof *data)
{
    if (data->timeout < 2)
    {
        fprintf(stderr, "Timeout minimum : 2 secondes. Valeur forcée à 2.\n");
        data->timeout = 2;
    }
    printf("Timeout défini à %d secondes\n", data->timeout);
}

void    opt_D(t_lsof *data)
{
    DIR             *proc_dir;
    DIR             *fd_dir;
    struct dirent   *proc_entry;
    struct dirent   *fd_entry;
    char            fd_path[256];
    char            full_fd[512];
    char            target[512];
    ssize_t         len;

    proc_dir = opendir("/proc");
    if (!proc_dir)
    {
        perror("opendir -D");
        return ;
    }
    while ((proc_entry = readdir(proc_dir)) != NULL)
    {
        if (atoi(proc_entry->d_name) <= 0)
            continue ;
        snprintf(fd_path, sizeof(fd_path), "/proc/%s/fd", proc_entry->d_name);
        fd_dir = opendir(fd_path);
        if (!fd_dir)
            continue ;
        while ((fd_entry = readdir(fd_dir)) != NULL)
        {
            snprintf(full_fd, sizeof(full_fd), "%s/%s", fd_path, fd_entry->d_name);
            len = readlink(full_fd, target, sizeof(target) - 1);
            if (len == -1)
                continue ;
            target[len] = '\0';
            if (strncmp(target, data->directory, strlen(data->directory)) == 0)
                printf("PID %s -> fd %s -> %s\n",
                    proc_entry->d_name, fd_entry->d_name, target);
        }
        closedir(fd_dir);
    }
    closedir(proc_dir);
}

void    opt_e(t_lsof *data)
{
    if (data->e_mode == 1)
    {
        printf("Mode +e : les erreurs sur '%s' seront ignorées\n",
            data->e_path);
    }
    else
    {
        printf("Mode -e : les erreurs sur '%s' seront signalées\n",
            data->e_path);
        DIR *dir = opendir(data->e_path);
        if (!dir)
        {
            fprintf(stderr, "Erreur : impossible d'accéder à '%s'\n",
                data->e_path);
            return ;
        }
        printf("Accès à '%s' OK\n", data->e_path);
        closedir(dir);
    }
}

void    opt_k(t_lsof *data)
{
    FILE    *f;
    char    line[512];
    int     count;

    f = fopen(data->kernel_map, "r");
    if (!f)
    {
        fprintf(stderr, "Impossible d'ouvrir le fichier kernel : %s\n",
            data->kernel_map);
        return ;
    }
    count = 0;
    printf("Lecture du fichier kernel map : %s\n", data->kernel_map);
    while (fgets(line, sizeof(line), f))
    {
        printf("%s", line);
        count++;
        if (count >= 20)
        {
            printf("... (et plus)\n");
            break ;
        }
    }
    fclose(f);
}

void    opt_w(t_lsof *data)
{
    if (data->w_mode == 1)
    {
        FILE *devnull = fopen("/dev/null", "w");
        if (!devnull)
        {
            perror("fopen /dev/null");
            return ;
        }
        if (dup2(fileno(devnull), STDERR_FILENO) == -1)
        {
            perror("dup2");
            fclose(devnull);
            return ;
        }
        fclose(devnull);
        printf("Mode +w : tous les avertissements sont désactivés\n");
    }
    else
    {
        FILE *terminal = fopen("/dev/tty", "w");
        if (!terminal)
        {
            perror("fopen /dev/tty");
            return ;
        }
        if (dup2(fileno(terminal), STDERR_FILENO) == -1)
        {
            perror("dup2");
            fclose(terminal);
            return ;
        }
        fclose(terminal);
        printf("Mode -w : tous les avertissements sont activés\n");
    }
}

int     main(int argc, char **argv)
{
    t_lsof  data;
    int     i;

    memset(&data, 0, sizeof(t_lsof));
    data.timeout   = 15;
    data.w_mode    = -1;
    data.fd_filter = -1;

    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s [options]\n", argv[0]);
        fprintf(stderr, "  -d <fd>        Filtrer par file descriptor\n");
        fprintf(stderr, "  -u <user>      Filtrer par utilisateur\n");
        fprintf(stderr, "  -l             Afficher les UIDs numériques\n");
        fprintf(stderr, "  -F             Format parseable\n");
        fprintf(stderr, "  -S <sec>       Timeout (min 2s)\n");
        fprintf(stderr, "  -D <dir>       Chercher dans un répertoire\n");
        fprintf(stderr, "  +e/-e <path>   Ignorer/signaler erreurs filesystem\n");
        fprintf(stderr, "  -k <file>      Fichier kernel map\n");
        fprintf(stderr, "  +w/-w          Supprimer/activer les warnings\n");
        return (1);
    }

    i = 1;
    while (i < argc)
    {
        if (strcmp(argv[i], "-d") == 0 && i + 1 < argc)
        {
            data.fd_filter = atoi(argv[++i]);
            opt_d(&data);
        }
        else if (strcmp(argv[i], "-u") == 0 && i + 1 < argc)
        {
            data.username = argv[++i];
            opt_u(&data);
        }
        else if (strcmp(argv[i], "-l") == 0)
        {
            opt_l(&data);
        }
        else if (strcmp(argv[i], "-F") == 0)
        {
            opt_F(&data);
        }
        else if (strcmp(argv[i], "-S") == 0 && i + 1 < argc)
        {
            data.timeout = atoi(argv[++i]);
            opt_S(&data);
        }
        else if (strcmp(argv[i], "-D") == 0 && i + 1 < argc)
        {
            data.directory = argv[++i];
            opt_D(&data);
        }
        else if (strcmp(argv[i], "+e") == 0 && i + 1 < argc)
        {
            data.e_mode = 1;
            data.e_path = argv[++i];
            opt_e(&data);
        }
        else if (strcmp(argv[i], "-e") == 0 && i + 1 < argc)
        {
            data.e_mode = 0;
            data.e_path = argv[++i];
            opt_e(&data);
        }
        else if (strcmp(argv[i], "-k") == 0 && i + 1 < argc)
        {
            data.kernel_map = argv[++i];
            opt_k(&data);
        }
        else if (strcmp(argv[i], "+w") == 0)
        {
            data.w_mode = 1;
            opt_w(&data);
        }
        else if (strcmp(argv[i], "-w") == 0)
        {
            data.w_mode = 0;
            opt_w(&data);
        }
        else
        {
            fprintf(stderr, "Option inconnue : %s\n", argv[i]);
        }
        i++;
    }
    return (0);
}






