//  Partie implantation du module opt.

#include "opt.h"
#include "word.h"
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#define L_OPT '-'

// HELP_MESSAGE-----------------------------------------------------------------
#define PROG_DESC                                                              \
  "Exclusive word counting. Print the number of occurrences of each word"      \
  " that\nappears in one and only one of given text FILES.\n\n"                \
  "A word is, by default, a maximum length sequence of characters that do "    \
  "not\nbelong to the white-space characters set.\n\n"                         \
  "Results are displayed in columns on the standard output. "                  \
  "Columns are separated\nby the tab character. Lines are terminated by"       \
  "the end-of-line character. A\nheader line shows the FILE names:"            \
  " the name of the first FILE appears in the\nsecond column, that of the"     \
  " second in the third, and so on. For the following\nlines, a word"          \
  " appears in the first column, its number of occurrences in the FILE\n"      \
  "in which it appears to the exclusion of all others in the column"           \
  " associated with\nthe FILE. No tab characters are written on a line"        \
  " after the number of\noccurrences.\n\n"                                     \
  "Read the standard input when no FILE is given or for any FILE which is\n"   \
  " \"-\". In such cases, \"\" is displayed in the column associated with\n"   \
  "the FILE on the header line.\n\n"                                           \
  "The locale specified by the environment affects sort order. Set\n"          \
  "'LC_ALL=C' to get the traditional sort order that uses native byte\n"       \
  "values.\n\n"                                                                \
  "Mandatory arguments to long options are mandatory for short options too."   \
  "\n\n"
#define PROG_INFO "Program Information\n"
#define HELP_DESC  "  -?, --help   Print this help message and exit.\n"
#define USAGE_DESC  "      --usage   Print a short usage message and exit.\n"
#define VERSION_DESC "      --version   Print version information.\n\n"
#define PROCESS  "Processing\n"
#define AVL_DESC "  -a\t\t\tSame as --words-processing=avl-binary-tree.\n\n"
#define HT_DESC "  -h\t\t\tSame as --words-processing=hash-table.\n\n"
#define WT_PROCESS_DESC                                                        \
  "  -w, --words-processing=TYPE\tProcess the words according\n"               \
  "\t\t\tto the data structure specified by TYPE. The\n"                       \
  "\t\t\tavailable values for TYPE are self explanatory:\n"                    \
  "\t\t\t'avl-binary-tree' and 'hash-table'. Default is\n"                     \
  "\t\t\t'hash-table'.\n"
#define INPUT_C "Input Control\n"
#define I_VAL_DESC                                                             \
  "  -i, --initial=VALUE\tSet the maximal number of significant\n"             \
  "\t\t\t  initial letters for words to VALUE. 0 means\n"                      \
  "\t\t\t  without limitation. Default is 0.\n\n"
#define PUNC_DESC                                                              \
  "  -p, --punctuation-like-space\tMake the punctuation characters\n"          \
  "\t\t\t  play the same role as white-space\n"                                \
  "\t\t\t  characters in the meaning of words.\n\n"
#define RFILE_DESC                                                             \
  "  -r, --restrict=FILE\tLimit the counting to the set of\n"                  \
  "\t\t\t  words that appear in FILE. FILE is displayed\n"                     \
  "\t\t\t  in the first column of the header line. If\n"                       \
  "\t\t\t  FILE is \"-\", read words from the standard\n"                      \
  "\t\t\t  input; in this case, \"\" is displayed in\n"                        \
  "\t\t\t  first column of the header line.\n"
#define OUTPUT_CONTROL "Output Control\n"
#define LSORT_DESC "  -l\t\tSame as --sort=lexicographical.\n\n"
#define NSORT_DESC "  -n\t\tSame as --sort=numeric.\n\n"
#define NO_SORT_DESC "  -S\t\tSame as --sort=none.\n\n"
#define SORT_DESC                                                              \
  "  -s, --sort=TYPE   Sort the results in ascending order,\n"                 \
  "\t\tby default, according to TYPE. The available\n"                         \
  "\t\tvalues for TYPE are: 'lexicographical', sort\n"                         \
  "\t\ton words, 'numeric', sort on number of\n"                               \
  "\t\toccurrences, first key, and words, second\n"                            \
  "\t\tkey, and 'none', don't try to sort, take it\n"                          \
  "\t\tas it comes. Default is 'none'.\n\n"
#define REV_DESC                                                               \
  "  -R, --reverse\tSort in descending order on the single or\n"               \
  "\t\tfirst key\n"
#define OPT_USAGE  "usage"
#define OPT_VERSION "version"
//END_HELP_MESSAGE--------------------------------------------------------------

#define LSORT "sort"
#define LWORD_PROCESS "words-processing"
#define LMAX_L_VAL "initial"
#define LPUNCT "punctuation-like-space"
#define LREV "reverse"
#define LRESTRICT "restrict"

