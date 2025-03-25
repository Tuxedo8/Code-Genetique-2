#include "parametrized_output.h"

static FILE *output = NULL;

void set_parametrized_output(char *name, int num_line, char *filename) {
  FILE *f = fopen(name, "w");
  if (f != NULL) {
    output = f;
  } else {
    alert("Cannot open %s for writing in %s, line %d\n", name, filename, num_line);
  }
}

FILE *get_parametrized_output() {
  if (output == NULL) {
    output = stdout;
  }
  return output;
}
