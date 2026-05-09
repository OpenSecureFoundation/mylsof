#ifndef OPTIONS_H
#define OPTIONS_H

/* ═══════════════════════════════════
   FONCTION UTILITAIRE PARTAGÉE
   ═══════════════════════════════════ */
void afficher_fichiers_pid(int pid);

/* ═══════════════════════════════════
   GESTION DES OPTIONS — 1 PAR MEMBRE
   retourne 1 = option traitée
   retourne 0 = pas mon option
   ═══════════════════════════════════ */
int gerer_options_Ange(int argc, char *argv[]);      /* -p -c -C +d -r      */
int gerer_options_Cassandra(int argc, char *argv[]); /* -d -u -l -F -S      */
int gerer_options_Naomy(int argc, char *argv[]);     /* -n fichier -i -R -b */
int gerer_options_Salif(int argc, char *argv[]);     /* -P -g -U -s -X      */
int gerer_options_Sobrin(int argc, char *argv[]);    /* -t -a +D -o -K      */

/* ═══════════════════════════════════
   OPTIONS DE ANGE
   ═══════════════════════════════════ */
void option_p(int pid);
void option_c(char *name);
void option_C();
void option_d_plus(char *dir);
void option_r(int interval);

/* ═══════════════════════════════════
   OPTIONS DE CASSANDRA
   ═══════════════════════════════════ */
void option_d_minus(char *dir);
void option_u(char *user);
void option_l();
void option_F(char *fields);
void option_S(int sec);

/* ═══════════════════════════════════
   OPTIONS DE NAOMY
   ═══════════════════════════════════ */
void option_n();
void option_fichier(char *f);
void option_i(char *addr);
void option_R();
void option_b();

/* ═══════════════════════════════════
   OPTIONS DE SALIF
   ═══════════════════════════════════ */
void option_P();
void option_g(char *grp);
void option_U();
void option_s(char *proto);
void option_X();

/* ═══════════════════════════════════
   OPTIONS DE SOBRIN
   ═══════════════════════════════════ */
void option_t();
void option_a();
void option_D_plus(char *dir);
void option_o();
void option_K();

#endif
