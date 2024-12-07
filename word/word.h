//  word : Un module permettant de gérer un mot de longueur indéfinie, mémorisé
//    dans une structure nommée « word ».

#ifndef WORD__H
#define WORD__H

#include <stdbool.h>
#include <stdlib.h>

//  struct word, word: type et nom de type d'un contrôleur regroupant
//    les informations nécessaires pour gérer un mot de longueur indéfinie.
typedef struct word word;

//  word_init :  tente d'allouer les ressources nécessaires pour gérer un
//    nouveau mot initialement vide. Renvoie NULL en cas de dépassement de
//    capacité. Renvoie sinon un pointeur vers le contrôleur associé au mot.
extern word *word_init();

//  word_add : tente d'insérer un caractére c au mot associé à w. Renvoie NULL
//    en cas de dépassement de capacité. Renvoie sinon le nouveau mot obtenue
//    aprés l'insertion.
extern void *word_add(word *w, int c);

//  word_renit : met à zero la longueur associée au mot w et le transforme en
//    mot vide.
extern void word_renit(word *w);

//  word_is_empty : renvoie true ou false selon que le mot associé à w est
//    vide ou non.
extern bool word_is_empty(word *w);

//  word_get : Renvoie la chaine de caractéres associée au mot w
extern char *word_get(word *w);

//  word_length : renvoie la longueur du mot associée à w.
extern size_t word_length(word *w);

//  stack_dispose : sans effet si *wptr vaut NULL. Libère sinon les ressources
//    allouées à la gestion du mot associé à *wptr puis affecte NULL à
//    *wptr.
extern void word_dispose(word **wptr);

#endif
