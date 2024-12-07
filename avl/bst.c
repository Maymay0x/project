//  bst.c : partie implantation d'un module polymorphe pour la spécification
//   ABINR du TDA ABinR(T).

#include "bst.h"

#define DISPLAY_ROTATION

//=== Type cbst ================================================================

//--- Définition cbst ----------------------------------------------------------

typedef struct cbst cbst;

struct cbst {
  cbst *next[2];
  const void *ref;
  int height;
};

//--- Raccourcis cbst ----------------------------------------------------------

#define EMPTY()     NULL
#define IS_EMPTY(p) ((p) == NULL)
#define LEFT(p)     ((p)->next[0])
#define RIGHT(p)    ((p)->next[1])
#define REF(p)      ((p)->ref)
#define HEIGHT(p) ((p)->height)
#define NEXT(p, d)  ((p)->next[(d) > 0])

//--- Divers -------------------------------------------------------------------

static size_t add__size_t(size_t x1, size_t x2) {
  return x1 + x2;
}

static int max__int(int x1, int x2) {
  return x1 > x2 ? x1 : x2;
}

static int min__int(int x1, int x2) {
  return x1 < x2 ? x1 : x2;
}

//--- Fonctions cbst -----------------------------------------------------------

//  DEFUN_CBST__MEASURE : définit la fonction récursive de nom « cbst__ ## fun »
//    et de paramètre un pointeur d'arbre binaire, qui renvoie zéro si l'arbre
//    est vide et « 1 + oper(r0, r1) » sinon, où r0 et r1 sont les valeurs
//    renvoyées par les appels récursifs de la fonction avec les pointeurs des
//    sous-arbres gauche et droit de l'arbre comme paramètres.
#define DEFUN_CBST__MEASURE(fun, oper)                                         \
  static size_t cbst__ ## fun(const cbst * p) {                                \
    return IS_EMPTY(p)                                                         \
      ? 0                                                                      \
      : 1 + oper(cbst__ ## fun(LEFT(p)), cbst__ ## fun(RIGHT(p)));             \
  }

//  cbst__size, cbst__height, cbst__distance : définition.

DEFUN_CBST__MEASURE(size, add__size_t)

static int cbst__height(const cbst *p) {
  return IS_EMPTY(p) ? 0 : HEIGHT(p);
}

static int cbst__distance(const cbst *p) {
  return IS_EMPTY(p) ? 0 : 1
    + min__int(cbst__distance(LEFT(p)), cbst__distance(RIGHT(p)));
}

//  cbst_update_height: Met à jour la hauteur du sous-arbre non vide associé à p
static void cbst__update_height(cbst *p) {
  HEIGHT(p) = 1 + max__int(cbst__height(RIGHT(p)), cbst__height(LEFT(p)));
}

//  cbst_balance:  renvoie l’équilibre du sous-arbre associé à p
static int cbst__balance(const cbst *p) {
  return IS_EMPTY(p) ? 0 : cbst__height(LEFT(p)) - cbst__height(RIGHT(p));
}

//  cbst__rotation_left: effectue une rotation gauche du sous-arbre non vide
//     associé à *pp dont le sous-arbre droit n'est pas vide. Et met à jour
//     les hauteurs des arbres impactés
static void cbst__rotation_left(cbst **pp) {
  cbst *p = *pp;
  *pp = RIGHT(p);
  RIGHT(p) = LEFT(*pp);
  cbst__update_height(p);
  LEFT(*pp) = p;
  cbst__update_height(*pp);
}

//  cbst__rotation_left: effectue une rotation droite du sous-arbre non vide
//     associé à *pp dont le sous-arbre droit n'est pas vide. Et met à jour
//     les hauteurs des arbres impactés
static void cbst__rotation_right(cbst **pp) {
  cbst *p = *pp;
  *pp = LEFT(p);
  LEFT(p) = RIGHT(*pp);
  cbst__update_height(p);
  RIGHT(*pp) = p;
  cbst__update_height(*pp);
}

//  cbst__rotation_left: effectue une rotation gauche droite du sous-arbre non
//     vide associé à *pp dont le sous-arbre droit n'est pas vide. Et met à jour
//     les hauteurs des arbres impactés
static void cbst__rotation_left_right(cbst **pp) {
  cbst__rotation_left(&LEFT(*pp));
  cbst__rotation_right(&(*pp));
}

//  cbst__rotation_left: effectue une rotation droite gauche du sous-arbre non
//     vide associé à *pp dont le sous-arbre droit n'est pas vide. Et met à jour
//     les hauteurs des arbres impactés
static void cbst__rotation_right_left(cbst **pp) {
  cbst__rotation_right(&RIGHT(*pp));
  cbst__rotation_left(&(*pp));
}

