#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>


int show_ppid = 0;
char target_file[512] = "";
int verbose = 0;
int items_found = 0;
int show_tcp_state = 0;
int filter_i_active = 0;
int filter_plus_m_active = 0;
char mount_point[512] = "";

void option_R(const char *pid) {
    
    char path[256];
    char line[256];
    FILE *fp;
    
    snprintf(path, sizeof(path), "/proc/%s/status", pid);
    fp = fopen(path, "r");
    if (fp) {
        while (fgets(line, sizeof(line), fp)) {
            if (strncmp(line, "PPID:", 5) == 0) {
                
                line[strcspn(line, "\n")] = 0; 
                printf(" %-10s", line + 6);
                break;
            }
        }
        fclose(fp);
    } else {
        printf(" %-10s", "-");
    }
}


void option_fichier(const char *symlink_target, const char *search_file, int *match) {
    
    if (strlen(search_file) > 0) {
        if (strstr(symlink_target, search_file) != NULL) {
            *match = 1;
        } else {
            *match = 0;
        }
    } else {
        *match = 1;
    }
}


void option_b() {
    // Implémentation complexe : nécessiterait d'utiliser O_NONBLOCK lors des open()
    // ou d'utiliser des threads/select() avec timeout pour éviter de bloquer sur stat().

}

void option_n() {
    // Désactiverait la résolution DNS.
    // Au lieu d'utiliser gethostbyaddr(), on afficherait l'IP brute.
}


int option_i(const char *inode_str) {
    FILE *fp = fopen("/proc/net/tcp", "r");
    if (fp == NULL) return 0;

    char line[512];
    fgets(line, sizeof(line), fp); 

    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, inode_str) != NULL) {
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}


void option_T(const char *inode_str) {
    FILE *fp = fopen("/proc/net/tcp", "r");
    if (fp == NULL) return;

    char line[512];
    fgets(line, sizeof(line), fp);

    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, inode_str) != NULL) {
            char st[4];
            
            if (sscanf(line, "%*s %*s %*s %2s", st) == 1) {
                
                int state_code = (int)strtol(st, NULL, 16);
                
                switch(state_code) {
                    case 1:  printf(" (ESTABLISHED)"); break;
                    case 2:  printf(" (SYN_SENT)"); break;
                    case 3:  printf(" (SYN_RECV)"); break;
                    case 4:  printf(" (FIN_WAIT1)"); break;
                    case 5:  printf(" (FIN_WAIT2)"); break;
                    case 6:  printf(" (TIME_WAIT)"); break;
                    case 7:  printf(" (CLOSE)"); break;
                    case 8:  printf(" (CLOSE_WAIT)"); break;
                    case 9:  printf(" (LAST_ACK)"); break;
                    case 10: printf(" (LISTEN)"); break;
                    case 11: printf(" (CLOSING)"); break;
                    default: printf(" (UNKNOWN_STATE: %s)", st); break;
                }
            }
            break;
        }
    }
    fclose(fp);
}

void option_x() {
    // Gérer les liens symboliques et les points de montage (utiliser lstat au lieu de stat).
}

void option_V() {
    verbose = 1;
}


void option_plus_m(const char *path) {
    FILE *fp = fopen("/proc/mounts", "r");
    if (fp == NULL) return;

    char line[1024];
    int is_mount = 0;

    while (fgets(line, sizeof(line), fp)) {
        char device[256], mnt_dir[256];
        
        if (sscanf(line, "%255s %255s", device, mnt_dir) == 2) {
            if (strcmp(mnt_dir, path) == 0) {
                is_mount = 1;
                break;
            }
        }
    }
    fclose(fp);

    if (is_mount == 0) {
        printf("mylsof: WARNING: %s is not a mount point.\n", path);
        exit(1); 
    }

    filter_plus_m_active = 1;
    strncpy(mount_point, path, sizeof(mount_point) - 1);
}


// --- Fonction principale d'exploration de /proc ---

void list_open_files() {
    DIR *proc_dir;
    struct dirent *proc_entry;
    
    printf("%-10s %-10s", "PID", "FD");
    if (show_ppid) printf(" %-10s", "PPID");
    printf(" %s\n", "NAME");

    proc_dir = opendir("/proc");
    if (proc_dir == NULL) {
        perror("Impossible d'ouvrir /proc");
        return;
    }

    while ((proc_entry = readdir(proc_dir)) != NULL) {
        if (proc_entry->d_type == DT_DIR && atoi(proc_entry->d_name) > 0) {
            char fd_path[256];
            DIR *fd_dir;
            struct dirent *fd_entry;

            snprintf(fd_path, sizeof(fd_path), "/proc/%s/fd", proc_entry->d_name);
            fd_dir = opendir(fd_path);

            if (fd_dir != NULL) {
                while ((fd_entry = readdir(fd_dir)) != NULL) {
                    if (strcmp(fd_entry->d_name, ".") != 0 && strcmp(fd_entry->d_name, "..") != 0) {
                        char link_path[512];
                        char target_path[1024];
                        ssize_t len;

                        snprintf(link_path, sizeof(link_path), "%s/%s", fd_path, fd_entry->d_name);
                        
                        len = readlink(link_path, target_path, sizeof(target_path) - 1);
                        if (len != -1) {
                            target_path[len] = '\0';
                            
                            int match = 0;
                            option_file(target_path, target_file, &match);
                            
                            if (filter_i_active == 1) {
   				 match = 0;
    
   				 if (strncmp(target_path, "socket:[", 8) == 0) {
       				     char inode_str[32];
       				     int inode_len = strlen(target_path) - 9;
        
       				     strncpy(inode_str, target_path + 8, inode_len);
       				     inode_str[inode_len] = '\0';
        

       				     if (option_i(inode_str) == 1) {
           				 match = 1;
        			     }
   				 }
			    }

			    if (filter_plus_m_active == 1) {
   				if (strncmp(target_path, mount_point, strlen(mount_point)) != 0) {
       				    match = 0; 
    				}
			    }

                            if (match) {
                                printf("%-10s %-10s", proc_entry->d_name, fd_entry->d_name);
                                if (show_ppid) {
                                    option_R(proc_entry->d_name);
                                }
                                printf(" %s\n", target_path);
				
				if (show_tcp_state == 1 && strncmp(target_path, "socket:[", 8) == 0) {
       				    char inode_str[32];
       				    int inode_len = strlen(target_path) - 9;
       				    strncpy(inode_str, target_path + 8, inode_len);
       				    inode_str[inode_len] = '\0';
       				    option_T(inode_str); 
   				}
				items_found++;
                            }
                        }
                    }
                }
                closedir(fd_dir);
            }
        }
    }
    closedir(proc_dir);
    if (verbose == 1 && strlen(target_file) > 0 && items_found == 0) {
        printf("mylsof: status error on %s: No such file or directory\n", target_file);
    }
}

int main(int argc, char *argv[]) {

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-R") == 0) {
            show_ppid = 1;
        }
	else if (strcmp(argv[i], "-V") == 0) {
            option_V();
        }
	else if (strcmp(argv[i], "-i") == 0) {
            filter_i_active = 1;
        }
	else if (strcmp(argv[i], "-T") == 0) {
            show_tcp_state = 1;
        }
	else if (strcmp(argv[i], "+m") == 0 && (i + 1) < argc) {
            option_plus_m(argv[i + 1]);
            i++;
        }
	else if (argv[i][0] != '-') {
            strncpy(target_file, argv[i], sizeof(target_file) - 1);
        }
    }

    list_open_files();
    return 0;
}