#define TYPE_AVL "avl-binary-tree"
#define TYPE_HT "hash-table"

#define TYPE_S_LEXICO "lexicographical"
#define TYPE_S_NUM "numeric"
#define TYPE_S_NONE "none"

#define WORD_PROCESS_AVL 'a'
#define WORD_PROCESS_HT 'h'
#define WORD_PROCESS_TYPE 'w'

#define SORT_LEXICO 'l'
#define SORT_NUM 'n'
#define SORT_NONE 'S'
#define SORT_TYPE 's'
#define SORT_REVERSED 'R'
#define RESTRICTED 'r'
#define AMBG_LETTER "n"

#define SORT_L_CODE 1
#define SORT_N_CODE -1
#define SORT_NONE_CODE 0

#define PUNCT_SPACE 'p'
#define MAX_L_VAL 'i'
#define OPT_HELP  '?'

#define VERSION_DATE "13/05/2024"
#define USAGE "Usage: xwc [OPTION]... [FILE]..."

int errno;

struct options {
  int sort_type;
  bool reversed_sort;
  bool word_process_ht;
  long int value;
  int (*isblank)(int);
  const char *r_file;
};

options *options_default() {
  options *opt = malloc(sizeof *opt);
  if (opt == NULL) {
    return NULL;
  }
  opt->sort_type = SORT_NONE_CODE;
  opt->reversed_sort = false;
  opt->word_process_ht = true;
  opt->value = I_VAL_DEFAULT;
  opt->isblank = isspace;
  opt->r_file = NULL;
  return opt;
}

void options_dispose(options **opt_ptr) {
  if (*opt_ptr == NULL) {
    return;
  }
  free(*opt_ptr);
  *opt_ptr = NULL;
}

//  is_space_or_punct : Renvoie 1 si le caractère c est un espace ou une
//    ponctuation, 0 sinon.
static int is_space_or_punct(int c) {
  return isspace(c) || ispunct(c);
}

//  op__help: affiche sur la sortie standard les informations d'aide
//    du programme et renvoie un entier positif.
static int op__help() {
  fprintf(stdout,
      USAGE "\n\n"
      PROG_DESC
      PROG_INFO
      HELP_DESC
      USAGE_DESC
      VERSION_DESC
      PROCESS
      AVL_DESC
      HT_DESC
      WT_PROCESS_DESC
      INPUT_C
      I_VAL_DESC
      PUNC_DESC
      RFILE_DESC
      OUTPUT_CONTROL
      LSORT_DESC
      NSORT_DESC
      NO_SORT_DESC
      SORT_DESC
      REV_DESC
      );
  return OP_RETURN_HLP_USG_VER;
}

#define EXEC_ "xwc - "

//  op_usage: affiche sur la sortie standard un message d'usage du programme
static int op_usage() {
  fprintf(stdout, EXEC_ USAGE "\n");
  return OP_RETURN_HLP_USG_VER;
}

//  op_ver: affiche sur la sortie standard un message contenant la date de la
//    derniére version du programme
static int op_ver() {
  fprintf(stdout, EXEC_ VERSION_DATE "\n");
  return OP_RETURN_HLP_USG_VER;
}

#define NOT_PREF -1

//  l_prefix: Renvoie la longueur du plus long préfixe commun si s1 et s2 en
//    partage. Un entier négatif si arg est vrai et que s1 n'est pas un préfixe
//    de s2.
static int l_prefix(const char *s1, const char *s2, bool arg_) {
  int length = 0;
  const char *p = s1;
  const char *q = s2;
  while (*p != '\0' && *q != '\0') {
    if (*p != *q) {
      return arg_ ? NOT_PREF : length;
    }
    length++;
    p++;
    q++;
  }
  return length;
}

//  get_arg_pos: Cherche le premier caractére d'un argument d'option pour
//    l'affecter à s et met l'identificateur d'espace arg_ à true. Renvoie une
//    valeur représentant une erreur si on atteint argc. Une valeur nulle sinon.
static int get_arg_pos(const char **s, int **k, bool *arg_, int argc,
    char **argv) {
  if (**s == '\0') {
    if (**k + 1 == argc) {
      ERROR_MSG(MISSING_ARG, argv[**k]);
      return OP_RETURN_INVALID_ARGUMENT;
    }
    **k += 1;
    *s = argv[**k];
    *arg_ = true;
  }
  return 0;
}

