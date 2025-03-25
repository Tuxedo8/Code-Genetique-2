#ifndef PARAMETRIZED_OUTPUT_H
#define PARAMETRIZED_OUTPUT_H
#include <stdio.h>

#ifdef VERBOSE
#define alert(f,...) fprintf(stderr, "*** WARNING *** " f, ##__VA_ARGS__)
#else
#define alert(f,...)
#endif

#ifdef DEBUG
#define debug(f,...) fprintf(stderr, "*** DEBUG *** " f, ##__VA_ARGS__)
#else
#define debug(f,...)
#endif

void set_parametrized_output(char *, int, char *);
FILE *get_parametrized_output();
#define set_result_output(x) set_parametrized_output(x, __LINE__, __FILE__)
#define print_result(f,...) fprintf(get_parametrized_output(), f, ##__VA_ARGS__)
#define check(x) ((x)?print_result("Check passed\n"):\
                  (print_result("*** CHECK FAILED *** at line %d in %s\n", __LINE__, __FILE__), exit(1)))

#endif