//  cbst_balancing: Met à jour la hauteur du sous-arbre non vide associé à *pp.
//    Renvoi 0 si le sous-arbre est équilibré. Sinon, effectue la rotation
//    nécessaire et l'affiche sur la sortie si la macrocanstante
//    DISPLAY_ROTATION est définie et renvoie une valeur non nulle.
static int cbst__balancing(cbst **pp) {
  cbst__update_height(*pp);
  int b = cbst__balance(*pp);
  int r = 0;
  if (b == 2) {
    if (cbst__balance(LEFT(*pp)) >= 0) {
      cbst__rotation_right(&(*pp));
    } else {
      cbst__rotation_left_right(&(*pp));
    }
    r = 1;
  } else if (b == -2) {
    if (cbst__balance(RIGHT(*pp)) <= 0) {
      cbst__rotation_left(&(*pp));
    } else {
      cbst__rotation_right_left(&(*pp));
    }
    r = 1;
  }
  return r;
}

//  cbst__dispose : libère les ressources allouées à l'arbre binaire pointé par
//    p.
static void cbst__dispose(cbst *p) {
  if (!IS_EMPTY(p)) {
    cbst__dispose(LEFT(p));
    cbst__dispose(RIGHT(p));
    free(p);
  }
}

//  cbst_add_endofpath : Recherche dans le sous-arbre binaire de recherche
//    associé à *pp la référence d'un objet égal à celui de référence ref au
//    sens de la fonction de comparaison. Si la recherche est positive, renvoie
//    la référence trouvée. Tente sinon d'ajouter la référence selon la méthode
//    de l'ajout en bout de chemin. Renvoie NULL en cas de dépassement de
//    capacité; renvoie ref sinon.
static void *cbst__add_endofpath(cbst **pp, const void *ref,
    int (*compar)(const void *, const void *)) {
  if (IS_EMPTY(*pp)) {
    *pp = malloc(sizeof **pp);
    if (pp == NULL) {
      return NULL;
    }
    REF(*pp) = ref;
    LEFT(*pp) = EMPTY();
    RIGHT(*pp) = EMPTY();
    HEIGHT(*pp) = 1;
    return (void *) ref;
  }
  int c = compar(ref, REF(*pp));
  if (c == 0) {
    return (void *) REF(*pp);
  }
  void *r = cbst__add_endofpath(&NEXT(*pp, c), ref, compar);
  if (r == (void *) ref) {
    cbst__balancing(&(*pp));
  }
  return r;
}

//  cbst__remove_max : Retire le noeud de valeur maximale du sous-arbre binaire
//    de recherche non vide associé à *pp.
static void *cbst__remove_max(cbst **pp) {
  if (IS_EMPTY(RIGHT(*pp))) {
    void *r = (void *) REF(*pp);
    cbst *p = *pp;
    *pp = LEFT(p);
    free(p);
    return r;
  }
  void *res = cbst__remove_max(&RIGHT(*pp));
  cbst__balancing(&(*pp));
  return res;
}

//  cbst__remove_root: Retire la racine du sous-arbre binaire de recherche non
//     vide associé à *pp si son sous-arbre gauche est vide. Retire sinon le
//     noeud de valeur maximale de son sous-arbre gauche et remplace la
//     référence de la racine par celle de la valeur maximale retirée.
static void cbst__remove_root(cbst **pp) {
  if (IS_EMPTY(LEFT(*pp))) {
    cbst *p = *pp;
    *pp = RIGHT(p);
    free(p);
    return;
  }
  REF(*pp) = cbst__remove_max(&LEFT(*pp));
  cbst__balancing(&(*pp));
}

//  cbst_remove_climbup_left : Recherche dans le sous-arbre binaire de recherche
//    associé à *pp la référence d'un objet égal à celui de référence ref au
//    sens de la fonction de comparaison. Si la recherche est négative,
//    renvoie NULL. Retire sinon la référence trouvée selon la méthode du
//    retrait par remontée gauche et renvoie la référence trouvée.
static void *cbst__remove_climbup_left(cbst **pp, const void *ref,
    int (*compar)(const void *, const void *)) {
  if (IS_EMPTY(*pp)) {
    return EMPTY();
  }
  int c = compar(ref, REF(*pp));
  if (c == 0) {
    cbst__remove_root(pp);
    return (void *) ref;
  }
  void *r = cbst__remove_climbup_left(&NEXT(*pp, c), ref, compar);
  if (r != NULL) {
    cbst__balancing(&(*pp));
  }
  return r;
}

//  cbst_search : Recherche dans l'arbre binaire de recherche associé à
//    *pp la référence d'un objet égal à celui de référence ref au sens de la
//    fonction de comparaison. Renvoie NULL si la recherche est négative,
//    la référence trouvée sinon.
static void *cbst__search(const cbst *p, const void *ref,
    int (*compar)(const void *, const void *)) {
  if (IS_EMPTY(p)) {
    return EMPTY();
  }
  int c = compar(ref, REF(p));
  if (c == 0) {
    return (void *) REF(p);
  }
  return cbst__search(NEXT(p, c), ref, compar);
}

