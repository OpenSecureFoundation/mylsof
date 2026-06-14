#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <pwd.h>
#include <signal.h>
#include "options.h"

/* ════════════════════════════════════════════════════
   FONCTION DE BASE
   Lit le nom du processus depuis /proc/PID/comm
   ════════════════════════════════════════════════════ */
static int	lire_comm(int pid, char *comm, int taille)
{
	char	path[256];
	FILE	*f;

	snprintf(path, sizeof(path), "/proc/%d/comm", pid);
	f = fopen(path, "r");
	if (!f)
		return (0);
	fgets(comm, taille, f);
	fclose(f);
	comm[strcspn(comm, "\n")] = 0;
	return (1);
}

/* ════════════════════════════════════════════════════
   FONCTION DE BASE
   Lit le nom d'utilisateur depuis /proc/PID/status
   ════════════════════════════════════════════════════ */
static void	lire_user(int pid, char *user, int taille)
{
	char		path[256];
	char		line[256];
	FILE		*f;
	uid_t		uid;
	struct passwd	*pw;

	snprintf(path, sizeof(path), "/proc/%d/status", pid);
	f = fopen(path, "r");
	if (!f) { snprintf(user, taille, "?"); return ; }
	uid = 0;
	while (fgets(line, sizeof(line), f))
		if (strncmp(line, "Uid:", 4) == 0)
		{ sscanf(line, "Uid:\t%u", &uid); break ; }
	fclose(f);
	pw = getpwuid(uid);
	if (pw)
		strncpy(user, pw->pw_name, taille);
	else
		snprintf(user, taille, "%u", uid);
}

/* ════════════════════════════════════════════════════
   FONCTION UTILITAIRE PARTAGÉE
   Affiche tous les fichiers ouverts par un PID
   ════════════════════════════════════════════════════ */
void	afficher_fichiers_pid(int pid)
{
	char		comm[256];
	char		user[64];
	char		path[256];
	char		fd_path[512];
	char		cible[512];
	DIR		*fd_dir;
	struct dirent	*fd;
	int		len;

	if (!lire_comm(pid, comm, sizeof(comm)))
		return ;
	lire_user(pid, user, sizeof(user));
	snprintf(path, sizeof(path), "/proc/%d/fd", pid);
	fd_dir = opendir(path);
	if (!fd_dir)
		return ;
	while ((fd = readdir(fd_dir)) != NULL)
	{
		if (fd->d_name[0] == '.')
			continue ;
		snprintf(fd_path, sizeof(fd_path), "/proc/%d/fd/%s", pid, fd->d_name);
		len = readlink(fd_path, cible, sizeof(cible) - 1);
		if (len < 0)
			continue ;
		cible[len] = 0;
		printf("%-15s %-6d %-10s %-4s %-6s %s\n",
			comm, pid, user, fd->d_name, "REG", cible);
	}
	closedir(fd_dir);
}

/* ════════════════════════════════════════════════════
   OPTION -p : fichiers ouverts par un PID
   Exemple : ./mylsof -p 1234
   ════════════════════════════════════════════════════ */
void	option_p(int pid)
{
	printf("%-15s %-6s %-10s %-4s %-6s %s\n",
		"COMMAND", "PID", "USER", "FD", "TYPE", "NAME");
	afficher_fichiers_pid(pid);
}

/* ════════════════════════════════════════════════════
   OPTION -c : fichiers des processus dont le nom
               commence par 'name'
   Exemple : ./mylsof -c bash
   ════════════════════════════════════════════════════ */
