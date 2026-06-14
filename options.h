#ifndef OPTIONS_H
#define OPTIONS_H

/* ═══════════════════════════════════════════════════════════
   FONCTION UTILITAIRE PARTAGÉE
   ═══════════════════════════════════════════════════════════ */
void	afficher_fichiers_pid(int pid);

/* ═══════════════════════════════════════════════════════════
   GESTION DES OPTIONS — 1 PAR MEMBRE
   retourne 1 = option traitée
   retourne 0 = pas mon option
   ═══════════════════════════════════════════════════════════ */
int	gerer_options_Ange(int argc, char *argv[]);      /* -p -c -C +d -r +|-E -A -v +|-M */
int	gerer_options_Cassandra(int argc, char *argv[]); /* -d -u -l -F -S +|-e -D -k +|-w  */
int	gerer_options_Naomy(int argc, char *argv[]);     /* -n -fichier -i -R -b +|-m -T -V -x */
int	gerer_options_Salif(int argc, char *argv[]);     /* -P -g -U -s -X +|-f -Z +c -N    */
int	gerer_options_Sobrin(int argc, char *argv[]);    /* -t -a +D -o -K +|-L -h -O -z    */

/* ═══════════════════════════════════════════════════════════
   OPTIONS DE ANGE
   ═══════════════════════════════════════════════════════════ */
void	option_p(int pid);
void	option_c(char *name);
void	option_C(void);
void	option_d_plus(char *dir);
void	option_r(int interval);
void	option_E(int afficher_erreurs);
void	option_A(void);
void	option_v(void);
void	option_M(int afficher_kernel);

/* ═══════════════════════════════════════════════════════════
   OPTIONS DE CASSANDRA
   ═══════════════════════════════════════════════════════════ */
void	option_d_minus(char *dir);
void	option_u(char *user);
void	option_l(void);
void	option_F(char *fields);
void	option_S(int sec);
void	option_e(int activer);
void	option_D(char *dir);
void	option_k(void);
void	option_w(int activer);

/* ═══════════════════════════════════════════════════════════
   OPTIONS DE NAOMY
   ═══════════════════════════════════════════════════════════ */
void	option_n(void);
void	option_fichier(char *f);
void	option_i(char *addr);
void	option_R(void);
void	option_b(void);
void	option_m(int activer);
void	option_T(void);
void	option_V(void);
void	option_x(void);

/* ═══════════════════════════════════════════════════════════
   OPTIONS DE SALIF
   ═══════════════════════════════════════════════════════════ */
void	option_P(void);
void	option_g(char *grp);
void	option_U(void);
void	option_s(char *proto);
void	option_X(void);
void	option_f(int activer);
void	option_Z(void);
void	option_c_salif(char *cmd);
void	option_N(void);

/* ═══════════════════════════════════════════════════════════
   OPTIONS DE SOBRIN
   ═══════════════════════════════════════════════════════════ */
void	option_t(void);
void	option_a(void);
void	option_D_plus(char *dir);
void	option_o(void);
void	option_K(void);
void	option_L(int activer);
void	option_h(void);
void	option_O(void);
void	option_z(void);

#endif
