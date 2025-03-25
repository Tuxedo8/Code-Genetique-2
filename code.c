#include "code.h"
#include <stdlib.h>
#include <stdio.h>

/*
void print_binary(uint8_t byte) {
    for (int i = 7; i >= 0; i--) {
        putchar((byte & (1 << i)) ? '1' : '0');
    }
    printf("\n");
}
*/

code_t *alloue_code() {

  //printf("\nDebut Alloue_code\n");
  
  code_t *v = (code_t *)malloc(sizeof(code_t));
  if(v == NULL){
      return NULL; // Echec d'allocation
  }
  v->sequence = (uint8_t *)malloc(sizeof(uint8_t));
  if(v->sequence == NULL){
    free(v);
    return NULL;
  }
  v->sequence[0]=0;
  v->alloc_size=1;
  v->taille=0;

  //printf("\nFin Alloue_code\n");

  return v;

}
void libere_code(code_t *c) {
  //printf("\nDebut Liberer_code\n");
  if(c == NULL){return;}
  free(c->sequence);
  free(c);

  //printf("\nFin Liberer_code\n");
}

int taille_code(code_t *c) {
  //printf("\nDebut Taille_code == 1er element : %d\n", c->sequence[0]);
  if(c == NULL){return 0;}
  
  //printf("\nFin Taille_code == %d\n", c->taille);
  
  return c->taille;

}

char translation_b2c(uint8_t bin){ // Traduit un octet en char, uniquement pour les codons
  
 // printf("\nDebut translation_b2c == %d\n", bin);
  //print_binary(bin);
  char cara = ' ';
  switch (bin) {
    case 0: // 00
      cara = 'A';
      break;
    case 1: // 01
      cara = 'C';
      break;
    case 2: // 10
      cara = 'G';
      break;
    case 3: // 11
      cara = 'T';
      break;
  }

  //printf("\nFin translation_b2c == %c\n", cara);
  return cara;
}


char code_element(code_t *c, int p) {  
    // Si la position donnée est égale à la fin du code, la valeur à renvoyer est ' '
  if(c == NULL || taille_code(c)==0 || p >= taille_code(c)){return ' ';}
  if(p<0){
    p=0;
    // La position 0 si position est négatif

  }
  int pos = (int)p/4;
  
  uint8_t s = 0;

  int decalage = (p%4)*2;
  s = (c->sequence[pos] >> decalage) & 0x03;
  // Sinon, la position qui précède le caractère de numéro position si position appartient à [0;taille_code(c)-1]
  return (translation_b2c(s));
}


void code_ajoute(code_t *c, char e) {

  //printf("\nDebut code_ajoute == add %c\n", e);
  
  if(c == NULL){return;}
  
  if(c->taille == c->alloc_size * 4){ // Si la taille de la séquence est égale à la taille alloué x 4, on realloc avec 1 octet de plus
    //printf("\nRealocation taille = %d , alloc_size = %d\n", c->taille, c->alloc_size);
    c->sequence = (uint8_t *)realloc(c->sequence, (c->alloc_size+1)*sizeof(uint8_t));
    
    if(c->sequence == NULL){
      return;
    }
    c->alloc_size ++;
    c->sequence[c->alloc_size-1]=0; // Initialisation a 0 de la nouvelle allocation
  }
  //c->sequence[c->taille/4]=0;

  int t = c->taille/4;

  switch(e){
    case 'A': // 00
      
      c->sequence[t] |= ((0 << ((c->taille * 2)%8)) | (0<<(((c->taille * 2)%8)+1)));
      c->taille ++; 
      break;
    
    case 'C': // 01
      c->sequence[t] |= 1<<(c->taille * 2)%8 | 0<<(((c->taille * 2)%8)+1);
      c->taille++;
      break;
    
    case 'G': // 10
      c->sequence[t] |= ((0 << ((c->taille * 2)%8)) | (1 << (((c->taille * 2)%8)+1)));
      c->taille++;
      break;
    
    case 'T': // 11
      c->sequence[t] |= ((1 << ((c->taille * 2)%8)) | (1 << (((c->taille * 2)%8)+1)));
      c->taille++;
      break;

    default: // Aucun de ces caractères
      return;
  }
  //printf("e == %c\n", e);

  //printf("\nFin code_ajoute == add dans [%d] = %d\n",t, c->sequence[t]);
  
}


int sous_code(code_t *a, code_t *b) {

  //printf("\nDebut sous_code\n");
  
  if(a == NULL || b == NULL){return -1;}

  int position = -1;  
  int k = 0;
    
  for (int i = 0; i < taille_code(a); i++) {
      if (code_element(a, i) == code_element(b, k)) {
          if (k == 0) {
              position = i;  
          }
          k++;
          if (k == taille_code(b)) {
              
    //          printf("\nFin sous_code\n");
              return position;  
          }
      } else {
          
          if (k > 0) {
              i = position;  
          }
          k = 0;
          position = -1;
      }
  }

  //printf("\nFin sous_code\n");
  return -1;
}
void coupe_code(code_t *a, int position, code_t **b, code_t **c){

  //printf("\nDebut coupe_code\n");
  
  if (a == NULL || b == NULL || c == NULL) {
    return;
  }

  *c = alloue_code();
  if (*c == NULL) {
        return;
  }

  if (position > taille_code(a)) {
        position = taille_code(a);
  }
  if(position < 0){
      position = 0;
  }

  *b = a; // On place le pointeur a, dans le pointeur *b
  
  for(int i = position; i<taille_code(a); i++){
    if(code_element(a, i) != ' '){
      code_ajoute(*c, code_element(a, i)); // On ajoute dans c le contenue de a à partir de la position demandé
    }else{return;}
  }

  // On réduit la taille de (*b) (anciennement a)
  (*b)->taille = position;

  int new_alloc = ((*b)->taille/4)+1; // +1 sinon on aura un indice et potentiellement 0
  
  (*b)->sequence = (uint8_t *)realloc((*b)->sequence, (new_alloc * sizeof(uint8_t))); // reallocation d'une nouvelle zone mémoire plus compacte
  
  (*b)->alloc_size = new_alloc; // Comme taille = alloc_size * 4; alors alloc_size = taille / 4
  
  //printf("\nFin coupe_code\n");
}
code_t *combine_codes(code_t *a, code_t *b) {

  //printf("\nDebut combine_code\n");
  if(a == NULL || b == NULL){return NULL;}

  for(int i=0; i<taille_code(b); i++){
    code_ajoute(a, code_element(b, i));
  }

  //printf("\nFin combine_code\n");
  return a;
}
