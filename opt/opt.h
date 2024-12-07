//  opt : Un module permettant de gérer les options entrées en ligne de
//    commande, dont le traitement est mémorisé dans une structure nommée
//    « options ».

#ifndef OPT__H
#define OPT__H

#include <stdio.h>
#include <stdbool.h>

#define OPT_HELP_L  "help"
#define READ_FROM_STDIN "-"
#define STDIN_FILE "\"\""
#define I_VAL_DEFAULT 0

typedef enum {
  OP_RETURN_SUCCESS,
  OP_RETURN_HLP_USG_VER,
  OP_RETURN_UNKNOWN_OP,
  OP_RETURN_INVALID_ARGUMENT
} opreturn;

#define ERROR_MSG(msg, opt) {                                                  \
    fprintf(stderr, "xwc: %s \'%s\'.\n", msg, opt);                            \
}

#define MISSING_ARG "Missing argument"
#define AMB_ARG "Ambiguous argument"
#define PROCESS_UNKNOWN "Unknown word processing type"
#define OVERFLOW "Overflowing argument"
#define MAX_VAL_ERR "Invalid value for max length"
#define ERR_OP "Unrecognized option"
#define SORT_UNKNOWN "Unknown sort type"

//  struct options, options: type et nom de type d'un contrôleur regroupant
//    les informations nécessaires pour gérer les options entrées en ligne de
//    commande.
typedef struct options options;

//  options_default : tente d'allouer les ressources nécessaires pour gérer les
//    les options en leurs affectant des valeurs par défaut. Renvoie NULL en cas
//    de dépassement de capacité. Renvoie sinon un pointeur vers le contrôleur
//    associé à la structure d'options.
extern options *options_default();

//  options_manager : Tente d'initialiser les champs de opt avec les options
//    courtes ou l'option longue lue sur le flot pointé par argv[k]. Renvoie une
//    valeur nulle en cas de réussite, une valeur non nulle sinon.
extern opreturn options_manager(int argc, char **argv, int *k, options *opt);

//  options_dispose : sans effet si *opt_ptr vaut NULL. Libère sinon les
//    ressources allouées à la gestion des options associé à *opt_ptr
//    puis affecte NULL à *opt_ptr.
extern void options_dispose(options **opt_ptr);

//  opt_get_sort : Renvoie le champ associé à l'option de tri de opt.
extern int opt_get_sort(options *opt);

//  opt_get_reversed : Renvoie le champ associé à l'option de tri décroissant
//    de opt.
extern bool opt_get_reversed(options *opt);

//  opt_get_word_process : Renvoie le champ associé à l'option de traitement des
//    mot de opt.
extern bool opt_get_word_process(options *opt);

//  opt_get_word_process : Renvoie le champ associé à l'option de ponctuation
//    de opt.
extern int(*opt_get_isblank_func(options * opt)) (int);

//  opt_get_value : Renvoie le champ associé à l'option de la longeur max d'un
//    mot de opt.
extern long int opt_get_ivalue(options *opt);

//  opt_get_value : Renvoie le champ associé à l'option restrict de opt.
extern const char *opt_get_r_file(options *opt);

#endif