//  short_options_manager : Tente d'apporter des modifications dans les champs
//    de opt selon si on trouve une ou plusieur options courtes à la position k
//    de argv et renvoie une valeur nulle. En cas de rencontre d'une option
//    d'aide, l'a traite et renvoie une valeur propre à ce cas. Renvoie sinon
//    une valeur non nulle propre à l'erreur rencontrée.
static opreturn short_options_manager(int argc, char **argv, int *k,
    options *opt) {
  errno = 0;
  char *p = &argv[*k][1];
  while (*p != '\0') {
    int arg_l = 0;
    bool arg_ = false;
    switch (*p) {
      case OPT_HELP:
        op__help();
        return OP_RETURN_HLP_USG_VER;
      case RESTRICTED:
        const char *s = p + 1;
        int test = get_arg_pos(&s, &k, &arg_, argc, argv);
        if (test == OP_RETURN_INVALID_ARGUMENT) {
          return OP_RETURN_INVALID_ARGUMENT;
        }
        arg_l = (int) strlen(s);
        opt->r_file = strcmp(s, READ_FROM_STDIN) == 0 ? STDIN_FILE : s;
        break;
      case WORD_PROCESS_AVL:
        opt->word_process_ht = false;
        break;
      case WORD_PROCESS_HT:
        opt->word_process_ht = true;
        break;
      case WORD_PROCESS_TYPE:
        s = p + 1;
        test = get_arg_pos(&s, &k, &arg_, argc, argv);
        if (test == OP_RETURN_INVALID_ARGUMENT) {
          return OP_RETURN_INVALID_ARGUMENT;
        }
        int rl = l_prefix(s, TYPE_AVL, arg_);
        if (rl > 0) {
          opt->word_process_ht = false;
          arg_l = rl;
          break;
        }
        rl = l_prefix(s, TYPE_HT, arg_);
        if (rl > 0) {
          opt->word_process_ht = true;
          arg_l = rl;
          break;
        }
        ERROR_MSG(PROCESS_UNKNOWN, argv[*k]);
        return OP_RETURN_INVALID_ARGUMENT;
      case SORT_LEXICO:
        opt->sort_type = SORT_L_CODE;
        break;
      case SORT_NUM:
        opt->sort_type = SORT_N_CODE;
        break;
      case SORT_NONE:
        opt->sort_type = SORT_NONE_CODE;
        break;
      case SORT_TYPE:
        s = p + 1;
        test = get_arg_pos(&s, &k, &arg_, argc, argv);
        if (test == OP_RETURN_INVALID_ARGUMENT) {
          return OP_RETURN_INVALID_ARGUMENT;
        }
        if (strcmp(s, AMBG_LETTER) == 0) {
          ERROR_MSG(AMB_ARG, argv[*k]);
          return OP_RETURN_INVALID_ARGUMENT;
        }
        rl = l_prefix(s, TYPE_S_LEXICO, arg_);
        if (rl > 0) {
          opt->sort_type = SORT_L_CODE;
          arg_l = rl;
          break;
        }
        rl = l_prefix(s, TYPE_S_NUM, arg_);
        if (rl > 0) {
          opt->sort_type = SORT_N_CODE;
          arg_l = rl;
          break;
        }
        rl = l_prefix(s, TYPE_S_NONE, arg_);
        if (rl > 0) {
          opt->sort_type = SORT_NONE_CODE;
          arg_l = rl;
          break;
        }
        ERROR_MSG(SORT_UNKNOWN, argv[*k]);
        return OP_RETURN_INVALID_ARGUMENT;
      case SORT_REVERSED:
        opt->reversed_sort = true;
        break;
      case PUNCT_SPACE:
        opt->isblank = is_space_or_punct;
        break;
      case MAX_L_VAL:
        s = p + 1;
        test = get_arg_pos(&s, &k, &arg_, argc, argv);
        if (test == OP_RETURN_INVALID_ARGUMENT) {
          return OP_RETURN_INVALID_ARGUMENT;
        }
        char *endptr;
        long int n = strtol(s, &endptr, 10);
        if (s == endptr || (arg_ && *endptr != '\0')) {
          ERROR_MSG(MAX_VAL_ERR, argv[*k]);
          return OP_RETURN_INVALID_ARGUMENT;
        }
        if (errno != 0) {
          ERROR_MSG(OVERFLOW, argv[*k]);
          return OP_RETURN_INVALID_ARGUMENT;
        }
        p = endptr - 1;
        opt->value = n;
        arg_l = 0;
        break;
      default:
        ERROR_MSG(ERR_OP, argv[*k]);
        return OP_RETURN_UNKNOWN_OP;
    }
    p += (arg_) ? 1 : arg_l + 1;
  }
  return OP_RETURN_SUCCESS;
}

//  ll_prefix: Renvoie un pointeur vers le premier caractére qui suit le plus
//    long préfixe commun si les deux options longues s1 et s2 en partage.
//    NULL sinon.
static void *ll_prefix(const void *s1, const void *s2) {
  const char *p = s1;
  const char *q = s2;
  while (*p != '\0' && *p != '=') {
    if (*p != *q) {
      return NULL;
    }
    p++;
    q++;
  }
  return (void *) p;
}