void	option_c(char *name)
{
	DIR		*proc;
	struct dirent	*entry;
	int		pid;
	char		comm[256];

	proc = opendir("/proc");
	if (!proc) { perror("Erreur /proc"); return ; }
	printf("%-15s %-6s %-10s %-4s %-6s %s\n",
		"COMMAND", "PID", "USER", "FD", "TYPE", "NAME");
	while ((entry = readdir(proc)) != NULL)
	{
		pid = atoi(entry->d_name);
		if (pid <= 0)
			continue ;
		if (!lire_comm(pid, comm, sizeof(comm)))
			continue ;
		if (strncmp(comm, name, strlen(name)) == 0)
			afficher_fichiers_pid(pid);
	}
	closedir(proc);
}

/* ════════════════════════════════════════════════════
   OPTION -C : affiche uniquement les fichiers avec
               un vrai chemin (pas sockets/pipes)
   Exemple : ./mylsof -C
   ════════════════════════════════════════════════════ */
void	option_C(void)
{
	DIR		*proc;
	struct dirent	*entry;
	int		pid;
	char		comm[256];
	char		path[256];
	char		fd_path[512];
	char		cible[512];
	DIR		*fd_dir;
	struct dirent	*fd;
	int		len;

	proc = opendir("/proc");
	if (!proc) { perror("Erreur /proc"); return ; }
	printf("%-15s %-6s %-4s %s\n", "COMMAND", "PID", "FD", "NAME");
	while ((entry = readdir(proc)) != NULL)
	{
		pid = atoi(entry->d_name);
		if (pid <= 0 || !lire_comm(pid, comm, sizeof(comm)))
			continue ;
		snprintf(path, sizeof(path), "/proc/%d/fd", pid);
		fd_dir = opendir(path);
		if (!fd_dir)
			continue ;
		while ((fd = readdir(fd_dir)) != NULL)
		{
			if (fd->d_name[0] == '.')
				continue ;
			snprintf(fd_path, sizeof(fd_path), "/proc/%d/fd/%s", pid, fd->d_name);
			len = readlink(fd_path, cible, sizeof(cible) - 1);
			if (len < 0) continue ;
			cible[len] = 0;
			/* On affiche seulement si le chemin commence par / */
			if (cible[0] == '/')
				printf("%-15s %-6d %-4s %s\n", comm, pid, fd->d_name, cible);
		}
		closedir(fd_dir);
	}
	closedir(proc);
}

/* ════════════════════════════════════════════════════
   OPTION +d : fichiers ouverts directement dans
               un dossier donné (pas récursif)
   Exemple : ./mylsof +d /tmp
   ════════════════════════════════════════════════════ */
void	option_d_plus(char *dir)
{
	DIR		*proc;
	struct dirent	*entry;
	int		pid;
	char		comm[256];
	char		path[256];
	char		fd_path[512];
	char		cible[512];
	DIR		*fd_dir;
	struct dirent	*fd;
	int		len;
	char		*reste;

	proc = opendir("/proc");
	if (!proc) { perror("Erreur /proc"); return ; }
	printf("%-15s %-6s %-4s %s\n", "COMMAND", "PID", "FD", "NAME");
	while ((entry = readdir(proc)) != NULL)
	{
		pid = atoi(entry->d_name);
		if (pid <= 0 || !lire_comm(pid, comm, sizeof(comm)))
			continue ;
		snprintf(path, sizeof(path), "/proc/%d/fd", pid);
		fd_dir = opendir(path);
		if (!fd_dir)
			continue ;
		while ((fd = readdir(fd_dir)) != NULL)
		{
			if (fd->d_name[0] == '.') continue ;
			snprintf(fd_path, sizeof(fd_path), "/proc/%d/fd/%s", pid, fd->d_name);
			len = readlink(fd_path, cible, sizeof(cible) - 1);
			if (len < 0) continue ;
			cible[len] = 0;
			/* Le fichier doit être dans dir et pas dans un sous-dossier */
			if (strncmp(cible, dir, strlen(dir)) == 0
				&& cible[strlen(dir)] == '/')
			{
				reste = cible + strlen(dir) + 1;
				if (strchr(reste, '/') == NULL)
					printf("%-15s %-6d %-4s %s\n",
						comm, pid, fd->d_name, cible);
			}
		}
		closedir(fd_dir);
	}
	closedir(proc);
}

