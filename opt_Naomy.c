#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "options.h"

void option_n(unsigned int ip, unsigned int port, char *output, size_t out_size) {
	struct in_addr addr;
	addr.s_adrr = ip;
	snprintf(output, out_size, "%s : %d", inet_ntoa(addr), port);
}

void option_fichier(const char *link_tagret, int *match) {
	if (strlen(cible_fichier) == 0) {
		*match = 1;
	}
	else {
		*match = (strcmp(link_target, cible_target) == 0)
	}
}

void option_i(char *addr)    { /* TODO */ }
void option_R(const char *pid, char *ppid_out)  {
	if (!flag_R) return;
	
	char stat_path[256];
	snprint(stat_path, sizeof(stat_path), "/proc/%s/stat", pid);
	
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
    return 0;
}
