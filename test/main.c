//  Affiche le nombre d’occurrences de chaque mot qui apparait dans un et un
//    seul (de manière exclusive) des fichiers dont les noms figurent sur la
//    ligne de commande avec les options demandées.
//  - Utilisation d'une table de hachage par défaut, un arbre avl si demandé
//    dans les options.
//  - Un mot est, par défaut, une séquence de longueur maximale de caractères
//    n’appartenant pas à la classe isspace, et donc de longueur indéfinie.
//  - Options et fichiers peuvent être entremêlés sur la ligne de commande.
//    Des formes réduites des options courtes peuvent également être utilisées
//    ainsi que l'emlpoi de préfixe pour les arguments ou les noms des options
//    longues.
//  - Utilisation d'une structure infos contenant les informations des mots
//    lus sur les fichiers. Le mot est contenu dans le tableau fléxible strfl
//    si le champ str est égale à NULL. Le mot est pointé par str sinon.
//  Résultats:
//  - Les résultats sont affichés en colonnes sur la sortie standard. Les
//    colonnes sont séparées par le caractère de tabulation horizontale.
//  - Une ligne d’en-tête est affiché pour indiquer les noms des fichiers : le
//    nom du premier fichier est affiché dans la deuxième colonne, celui du
//    deuxième dans la troisième, etc. Le nom du « fichier restricteur » est
//    affiché dans la première colonne de la ligne d’en-tête si cette option
//    est entrée en ligne de commande.
//  - Pour les lignes suivantes, un mot est affiché dans la première colonne,
//    son nombre d’occurrences dans le fichier dans lequel il apparait à
//    l’exclusion de tous les autres dans la colonne associée au fichier.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <locale.h>
#include "hashtable.h"
#include "holdall.h"
#include "bst.h"
#include "word.h"
#include "opt.h"

#define EXEC "xwc"
#define WRNG_MSG EXEC ": Word from file '%s' cut: '%s...'.\n"
#define START_READ "--- starts reading for "
#define R_FILE_MSG "restrict FILE"
#define PRINT_NUMF "#%ld FILE"
#define END_READ "--- ends reading for "
#define MISSING_FILE "Missing filename after '" OPT__LONG "'.\n"
#define EMPTY_ ""
#define WRITE_ERROR "A write error occured\n"
#define HELP_ERROR "Try '"EXEC                                                 \
  " "OPT__LONG OPT_HELP_L "' for more information.\n"

#define DEFAULT_COLOR() {                                                      \
    fprintf(stderr, "\033[0m");                                                \
}
#define MAKE_COLOR() {                                                         \
    fprintf(stderr, "\033[30;47m");                                            \
}

#define PRINT_START_READING(num) {                                             \
    MAKE_COLOR();                                                              \
    fprintf(stderr, START_READ);                                               \
    if (num == 0) {                                                            \
      fprintf(stderr, R_FILE_MSG);                                             \
    } else {                                                                   \
      fprintf(stderr, PRINT_NUMF, num);                                        \
    }                                                                          \
    DEFAULT_COLOR();                                                           \
    fprintf(stderr, "\n");                                                     \
}

#define PRINT_ENDS_READING(num) {                                              \
    MAKE_COLOR();                                                              \
    fprintf(stderr, END_READ);                                                 \
    if (num == 0) {                                                            \
      fprintf(stderr, R_FILE_MSG);                                             \
    } else {                                                                   \
      fprintf(stderr, PRINT_NUMF, num);                                        \
    }                                                                          \
    DEFAULT_COLOR();                                                           \
    fprintf(stderr, "\n");                                                     \
}

#define ERROR_FILE_MSG(msg, file) {                                            \
    fprintf(stderr, EXEC " %s \'%s\'.\n", msg, file);                          \
}

#define ERROR__MSG(msg) {                                                      \
    fprintf(stderr, "%s" HELP_ERROR, msg);                                     \
}

#define CUT_WARNING(cutword, filename) {                                       \
    fprintf(stderr, WRNG_MSG, filename,                                        \
    cutword);                                                                  \
}