/* ════════════════════════════════════════════════════
   OPTION -r : répéter l'affichage toutes les N sec
               Ctrl+C pour arrêter
   Exemple : ./mylsof -r 3
   ════════════════════════════════════════════════════ */
static int	continuer = 1;
static void	stopper(int sig) { (void)sig; continuer = 0; }

void	option_r(int interval)
{
	DIR		*proc;
	struct dirent	*entry;
	int		pid;

	signal(SIGINT, stopper);
	printf("Répétition toutes les %d sec. Ctrl+C pour arrêter.\n", interval);
	while (continuer)
	{
		printf("\n=== Mise à jour ===\n");
		printf("%-15s %-6s %-10s %-4s %-6s %s\n",
			"COMMAND", "PID", "USER", "FD", "TYPE", "NAME");
		proc = opendir("/proc");
		if (!proc) return ;
		while ((entry = readdir(proc)) != NULL)
		{
			pid = atoi(entry->d_name);
			if (pid > 0)
				afficher_fichiers_pid(pid);
		}
		closedir(proc);
		sleep(interval);
	}
	printf("\nArrêt.\n");
}

/* ════════════════════════════════════════════════════
   OPTION +E / -E : afficher ou masquer les erreurs
   +E = affiche les erreurs  |  -E = les cache
   Exemple : ./mylsof +E   ou   ./mylsof -E
   ════════════════════════════════════════════════════ */
void	option_E(int afficher_erreurs)
{
	DIR		*proc;
	struct dirent	*entry;
	int		pid;
	char		comm[256];
	char		path[256];
	char		fd_path[512];
	char		cible[512];
	DIR		*fd_dir;
	struct dirent	*fd;
	int		len;

	proc = opendir("/proc");
	if (!proc) { perror("Erreur /proc"); return ; }
	printf("%-15s %-6s %-4s %s\n", "COMMAND", "PID", "FD", "NAME");
	while ((entry = readdir(proc)) != NULL)
	{
		pid = atoi(entry->d_name);
		if (pid <= 0 || !lire_comm(pid, comm, sizeof(comm)))
			continue ;
		snprintf(path, sizeof(path), "/proc/%d/fd", pid);
		fd_dir = opendir(path);
		if (!fd_dir)
		{
			/* Si afficher_erreurs = 1, on affiche l'erreur */
			if (afficher_erreurs)
				fprintf(stderr, "Erreur : PID %d inaccessible\n", pid);
			continue ;
		}
		while ((fd = readdir(fd_dir)) != NULL)
		{
			if (fd->d_name[0] == '.') continue ;
			snprintf(fd_path, sizeof(fd_path), "/proc/%d/fd/%s", pid, fd->d_name);
			len = readlink(fd_path, cible, sizeof(cible) - 1);
			if (len < 0)
			{
				if (afficher_erreurs)
					fprintf(stderr, "Erreur : fd %s PID %d\n",
						fd->d_name, pid);
				continue ;
			}
			cible[len] = 0;
			printf("%-15s %-6d %-4s %s\n", comm, pid, fd->d_name, cible);
		}
		closedir(fd_dir);
	}
	closedir(proc);
}

/* ════════════════════════════════════════════════════
   OPTION -A : afficher uniquement les sockets réseau
   Exemple : ./mylsof -A
   ════════════════════════════════════════════════════ */
