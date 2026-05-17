#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "options.h"
#include <arpa/inet.h>
#include <unistd.h>

int flag_R = 0;
int flag_n = 0;
int flag_b = 0;
int flag_i = 0;
char cible_fichier[256] = "";

void option_n(unsigned int ip, unsigned int port, char *output, size_t out_size) {
	struct in_addr addr;
	addr.s_addr = ip;
	snprintf(output, out_size, "%s : %d", inet_ntoa(addr), port);
}

void option_fichier(const char *link_target, int *match) {
	if (strlen(cible_fichier) == 0) {
		*match = 1;
	}
	else {
		*match = (strcmp(link_target, cible_fichier) == 0);
	}
}

void option_i(const char *link_target, char *net_info, size_t size) {
	if (!flag_i) return;
	if (strncmp(link_target, "socket:[", 8) != 0) {
		net_info[0] = '\0';
		return;
	}
	long inode;
	sscanf(link_target, "socket : [%ld]", &inode);
	
	FILE *fp = fopen("/proc/net/tcp", "r");
	if (!fp) return;
	
	char line[256];
	while (fgets(line, sizeof(line), fp)) {
		unsigned int loc_ip, loc_port, rem_ip, rem_port;
		int state;
		long curr_inode;

		if (sscanf(line, "%*d : %X:%X %X:%X %X %*x:%*x %*x:%*x %*x %*d %*d %ld", &loc_ip, &loc_port, &rem_ip, &rem_port, &state, &curr_inode) == 6) {
			if (curr_inode == inode) {
				char loc_str[64], rem_str[64];
				option_n(loc_ip, loc_port, loc_str, sizeof(loc_str));
				option_n(rem_ip, rem_port, rem_str, sizeof(rem_str));
				
				const char *st = (state == 0x0A) ? "LISTEN" : (state == 0x01) ? "ESTABLISHED" : "UNKNOWN";
				snprintf(net_info, size, "TCP %s -> %s (%s)", loc_str, rem_str, st);
				break;
			}
		}
	}
	fclose(fp);
}

void option_R(const char *pid, char *ppid_out)  {
	if (!flag_R) return;
	
	char stat_path[256];
	snprintf(stat_path, sizeof(stat_path), "/proc/%s/stat", pid);
	
	FILE *f = fopen(stat_path, "r");
	if (f) {
		int p;
		char comm[256], state;
		
		fscanf(f, "%d (%255[^)]) %c %15s", &p, comm, &state, ppid_out);
		fclose(f);
	}
	else {
		strcpy (ppid_out, "?");
	}
}

void option_b(const char *fd_path, char *link_target, size_t size) {
	ssize_t len = readlink(fd_path, link_target, size -1);
	if (len != -1) {
		link_target[len] = '\0';
	}
	else {
		link_target[0] = '\0';
	}
}


int gerer_options_Naomy(int argc, char *argv[]) {
	for (int j = 1; j < argc; j++) {
		if (strcmp (argv[j], "-R") == 0) flag_R = 1;
		else if (strcmp (argv[j], "-n") == 0) flag_n = 1;
		else if (strcmp (argv[j], "-b") == 0) flag_b = 1;
		else if (strcmp (argv[j], "-i") == 0) flag_i = 1;
		else if (argv[j][0] != '-' ) {
			strncpy (cible_fichier, argv[j], sizeof(cible_fichier) - 1);
		} 
   	}
	return 0;
}

/*int main(int argc, char *argv[]) {
	printf("--- TEST DES OPTIONS ---\n");
	
	gerer_options_Naomy(argc, argv);
	printf("Options lues : R=%d, n=%d, b=%d, i=%d, fichier_cible='%s'\n\n", flag_R, flag_n, flag_b, flag_i, cible_fichier);

	char ppid[16] = "erreur";
	flag_R = 1;
	option_R("1", ppid);
	printf("Test Option R (PID 1) -> Son parent (PPID) est : %s\n", ppid);

	char link_target[256] = "";
	option_b("/proc/1/fd/1", link_target, sizeof(link_target));
	printf("Test Option b (sur /proc/1/fd/1) -> pointe vers : %s\n", link_target);

	int correspond = 0;
	strncpy(cible_fichier, "/dev/null", 256);
	option_fichier("/dev/null", &correspond);
	printf("Test Option fichier -> /dev/null correspond-il ? : %s\n", correspond ? "OUI" : "NON");
	
	return 0;

}*/