#define ON_FILE_ERROR_GOTO(msg, file, s1, label) {                             \
    ERROR_FILE_MSG(msg, file);                                                 \
    free(s1);                                                                  \
    goto label;                                                                \
}

#define FERROR "An error occured while treating file: "
#define FCLOSE_ERROR "An error occured while closing file: "
#define ALLOC_ERROR "An allocation error occured"
#define FOPEN_ERROR "Can't open for reading file: "

#define ERROR_ -1
#define SUCCESS_FILE 0
#define OPT__SHORT  '-'
#define OPT__LONG   "--"

#define APPEAR_MULT -1
#define APPEAR_R_FILE 0

typedef struct {
  long int cptr;
  long int fnum;
  char *str;
  char strfl[];
} infos;

typedef union {
  bst *bt;
  hashtable *ht;
} type_process;

//  str_hashfun : l'une des fonctions de pré-hachage conseillées par Kernighan
//    et Pike pour les chaines de caractères.
static size_t str_hashfun(const char *s);

//  scptr_display : affiche sur la sortie standard le champ str de info, fnum
//    tabulations et le champ cptr suivi de la fin de ligne selon si le
//    le champ ap de info valide une condition ou pas. Renvoie zéro en cas de
//    succès, une valeur non nulle en cas d'échec.
static int scptr_display(infos *info);

//  comp1: Renvoie une fonction de tri sur les deux champs str de s1 et s2.
//    (tri lexicographique).
int comp1(const infos *s1, const infos *s2);

//  comp1: Renvoie une fonction de tri sur les deux champs cptr de s1 et s2.
//    (tri numérique).
int comp2(const infos *s1, const infos *s2);

//  comp1: Renvoie une fonction de tri décroissant sur les deux champs str de
//    s1 et s2. (tri décroissant lexicographique).
int r_comp1(const infos *s1, const infos *s2);

//  comp1: Renvoie une fonction de tri décroissant sur les deux champs cptr de
//    s1 et s2. (tri décroissant numérique).
int r_comp2(const infos *s1, const infos *s2);

//  bst_comp: Renvoie une fonction de comparaison pour les arbres avl
int bst_comp(const infos *s1, const infos *s2);

//  rfree : libère la zone mémoire pointée par ptr et renvoie zéro.
int rfree(infos *ptr);

//  xwc_process: Pour chaque mot lu dans le fichier de nom file et de numéro
//    numf stocké dans w et traité selon les options définies dans opt, tente
//    de le rajouter avec ses informations à la structure de donnée associée
//    à tp et au holdall ha dans le cas où ce mot n'apparait pas dans cette
//    structure. Sinon si il apparait avec un numéro de fichier différent,
//    lui associe un numéro de fichier négatif. Incrémente son compteur sinon.
//    Renvoie zéro en cas de traitement réussi, une valeur non nulle sinon.
int xwc_process(const char *file, long int numf, type_process *tp, holdall *ha,
    options *opt, word *w);

