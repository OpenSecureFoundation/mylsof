

#include <pwd.h>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "options.h"

/* Cassandra écrit ton code ici — options : -d -u -l -F -S */

     void opt_d(t_lsof*data) {
   DIR *dir;
   struct dirent *entry;
   char path[256];
  
   snprintf(path,sizeof(path), "/proc/%d/fd",data->pid);
   dir = opendir(path);
   if(!dir)
   {
     perror("opendir -d");
     return;
   }
   while((entry = readdir(dir)) !=NULL)
   {
     if (atoi(entry->d_name) == data->fd_filter)
     {
       snprint(fd_path,sizeof(fd_path),"%s/%s",path,entry->d_name);
       printf("fd: %s-> %s\n",entry->d_name,fd_path);
     }
   }
    closedir(dir);
}
   void opt_u(t_lsof*data)
{
  struct passwd *pw;
   DIR    *dir;
   struct dirent  *entry;

   pw = getpwnam(data->username);
   if(!pw)
   {
    fprintf(stderr, "utilisateur introuvable: %s/n",data->username);
    return;
   }
   dir = opendir("/proc");
   if(!dir)
   {
     perror("opendir -u");
     return;
   }
   while((entry = readdir(dir))!=NULL)
   {
    if(atoi(entry->d_name)> 0)
       printf("PID %s appartient a uid %d\n",entry->d_name,pw->pw_uid);
   }
    closedir(dir);
}
   void opt_l(t_lsof*data)
 {
   DIR *dir;
  struct dirent *entry;
  char status_path[256];
  FILE *f;
  char line[256];

  dir = opendir("/proc");
  if (!dir)
  {
   perror("opendir -l");
   return;
  }
  while ((entry = readdir(dir))!=NULL)
  {
    if (atoi(entry->d_name)>0)
    {
      snprintf(status_path,sizeof(status_path),"/proc/%s/status",entry->d_name);
      f = fopen(status_path,"r");
      if(!f)
         continue;
      while(fgets(line,sizeof(line),f))
      {
        if(strncmp(line, "Uid:",4) ==0)
        {
          printf("PID %s ->%s",entry->d_name,line);
          break;
        }
      }
      fclose(f);
    }
  }
   closedir(dir);
}


  void opt_F(t_lsof *data)
{
  DIR *dir;
  struct dirent *entry;
  char fd_path[256];
  char target[256];
  ssize_t len;

  snprintf(fd_path,sizeof(fd_path),"/proc/%d/fd",data->pid);
  dir = opendir(fd_path);
  if(!dir)
  {
    perror("opendir -F");
    return;
  }
  printf("p%d\n",data->pid);
  while((entry = readdir(dir) !=NULL)
  {
   if(atoi(entry->d_name)>=0)
   {
     char full[512];
     snprintf(full,sizeof(full),"%s/%s",fd_path,entry->d_name);
     len = readlink(full,target,sizeof(target)-1);
     if (len!=1)
     {
       target[len]='\0';
       printf("f%s\n",entry->d_name);
       printf("n%s\n",target);
     }
   }
  }
   closedir(dir);
}
   void opt_S(t_lsof *data)
{
  if(data->timeout < 2)
  {
    fprintf(stderr,"Timeout minimum:2 secondes.valeur forcee a 2.\n");
    data->timeout = 2;
  }
   printf("timeout defini a %d secondes\n",data->timeout);
}

int gerer_options_Cassandra(int argc, char *argv[]) {
    /* Cassandra ajoute tes if ici */
    return 0;
}
