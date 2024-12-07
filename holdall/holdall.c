//  Partie implantation du module holdall.

#include "holdall.h"

//  struct holdall, holdall : implantation par liste dynamique simplement
//    chainée.

//  Si la macroconstante HOLDALL_PUT_TAIL est définie et que sa macro-évaluation
//    donne un entier non nul, l'insertion dans la liste a lieu en queue. Dans
//    le cas contraire, elle a lieu en tête.

typedef struct choldall choldall;

struct choldall {
  void *ref;
  choldall *next;
};

struct holdall {
  choldall *head;
#if defined HOLDALL_PUT_TAIL && HOLDALL_PUT_TAIL != 0
  choldall **tailptr;
#endif
  size_t count;
};

holdall *holdall_empty(void) {
  holdall *ha = malloc(sizeof *ha);
  if (ha == NULL) {
    return NULL;
  }
  ha->head = NULL;
#if defined HOLDALL_PUT_TAIL && HOLDALL_PUT_TAIL != 0
  ha->tailptr = &ha->head;
#endif
  ha->count = 0;
  return ha;
}

void holdall_dispose(holdall **haptr) {
  if (*haptr == NULL) {
    return;
  }
  choldall *p = (*haptr)->head;
  while (p != NULL) {
    choldall *t = p;
    p = p->next;
    free(t);
  }
  free(*haptr);
  *haptr = NULL;
}

int holdall_put(holdall *ha, void *ref) {
  choldall *p = malloc(sizeof *p);
  if (p == NULL) {
    return -1;
  }
  p->ref = ref;
#if defined HOLDALL_PUT_TAIL && HOLDALL_PUT_TAIL != 0
  p->next = NULL;
  *ha->tailptr = p;
  ha->tailptr = &p->next;
#else
  p->next = ha->head;
  ha->head = p;
#endif
  ha->count += 1;
  return 0;
}

size_t holdall_count(holdall *ha) {
  return ha->count;
}

int holdall_apply(holdall *ha,
    int (*fun)(void *)) {
  for (const choldall *p = ha->head; p != NULL; p = p->next) {
    int r = fun(p->ref);
    if (r != 0) {
      return r;
    }
  }
  return 0;
}

int holdall_apply_context(holdall *ha,
    void *context, void *(*fun1)(void *context, void *ptr),
    int (*fun2)(void *ptr, void *resultfun1)) {
  for (const choldall *p = ha->head; p != NULL; p = p->next) {
    int r = fun2(p->ref, fun1(context, p->ref));
    if (r != 0) {
      return r;
    }
  }
  return 0;
}

int holdall_apply_context2(holdall *ha,
    void *context1, void *(*fun1)(void *context1, void *ptr),
    void *context2, int (*fun2)(void *context2, void *ptr, void *resultfun1)) {
  for (const choldall *p = ha->head; p != NULL; p = p->next) {
    int r = fun2(context2, p->ref, fun1(context1, p->ref));
    if (r != 0) {
      return r;
    }
  }
  return 0;
}

#if defined HOLDALL_WANT_EXT && HOLDALL_WANT_EXT != 0

#define HALF(k) ((k) >> 1)

//  partage : Coupe la liste pointée par ha en deux listes de longueur
//    équivalente (±1) et renvoie un pointeur vers la premiére cellule de la
//    deuxiéme liste.
static choldall *partage(holdall *ha) {
  size_t n = HALF(ha->count);
  choldall **pp = &ha->head;
  for (size_t k = 0; k < n; ++k) {
    pp = &(*pp)->next;
  }
  choldall *p = *pp;
  *pp = NULL;
  return p;
}

//  fuse : fusionne dans la liste pointée par *pp les deux listes pointées par
//    h1 et h2
static void fuse(choldall **pp, choldall *h1, choldall *h2,
    int (*compar)(const void *, const void *)) {
  choldall *p = NULL;
  choldall **pp2 = &p;
  while (h1 != NULL && h2 != NULL) {
    if (compar(h1->ref, h2->ref) <= 0) {
      choldall *p1 = h1->next;
      *pp2 = h1;
      pp2 = &h1->next;
      h1 = p1;
    } else {
      choldall *p2 = h2->next;
      *pp2 = h2;
      pp2 = &h2->next;
      h2 = p2;
    }
  }
  *pp = p;
  if (h1 != NULL) {
    *pp2 = h1;
  }
  if (h2 != NULL) {
    *pp2 = h2;
  }
}

void holdall_sort(holdall *ha,
    int (*compar)(const void *, const void *)) {
  if (ha->count <= 1) {
    return;
  }
  size_t n = ha->count;
  holdall ha2 = {
    partage(ha),
    .count = ha->count - HALF(ha->count)
  };
  ha->count = HALF(ha->count);
  holdall_sort(&ha2, compar);
  holdall_sort(ha, compar);
  choldall *p = NULL;
  fuse(&p, ha->head, ha2.head, compar);
  ha->head = p;
#if defined HOLDALL_PUT_TAIL && HOLDALL_PUT_TAIL != 0
  while (p->next != NULL) {
    p = p->next;
  }
  ha->tailptr = &p;
#endif
  ha->count = n;
}

#endif
