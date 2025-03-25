#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <getopt.h>
#include <limits.h>
#include "parametrized_output.h"
#include "my_memory.h"
#include "code.h"

#define NUMBER 10
#define ALLOC_ROUNDS 10
#ifdef DYNAMIC_ALLOCATIONS
int has_duplicates = 0;
#else
int has_duplicates = 1;
#endif
#ifdef IN_PLACE
int fixed_size = 0;
#else
int fixed_size = 1;
#endif
#ifndef CODES_PER_BYTE
#define CODES_PER_BYTE 1
#endif


void usage(char *nom) {
  fprintf(stderr,
         "Usage :\n"
         "%s [ -h ] [ -f position ] [ -c content ] [ -l length ] [ -n num_test ] [ -p position ] [ -s graine ] [ -w word ]\n"
         "\n"
         "Tests the proper behavior of 'codes', handles the following options (all of them but 'in-place' with an argument):\n"
         "-c, --content            Genetic code content (default random)\n"
         "-f, --fail               programs a memory failure at various points in the tests\n"
         "-l, --length             Length of generated genetic codes\n"
         "-n, --num-test           Test number:\n"
         "                         0. Allocations and deallocations (default)\n"
         "                         1. Additions\n"
         "                         2. Subcode\n"
         "                         3. Cut\n"
         "                         4. Combine content with word\n"
         "                         5. Combine word with content\n"
         "                         6. Memory usage check\n"
         "                         7. Memory allocation scheme check\n"
         "-p, --position           Position of a cut / length of the word (default random)\n"
         "-s, --seed               Seed for the random generation\n"
         "-w, --word               Word for the subcode search\n"
         , nom);
  exit(0);
}

static char codes[] = {'A', 'C', 'G', 'T'};

char *random_string(int length) {
  char *content;
  content = malloc(sizeof(char)*(length+1));
  if (content) {
    for (int i=0; i<length; i++) {
      content[i] = codes[random()%4];
    }
    content[length] = '\0';
  }
  return content;
}

code_t *create_code(char *content, int length) {
  code_t *c = NULL;
  char *to_be_freed = NULL;

  if (content == NULL) {
    content = random_string(length);
    to_be_freed = content;
  }
  if (content != NULL) {
    c = alloue_code();
    if (c == NULL) {
      fprintf(stderr,"Allocation failed, allocs count %d\n", current_allocs());
    } else {
      for (int i=0; content[i]; i++) {
        code_ajoute(c, content[i]);
        content[i] = '0';
      }
    }
  }
  free(to_be_freed);
  return c;
}

void free_duplicate(code_t *c) {
  if (has_duplicates) {
    libere_code(c);
  }
}

void affiche_code(char *name, code_t *c) {
  print_result("Code %s : '", name);
  for (int i=0; i<taille_code(c); i++)
    print_result("%c", code_element(c, i));
  print_result("' (taille %d)\n", taille_code(c));
}

int bits_to_bytes(int n) {
  return (n/8)+((n%8) != 0);
}

void check_memory(int codons, int structs) {
  // Estimates for the allocations
  int min_struct_size = 0; // the bare minimum is reached when code_t is a bare C string with a terminating char
  int max_struct_size = 32; // even with 8 byte integer, this should be enough
  int min_bits_per_codon = 2; // we have to store the codons
  int max_bits_per_codon = 4; // typical efficient allocation scheme

  int min = bits_to_bytes(codons * min_bits_per_codon) + structs * min_struct_size;
  int max = bits_to_bytes(codons * max_bits_per_codon) + structs * max_struct_size;
  debug("Memory allocated (%lu bytes)\n", memory_used());
  print_result("Checking that memory allocated is within %4d and %4d... ", min, max);
  check((memory_used() >= min) && (memory_used() <= max));
}

int adjust_expected(int *expected, int size) {
  int new_allocs=0;
  while (*expected < size) {
    new_allocs++;
    *expected *= 2;
  }
  return new_allocs;
}

