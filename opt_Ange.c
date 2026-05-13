#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/stat.h>
#include "options.h"


void	afficher_fichiers_pid(int pid)
{
	char		path[256];
	char		comm[256];
	FILE		*f;
	DIR		*fd_dir;
	struct dirent	*fd_entry;
	struct passwd	*pw;
	char		fd_path[512];
	char		link_target[512];
	int		len;
	char		user[64];

	/* Lire le nom du processus depuis /proc/PID/comm */
	snprintf(path, sizeof(path), "/proc/%d/comm", pid);
	f = fopen(path, "r");
	if (!f)
		return ;
	fgets(comm, sizeof(comm), f);
	fclose(f);
	comm[strcspn(comm, "\n")] = 0;

	/* Lire l'UID du processus depuis /proc/PID/status */
	snprintf(path, sizeof(path), "/proc/%d/status", pid);
	f = fopen(path, "r");
	if (!f)
		return ;
	uid_t uid = 0;
	char line[256];
	while (fgets(line, sizeof(line), f))
	{
		if (strncmp(line, "Uid:", 4) == 0)
		{
			sscanf(line, "Uid:\t%u", &uid);
			break ;
		}
	}
	fclose(f);

	/* Convertir l'UID en nom d'utilisateur */
	pw = getpwuid(uid);
	if (pw)
		strncpy(user, pw->pw_name, sizeof(user));
	else
		snprintf(user, sizeof(user), "%u", uid);

	/* Parcourir les descripteurs de fichiers dans /proc/PID/fd/ */
	snprintf(path, sizeof(path), "/proc/%d/fd", pid);
	fd_dir = opendir(path);
	if (!fd_dir)
		return ;
	while ((fd_entry = readdir(fd_dir)) != NULL)
	{
		if (fd_entry->d_name[0] == '.')
			continue ;
		snprintf(fd_path, sizeof(fd_path), "/proc/%d/fd/%s", pid, fd_entry->d_name);
		len = readlink(fd_path, link_target, sizeof(link_target) - 1);
		if (len < 0)
			continue ;
		link_target[len] = 0;
		printf("%-15s %-6d %-10s %-4s %-6s %s\n",
			comm, pid, user, fd_entry->d_name, "REG", link_target);
	}
	closedir(fd_dir);
}


void	option_p(int pid)
{
	printf("%-15s %-6s %-10s %-4s %-6s %s\n",
		"COMMAND", "PID", "USER", "FD", "TYPE", "NAME");
	afficher_fichiers_pid(pid);
}


void	option_c(char *name)
{
	DIR		*proc_dir;
	struct dirent	*entry;
	int		pid;
	char		path[256];
	char		comm[256];
	FILE		*f;

	proc_dir = opendir("/proc");
	if (!proc_dir)
	{
		perror("Erreur /proc");
		return ;
	}
	printf("%-15s %-6s %-10s %-4s %-6s %s\n",
		"COMMAND", "PID", "USER", "FD", "TYPE", "NAME");
	while ((entry = readdir(proc_dir)) != NULL)
	{
		/* On ne garde que les dossiers dont le nom est un nombre (= un PID) */
		pid = atoi(entry->d_name);
		if (pid <= 0)
			continue ;

		/* Lire le nom du processus */
		snprintf(path, sizeof(path), "/proc/%d/comm", pid);
		f = fopen(path, "r");
		if (!f)
			continue ;
		fgets(comm, sizeof(comm), f);
		fclose(f);
		comm[strcspn(comm, "\n")] = 0;

		/* Vérifier si le nom commence par ce qu'on cherche */
		if (strncmp(comm, name, strlen(name)) == 0)
			afficher_fichiers_pid(pid);
	}
	closedir(proc_dir);
}


void	option_C()
{
	printf("Option -C : pas encore implementee\n");
}

void	option_d_plus(char *dir)
{
	(void)dir;
	printf("Option +d : pas encore implementee\n");
}

void	option_r(int interval)
{
	(void)interval;
	printf("Option -r : pas encore implementee\n");
}


int	gerer_options_Ange(int argc, char *argv[])
{
	if (strcmp(argv[1], "-p") == 0 && argc >= 3)
	{
		option_p(atoi(argv[2]));
		return (1);
	}
	if (strcmp(argv[1], "-c") == 0 && argc >= 3)
	{
		option_c(argv[2]);
		return (1);
	}
	if (strcmp(argv[1], "-C") == 0)
	{
		option_C();
		return (1);
	}
	if (strcmp(argv[1], "+d") == 0 && argc >= 3)
	{
		option_d_plus(argv[2]);
		return (1);
	}
	if (strcmp(argv[1], "-r") == 0 && argc >= 3)
	{
		option_r(atoi(argv[2]));
		return (1);
	}
	return (0);
}