int main(int argc, char *argv[]) {
  int r = EXIT_SUCCESS;
  const char *file_tab[argc];
  options *opt = options_default();
  long int nb_file = 0;
  for (int k = 1; k < argc; ++k) {
    const char *a = argv[k];
    if (strcmp(a, READ_FROM_STDIN) == 0) {
      file_tab[nb_file + 1] = STDIN_FILE;
      ++nb_file;
    } else {
      if (a[0] == OPT__SHORT) {
        if (strcmp(a, OPT__LONG) == 0) {
          if (k + 1 < argc) {
            file_tab[nb_file + 1] = argv[k + 1];
            ++nb_file;
            ++k;
            continue;
          } else {
            ERROR__MSG(MISSING_FILE);
            goto dispose_opt;
          }
        }
        opreturn opt_return = options_manager(argc, argv, &k, opt);
        if (opt_return != OP_RETURN_SUCCESS) {
          if (opt_return != OP_RETURN_HLP_USG_VER) {
            ERROR__MSG(EMPTY_);
          }
          goto dispose_opt;
        }
        continue;
      }
      file_tab[nb_file + 1] = a;
      ++nb_file;
    }
  }
  if (nb_file == 0) {
    file_tab[nb_file + 1] = STDIN_FILE;
    ++nb_file;
  }
  type_process tp;
  word *w = word_init();
  if (opt_get_word_process(opt)) {
    tp.ht = hashtable_empty((int (*)(const void *, const void *))strcmp,
        (size_t (*)(const void *))str_hashfun);
  } else {
    tp.bt = bst_empty((int (*)(const void *, const void *))bst_comp);
  }
  holdall *ha = holdall_empty();
  if ((tp.bt == NULL && tp.ht == NULL)
      || ha == NULL
      || w == NULL) {
    goto error_capacity;
  }
  const char *r_file = opt_get_r_file(opt);
  bool is_r_file = r_file != NULL;
  if (is_r_file) {
    file_tab[0] = r_file;
  }
  for (long int k = !is_r_file; k <= nb_file; ++k) {
    if (strcmp(file_tab[k], STDIN_FILE) == 0) {
      PRINT_START_READING(k);
    }
    if (xwc_process(file_tab[k], k, &tp, ha, opt, w) != 0) {
      goto error;
    }
  }
  if (is_r_file) {
    printf("%s", r_file);
  }
  for (long int k = 1; k <= nb_file; ++k) {
    printf("\t");
    printf("%s", file_tab[k]);
  }
  printf("\n");
#if defined HOLDALL_WANT_EXT && HOLDALL_WANT_EXT != 0
  int sort = opt_get_sort(opt);
  if (sort != 0) {
    setlocale(LC_COLLATE, "");
    int (*compar)(const infos *, const infos *);
    bool r_rev_sort = opt_get_reversed(opt);
    if (r_rev_sort) {
      compar = sort > 0 ? r_comp1 : r_comp2;
    } else {
      compar = sort > 0 ? comp1 : comp2;
    }
    holdall_sort(ha, (int (*)(const void *, const void *))compar);
  }
#endif
  if (holdall_apply(ha, (int (*)(void *))scptr_display) != 0) {
    fprintf(stderr, WRITE_ERROR);
    goto error;
  }
  goto dispose;
error_capacity:
  fprintf(stderr, ALLOC_ERROR);
  goto error;
error:
  r = EXIT_FAILURE;
  goto dispose;
dispose:
  word_dispose(&w);
  if (opt_get_word_process(opt)) {
    hashtable_dispose(&tp.ht);
  } else {
    bst_dispose(&tp.bt);
  }
  if (ha != NULL) {
    holdall_apply(ha, (int (*)(void *))rfree);
  }
  holdall_dispose(&ha);
dispose_opt:
  options_dispose(&opt);
  return r;
}

size_t str_hashfun(const char *s) {
  size_t h = 0;
  for (const unsigned char *p = (const unsigned char *) s; *p != '\0'; ++p) {
    h = 37 * h + *p;
  }
  return h;
}

int scptr_display(infos *si) {
  if (si->fnum > 0) {
    if (printf("%s\t", si->strfl) < 0) {
      return ERROR_;
    }
    for (int i = 1; i < si->fnum; ++i) {
      if (printf("\t") < 0) {
        return ERROR_;
      }
    }
    if (printf("%ld\n", si->cptr) < 0) {
      return ERROR_;
    }
  }
  return 0;
}

int comp1(const infos *s1, const infos *s2) {
  return strcoll(s1->strfl, s2->strfl);
}

int bst_comp(const infos *s1, const infos *s2) {
  const char *p1 = s1->str != NULL ? s1->str : s1->strfl;
  const char *p2 = s2->str != NULL ? s2->str : s2->strfl;
  return strcmp(p1, p2);
}

int r_comp1(const infos *s1, const infos *s2) {
  return -comp1(s1, s2);
}

int comp2(const infos *s1, const infos *s2) {
  if (s1->cptr == s2->cptr) {
    return comp1(s1, s2);
  }
  return s1->cptr > s2->cptr ? 1 : -1;
}