void	option_A(void)
{
	DIR		*proc;
	struct dirent	*entry;
	int		pid;
	char		comm[256];
	char		path[256];
	char		fd_path[512];
	char		cible[512];
	DIR		*fd_dir;
	struct dirent	*fd;
	int		len;

	proc = opendir("/proc");
	if (!proc) { perror("Erreur /proc"); return ; }
	printf("%-15s %-6s %-4s %s\n", "COMMAND", "PID", "FD", "NAME");
	while ((entry = readdir(proc)) != NULL)
	{
		pid = atoi(entry->d_name);
		if (pid <= 0 || !lire_comm(pid, comm, sizeof(comm)))
			continue ;
		snprintf(path, sizeof(path), "/proc/%d/fd", pid);
		fd_dir = opendir(path);
		if (!fd_dir) continue ;
		while ((fd = readdir(fd_dir)) != NULL)
		{
			if (fd->d_name[0] == '.') continue ;
			snprintf(fd_path, sizeof(fd_path), "/proc/%d/fd/%s", pid, fd->d_name);
			len = readlink(fd_path, cible, sizeof(cible) - 1);
			if (len < 0) continue ;
			cible[len] = 0;
			/* Afficher seulement socket: et pipe: */
			if (strncmp(cible, "socket:", 7) == 0
				|| strncmp(cible, "pipe:", 5) == 0)
				printf("%-15s %-6d %-4s %s\n", comm, pid, fd->d_name, cible);
		}
		closedir(fd_dir);
	}
	closedir(proc);
}

/* ════════════════════════════════════════════════════
   OPTION -v : afficher la version du programme
   Exemple : ./mylsof -v
   ════════════════════════════════════════════════════ */
void	option_v(void)
{
	printf("mylsof version 1.0\n");
	printf("Projet ING3 SRT — OpenSecureFoundation\n");
}

/* ════════════════════════════════════════════════════
   OPTION +M / -M : afficher ou masquer les tâches
                    kernel (processus système PID < 10)
   +M = affiche tout  |  -M = cache les PID < 10
   Exemple : ./mylsof +M   ou   ./mylsof -M
   ════════════════════════════════════════════════════ */
void	option_M(int afficher_kernel)
{
	DIR		*proc;
	struct dirent	*entry;
	int		pid;

	proc = opendir("/proc");
	if (!proc) { perror("Erreur /proc"); return ; }
	printf("%-15s %-6s %-10s %-4s %-6s %s\n",
		"COMMAND", "PID", "USER", "FD", "TYPE", "NAME");
	while ((entry = readdir(proc)) != NULL)
	{
		pid = atoi(entry->d_name);
		if (pid <= 0) continue ;
		if (!afficher_kernel && pid < 10) continue ;
		afficher_fichiers_pid(pid);
	}
	closedir(proc);
}

/* ════════════════════════════════════════════════════
   GERER_OPTIONS_ANGE
   Vérifie si l'option tapée est une option d'Ange
   Retourne 1 si oui, 0 sinon
   ════════════════════════════════════════════════════ */
int	gerer_options_Ange(int argc, char *argv[])
{
	if (strcmp(argv[1], "-p") == 0 && argc >= 3)
		{ option_p(atoi(argv[2])); return (1); }
	if (strcmp(argv[1], "-c") == 0 && argc >= 3)
		{ option_c(argv[2]); return (1); }
	if (strcmp(argv[1], "-C") == 0)
		{ option_C(); return (1); }
	if (strcmp(argv[1], "+d") == 0 && argc >= 3)
		{ option_d_plus(argv[2]); return (1); }
	if (strcmp(argv[1], "-r") == 0 && argc >= 3)
		{ option_r(atoi(argv[2])); return (1); }
	if (strcmp(argv[1], "+E") == 0)
		{ option_E(1); return (1); }
	if (strcmp(argv[1], "-E") == 0)
		{ option_E(0); return (1); }
	if (strcmp(argv[1], "-A") == 0)
		{ option_A(); return (1); }
	if (strcmp(argv[1], "-v") == 0)
		{ option_v(); return (1); }
	if (strcmp(argv[1], "+M") == 0)
		{ option_M(1); return (1); }
	if (strcmp(argv[1], "-M") == 0)
		{ option_M(0); return (1); }
	return (0);
}
