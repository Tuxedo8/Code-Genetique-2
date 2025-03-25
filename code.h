#ifndef CODE_H
#define CODE_H

#include <stdint.h>
/* A COMPLETER */
/* Définir ici le type code_t */
typedef struct {
  uint8_t *sequence;
  int taille;
  int alloc_size;
} code_t;

code_t *alloue_code();
void libere_code(code_t *c);

int taille_code(code_t *c);
char code_element(code_t *c, int position);
void code_ajoute(code_t *c, char element);

int sous_code(code_t *a, code_t *b);
void coupe_code(code_t *a, int position, code_t **b, code_t **c);
code_t *combine_codes(code_t *a, code_t *b);

#endif