int r_comp2(const infos *s1, const infos *s2) {
  if (s1->cptr == s2->cptr) {
    return comp1(s1, s2);
  }
  return s1->cptr > s2->cptr ? -1 : 1;
}

int rfree(infos *ptr) {
  if (ptr->str != NULL) {
    free(ptr->str);
  }
  free(ptr);
  return 0;
}

int xwc_process(const char *file, long int numf, type_process *tp, holdall *ha,
    options *opt, word *w) {
  FILE *f;
  bool is_stdin = false;
  int r = SUCCESS_FILE;
  bool is_r_file = opt_get_r_file(opt) == NULL ? false : true;
  bool test_ht = opt_get_word_process(opt);
  if (strcmp(file, STDIN_FILE) == 0) {
    f = stdin;
    is_stdin = true;
  } else {
    f = fopen(file, "rb");
    if (f == NULL) {
      ERROR_FILE_MSG(FOPEN_ERROR, file);
      goto error_file;
    }
  }
  int c;
  while ((c = fgetc(f)) != EOF) {
    word_renit(w);
    long int opt_i = opt_get_ivalue(opt);
    bool i_default = opt_i == I_VAL_DEFAULT;
    long int i_cpt = 0;
    while (c != EOF && !(opt_get_isblank_func(opt))(c)
        && (i_default || i_cpt < opt_i)) {
      if (word_add(w, c) == NULL) {
        goto error_file;
      }
      ++i_cpt;
      c = fgetc(f);
    }
    if (!i_default
        && (c != EOF && !(opt_get_isblank_func(opt))(c))) {
      CUT_WARNING(word_get(w), file);
      while ((c = fgetc(f)) != EOF && !(opt_get_isblank_func(opt))(c)) {
      }
    }
    if (!word_is_empty(w)) {
      char *s1 = word_get(w);
      infos *inf = NULL;
      if (test_ht) {
        inf = hashtable_search(tp->ht, s1);
      } else {
        infos i_tmp = {
          .str = s1
        };
        inf = bst_search(tp->bt, &i_tmp);
      }
      if (inf == NULL) {
        if ((!is_r_file || (is_r_file && numf == APPEAR_R_FILE))) {
          infos *i = malloc(sizeof *i + word_length(w) + 1);
          if (i == NULL) {
            ON_FILE_ERROR_GOTO(ALLOC_ERROR, file, NULL, error_file)
          }
          i->cptr = 1;
          i->fnum = numf;
          i->str = NULL;
          memcpy(i->strfl, s1, word_length(w) + 1);
          if (test_ht) {
            if (hashtable_add(tp->ht, i->strfl, i) == NULL) {
              ON_FILE_ERROR_GOTO(ALLOC_ERROR, file, i, error_file)
            }
          } else {
            if (bst_add_endofpath(tp->bt, i) == NULL) {
              ON_FILE_ERROR_GOTO(ALLOC_ERROR, file, i, error_file)
            }
          }
          if (holdall_put(ha, i) != 0) {
            ON_FILE_ERROR_GOTO(ALLOC_ERROR, file, i, error_file)
          }
        }
      } else {
        if (inf->fnum == numf) {
          ++inf->cptr;
        } else {
          if (is_r_file && inf->fnum == APPEAR_R_FILE) {
            inf->cptr = 1;
            inf->fnum = numf;
          } else {
            inf->fnum = APPEAR_MULT;
          }
        }
      }
    }
  }
  goto end;
error_file:
  r = ERROR_;
end:
  if (f != NULL) {
    if (!feof(f)) {
      ERROR_FILE_MSG(FERROR, file);
      r = ERROR_;
    }
    if (!is_stdin) {
      if (fclose(f) != 0) {
        ERROR_FILE_MSG(FCLOSE_ERROR, file);
        r = ERROR_;
      }
    } else {
      PRINT_ENDS_READING(numf);
      rewind(stdin);
    }
  }
  return r;
}