int main(int argc, char *argv[]) {
  int num_test, seed, length, position, memory_fail, memory_fail_position;
  int opt;
  char *content, *word;

  content = NULL;
  word = NULL;
  length = 42;
  num_test = 0;
  position = INT_MAX;
  seed = 42;
  memory_fail = 0;
  memory_fail_position = 0;

  struct option longopts[] = {
    { "content", required_argument, NULL, 'c' },
    { "fail", required_argument, NULL, 'f' },
    { "help", no_argument, NULL, 'h' },
    { "length", required_argument, NULL, 'l' },
    { "num-test", required_argument, NULL, 'n' },
    { "position", required_argument, NULL, 'p' },
    { "seed", required_argument, NULL, 's' },
    { "word", required_argument, NULL, 'w' },
    { NULL, 0, NULL, 0 }
  };

  while ((opt = getopt_long(argc, argv, "c:l:n:p:s:w:f:h", longopts, NULL)) != -1) {
    switch(opt) {
    case 'c':
      content = optarg;
      break;
    case 'l':
      length = atoi(optarg);
      break;
    case 'n':
      num_test = atoi(optarg);
      break;
    case 'p':
      position = atoi(optarg);
      break;
    case 's':
      seed = atoi(optarg);
      break;
    case 'h':
      usage(argv[0]);
      break;
    case 'f':
      memory_fail = 1;
      memory_fail_position = atoi(optarg);
      break;
    case 'w':
      word = optarg;
      break;
    default:
      fprintf(stderr, "Unrecognized option %c\n", opt);
      exit(1);
    }
  }
  if (optind < argc) {
    set_result_output(argv[optind]);
  }
  print_result("Tests invoked with num-test %d, seed %d (code functions %s)\n",
         num_test, seed, has_duplicates?"duplicate codes":"are in place");
  if (content)
    print_result("Content is '%s'\n", content);
  else
    print_result("Content is random with length %d\n", length);
  srandom(seed);
  if (num_test == 0) {
    code_t *s[NUMBER];

    print_result("Allocating %d codes\n", NUMBER);
    for (int i=0; i<NUMBER; i++) {
      s[i] = create_code(content, length);
    }
    print_result("Freeing half of them\n");
    for (int i=1; i<NUMBER; i+=2) {
      if (s[i] != NULL)
        libere_code(s[i]);
    }
    print_result("Checking that the number of allocations is at least %d\n", NUMBER/2);
    check(current_allocs() >= (NUMBER/2));
    print_result("Allocating %d codes\n", NUMBER/2);
    for (int i=1; i<NUMBER; i+=2) {
      s[i] = create_code(content, length);
    }
    print_result("Checking that the number of allocations is at least %d\n", NUMBER);
    check(current_allocs() >= NUMBER);
    print_result("Freeing all the codes\n");
    for (int i=0; i<NUMBER; i++) {
      if (s[i] != NULL)
        libere_code(s[i]);
    }
    print_result("Checking that the number of allocations is 0\n");
    check(current_allocs() == 0);
  } else {
    code_t *a, *b, *c = NULL;

    if (memory_fail && memory_fail_position == 0) {
      trigger_soft_fail(content == NULL);
    }

    a = create_code(content, length);
    affiche_code("a", a);

    if (position == INT_MAX)
      position = taille_code(a)?random()%taille_code(a):0;
    print_result("Position %d\n", position);

    if (num_test != 3) {
      if (memory_fail && memory_fail_position == 1) {
        trigger_soft_fail(word == NULL);
      }

      b = create_code(word, position);
      affiche_code("b", b);
    }

    if (memory_fail && memory_fail_position == 2) {
      trigger_soft_fail(0);
    }

    if (num_test == 2) {
      print_result("Searching b in a, result : %d\n", sous_code(a, b));
    }

    if (num_test == 3) {
      print_result("Spliting a code in two :\n");
      coupe_code(a, position, &c, &b);
      affiche_code("a", a);
      affiche_code("first part", c);
      affiche_code("second part", b);
    }

    if (num_test == 4) {
      print_result("Combining a and b :\n");
      c = combine_codes(a, b);
      affiche_code("a", a);
      affiche_code("b", b);
      affiche_code("c", c);
    }

    if (num_test == 5) {
      print_result("Combining b and a :\n");
      c = combine_codes(b, a);
      affiche_code("b", b);
      affiche_code("a", a);
      affiche_code("c", c);
      // exchange a and b for freeing the memory in the same way
      // as in the other tests
      code_t *tmp = a;
      a = b;
      b = tmp;
    }

    if (num_test < 6) {
      if (num_test > 2) {
        free_duplicate(c);
      }
    }

    if (num_test >= 6) {
      int total_char = taille_code(a) + taille_code(b);
      int allocs = 6; // 2 allocs per code + potential mallocs within random_string
      // Under an amortized allocation scheme we can expect that codes start from 1 byte
      int expected_a_size = CODES_PER_BYTE;
      int expected_b_size = CODES_PER_BYTE;
      // The factor should take into account various growing factors
      int factor = 2;

      if (has_duplicates)
        factor = 1;
      for (int i=0; i<ALLOC_ROUNDS; i++) {
        c = combine_codes(a, b);
        total_char += taille_code(b);
        free_duplicate(a);
        a = c;
        if ((num_test == 6) && !fixed_size)
          check_memory(total_char, 2);
        if (i < NUMBER) {
          print_result("Round %d (merge), ", i);
          affiche_code("a", a);
          affiche_code("b", b);
        }
        if (has_duplicates) {
          allocs += 2;
        } else {
          allocs += adjust_expected(&expected_a_size, taille_code(a));
          allocs += adjust_expected(&expected_b_size, taille_code(b));
        }
        if (num_test == 7) {
          int bound = allocs * factor;
          debug("Current number of allocations performed: %d\n", total_allocs());
          print_result("Checking that the number of allocations is no more than %4d... ", bound);
          check(total_allocs() <= bound);
        }
      }
      for (int i=ALLOC_ROUNDS-1; (i >= 0) && (taille_code(a) > 0); i--) {
        total_char -= taille_code(b);
        libere_code(b);
        int position = random()%taille_code(a);
        coupe_code(a, position, &c, &b);
        free_duplicate(a);
        a = c;
        if ((num_test == 6) && !fixed_size)
          check_memory(total_char, 2);
        if (i < NUMBER) {
          print_result("Round %d (cut at %d), ", i, position);
          affiche_code("a", a);
          affiche_code("b", b);
        }
      }
    }

    libere_code(a);
    libere_code(b);
    print_result("Checking that the number of allocations is 0\n");
    check(current_allocs() == 0);
  }
}
