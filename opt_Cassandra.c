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
int	gerer_options_Cassandra(int argc, char *argv[])
{
         t_lsof data;

         memset(&data,0, sizeof(t_lsof));
         data.timeout   = 15;
         data.w_mode    = -1;
         data.fd_filter = -1;

        if (strcmp(argv[1], "-d") == 0 && argc >= 3)
	       { data.fd_filter = atoi(argv[2]); opt_d(&data); return (1); }
        if (strcmp(argv[1], "-u") == 0 && argc >= 3)
	       { data.username = argv[2]; opt_u(&data); return (1); }
        if (strcmp(argv[1], "-l") == 0)
	       { opt_l(&data); return (1); }
        if (strcmp(argv[1], "-F") == 0)
	       { opt_F(&data); return (1); }
        if (strcmp(argv[1], "-S") == 0 && argc >= 3)
	       { data.timeout = atoi(argv[2]); opt_S(&data); return (1); }
	if (strcmp(argv[1], "-D") == 0 && argc >= 3)
	       { data.directory = argv[2]; opt_D(&data); return (1); }
       	if (strcmp(argv[1], "+e") == 0 && argc >= 3)
	       { data.e_mode = 1; data.e_path = argv[2]; opt_e(&data); return (1); }
	if (strcmp(argv[1], "-e") == 0 && argc >= 3)
	       { data.e_mode = 0; data.e_path = argv[2]; opt_e(&data); return (1); }
	if (strcmp(argv[1], "-k") == 0 && argc >= 3)
	       { data.kernel_map = argv[2]; opt_k(&data); return (1); }
	if (strcmp(argv[1], "+w") == 0)
	       { data.w_mode = 1; opt_w(&data); return (1); }
	if (strcmp(argv[1], "-w") == 0)
	       { data.w_mode = 0; opt_w(&data); return (1); }
	return (0);
}