//  cbsr__number : Calcule le numéro du nœud du sous-arbre binaire de recherche
//    associé à p dont la valeur est égale à celle de l'objet pointé par ref au
//    sens de la fonction de comparaison. Renvoie ce numéro si une telle valeur
//    existe. Renvoie sinon le numéro qu'aurait le nœud si la référence
//    ref était ajoutée à l'arbre.
static size_t cbst__number(const cbst *p, const void *ref,
    int (*compar)(const void *, const void *), size_t number) {
  int c;
  if (IS_EMPTY(p) || (c = compar(ref, REF(p))) == 0) {
    return number;
  }
  return cbst__number(NEXT(p, c), ref, compar,
      c > 0 ? number * 2 + 1 : number * 2);
}

//  cbsr__rank : Calcule le rang du nœud du sous-arbre binaire de recherche
//    associé à p dont la valeur est égale à celle de l'objet pointé par ref
//    au sens de la fonction de comparaison. Renvoie ce rang si une telle valeur
//    existe. Renvoie sinon le rang qu'aurait le nœud si la référence ref était
//    ajoutée à l'arbre.
static size_t cbst__rank(const cbst *p, const void *ref,
    int (*compar)(const void *, const void *), size_t rank) {
  if (IS_EMPTY(p)) {
    return rank;
  }
  int c = compar(ref, REF(p));
  if (c == 0) {
    return rank + cbst__size(LEFT(p));
  }
  return cbst__rank(NEXT(p, c), ref, compar, rank + (c < 0 ? 0
      : 1 + cbst__size(LEFT(p))));
}

#define REPR__TAB 4

#define REPR_SYM_GRAPH_EMPTY "|"

//  cbst__repr_graphic : affiche la représentation graphique valuée par rotation
//    antihoraire d'un quart de tour du sous-arbre binaire de recherche p avec
//    une indentation par niveau de REPR_TAB caractères et suivie de la hauteur
//    de ses sous-arbres non vide. Le niveau du sous-arbre est supposé être la
//    valeur de level. L'affichage des valeurs se fait avec la fonction put.
static void cbst__repr_graphic(const cbst *p,
    void (*put)(const void *ref), size_t level) {
  if (IS_EMPTY(p)) {
    printf("%*s", (int) (REPR__TAB * level), "");
    printf(REPR_SYM_GRAPH_EMPTY);
    printf("\n");
    return;
  }
  cbst__repr_graphic(RIGHT(p), put, level + 1);
  printf("%*s", (int) (REPR__TAB * level), "");
  put(REF(p));
  printf(" h=%d", HEIGHT(p));
  printf(" b=%d", cbst__balance(p));
  printf("\n");
  cbst__repr_graphic(LEFT(p), put, level + 1);
}

//=== Type bst =================================================================

//--- Définition bst -----------------------------------------------------------

struct bst {
  int (*compar)(const void *, const void *);
  cbst *root;
};

//--- Fonctions bst ------------------------------------------------------------

bst *bst_empty(int (*compar)(const void *, const void *)) {
  bst *t = malloc(sizeof *t);
  if (t == NULL) {
    return NULL;
  }
  t->compar = compar;
  t->root = EMPTY();
  return t;
}

void bst_dispose(bst **tptr) {
  if (*tptr == NULL) {
    return;
  }
  cbst__dispose((*tptr)->root);
  free(*tptr);
  *tptr = NULL;
}

void *bst_add_endofpath(bst *t, const void *ref) {
  if (ref == NULL) {
    return NULL;
  }
  return cbst__add_endofpath(&t->root, ref, t->compar);
}

extern void *bst_remove_climbup_left(bst *t, const void *ref) {
  if (ref == NULL) {
    return NULL;
  }
  return cbst__remove_climbup_left(&t->root, ref, t->compar);
}

void *bst_search(bst *t, const void *ref) {
  if (ref == NULL) {
    return NULL;
  }
  return cbst__search(t->root, ref, t->compar);
}

//  DEFUN_BST__MEASURE : définit la fonction récursive de nom « bst__ ## fun »
//     de paramètre un pointeur d'arbre binaire de recherche et de type size_t.
//     Applique la fonction cbst__ ## fun à la racine pointé par t
#define DEFUN_BST__MEASURE(fun)                                                \
  size_t bst_ ## fun(bst * t) {                                                \
    return cbst__ ## fun(t->root);                                             \
  }

DEFUN_BST__MEASURE(size)

size_t bst_height(bst *t) {
  return (size_t) cbst__height(t->root);
}

size_t bst_distance(bst *t) {
  return (size_t) cbst__distance(t->root);
}

size_t bst_number(bst *t, const void *ref) {
  if (ref == NULL) {
    return (size_t) (-1);
  }
  return cbst__number(t->root, ref, t->compar, 1);
}

size_t bst_rank(bst *t, const void *ref) {
  if (ref == NULL) {
    return (size_t) (-1);
  }
  return cbst__rank(t->root, ref, t->compar, 0);
}

void bst_repr_graphic(bst *t, void (*put)(const void *ref)) {
  cbst__repr_graphic(t->root, put, 0);
}
