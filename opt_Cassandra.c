

#include <pwd.h>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "options.h"

/* Cassandra écrit ton code ici — options : -d -u -l -F -S */

void filter_by_fd(const char *fd_list) {
   DIR *proc_dir;
   struct dirent *entry;
   char fd_path[512];
   proc_dir = opendir("/proc");
   if(!proc_dir)
      return ;
   while ((entry = readdir(proc_dir)) !=NULL
   {
     if (!is_numeric(entry->d_name))
         continue;
     snprintf(fd_path, sizeof(fd_path),
          "/proc/%s/fd" , entry->d_name);
     list_fds_matching(fd_path,fd_list);
   }
 
  closedir(proc_dir);

 }
void filter_by_user(const char *usename)      {
    
   struct passwd *pw;
   uid_t          target_uid;
   DIR            *proc_dir;
   struct dirent  *entry;

  pw = getpwnam(username);
  if(!pw)
  {
    fprintf(stderr, "mylsof: user not found: %s/n,username);
    return;
}
target_uid = pw->pw_uid;
proc_dir = opendir("/proc");
if (!proc_dir)
    return ;
while  ((entry =readdir(proc_dir)) !=NULL)
  {
    if(!is_numeric(entry->d_name))
        continue;
    print_if_owner(entry->d_name, target_uid);
  }
  closedir(proc_dir);
}
 
void option_l(uid_t uid, int show_numeric)
 {
  
   struct passwd *pw;

  if(show_numeric)
   {
     print("%-8u" , uid);
     return ;
   }
pw = getpwuid(uid);
if (pw)
    printf("%-8u",pw->pw_name);
else
   printf("%-8u" ,uid);
}

void option_F(char *fields)    { /* TODO */ }
void option_S(int sec)         { /* TODO */ }

int gerer_options_Cassandra(int argc, char *argv[]) {
    /* Cassandra ajoute tes if ici */
    return 0;
}