//  long_options_manager : En cas de rencontre d'une option d'aide, la
//    traite et renvoie une valeur propre à ce cas. Tente sinon d'apporter
//    des modifications dans les champs de opt selon si on trouve une option
//    longue à la position arg. Renvoie une valeur nulle en cas de réussite,
//    une valeur non nulle propre à l'erreur rencontrée sinon.
static opreturn long_options_manager(char *arg, options *opt) {
  char *p = arg + 2;
  if (strncmp(p, OPT_HELP_L, strlen(p)) == 0) {
    return op__help();
  }
  if (strncmp(p, OPT_VERSION, strlen(p)) == 0) {
    return op_ver();
  }
  if (strncmp(p, OPT_USAGE, strlen(p)) == 0) {
    return op_usage();
  }
  if (strncmp(p, LPUNCT, strlen(p)) == 0) {
    opt->isblank = is_space_or_punct;
    return 0;
  }
  if (strncmp(p, LREV, strlen(p)) == 0) {
    opt->reversed_sort = true;
    return 0;
  }
  char *q = ll_prefix(p, LSORT);
  if (q != NULL) {
    if (*q != '=' || *(++q) == '\0') {
      ERROR_MSG(MISSING_ARG, arg);
      return OP_RETURN_INVALID_ARGUMENT;
    }
    if (ll_prefix(q, TYPE_S_LEXICO) != NULL) {
      opt->sort_type = SORT_L_CODE;
      return OP_RETURN_SUCCESS;
    }
    if (ll_prefix(q, TYPE_S_NUM) != NULL) {
      opt->sort_type = SORT_N_CODE;
      return OP_RETURN_SUCCESS;
    }
    if (ll_prefix(q, TYPE_S_NONE) != NULL) {
      opt->sort_type = SORT_NONE_CODE;
      return OP_RETURN_SUCCESS;
    }
    ERROR_MSG(SORT_UNKNOWN, arg);
    return OP_RETURN_INVALID_ARGUMENT;
  }
  q = ll_prefix(p, LWORD_PROCESS);
  if (q != NULL) {
    if (*q != '=' || *(++q) == '\0') {
      ERROR_MSG(MISSING_ARG, arg);
      return OP_RETURN_INVALID_ARGUMENT;
    }
    if (ll_prefix(q, TYPE_AVL) != NULL) {
      opt->word_process_ht = false;
      return OP_RETURN_SUCCESS;
    }
    if (ll_prefix(q, TYPE_HT) != NULL) {
      opt->word_process_ht = true;
      return OP_RETURN_SUCCESS;
    }
    ERROR_MSG(PROCESS_UNKNOWN, arg);
    return OP_RETURN_INVALID_ARGUMENT;
  }
  q = ll_prefix(p, LRESTRICT);
  if (q != NULL) {
    if (*q == '\0' || *(++q) == '\0') {
      ERROR_MSG(MISSING_ARG, arg);
      return OP_RETURN_INVALID_ARGUMENT;
    }
    opt->r_file = strcmp(q, READ_FROM_STDIN) == 0 ? STDIN_FILE : q;
    return OP_RETURN_SUCCESS;
  }
  q = ll_prefix(p, LMAX_L_VAL);
  if (q != NULL) {
    if (*q != '=' || *(++q) == '\0') {
      ERROR_MSG(MISSING_ARG, arg);
      return OP_RETURN_INVALID_ARGUMENT;
    }
    char *endptr;
    long int n = strtol(q, &endptr, 10);
    if (*q == *endptr || *endptr != '\0') {
      ERROR_MSG(MAX_VAL_ERR, arg);
      return OP_RETURN_INVALID_ARGUMENT;
    }
    if (errno != 0) {
      ERROR_MSG(OVERFLOW, arg);
      return OP_RETURN_INVALID_ARGUMENT;
    }
    opt->value = n;
    return OP_RETURN_SUCCESS;
  }
  ERROR_MSG(ERR_OP, arg);
  return OP_RETURN_UNKNOWN_OP;
}

opreturn options_manager(int argc, char **argv, int *k, options *opt) {
  if (argv[*k][1] != L_OPT) {
    return short_options_manager(argc, argv, k, opt);
  }
  return long_options_manager(argv[*k], opt);
}

int opt_get_sort(options *opt) {
  return opt->sort_type;
}

bool opt_get_reversed(options *opt) {
  return opt->reversed_sort;
}

bool opt_get_word_process(options *opt) {
  return opt->word_process_ht;
}

int(*opt_get_isblank_func(options * opt)) (int) {
  return opt->isblank;
}

long int opt_get_ivalue(options *opt) {
  return opt->value;
}

const char *opt_get_r_file(options *opt) {
  return opt->r_file;
}
