#ifdef malloc
#undef malloc
#endif

#ifdef calloc
#undef calloc
#endif

#ifdef realloc
#undef realloc
#endif

#ifdef strdup
#undef strdup
#endif

#ifdef free
#undef free
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "parametrized_output.h"

#ifndef MEM_FAIL_AFTER
#define MEM_FAIL_AFTER -1
#endif

static int mallocs_counter = 0;
static int callocs_counter = 0;
static int reallocs_counter = 0;
static int strdup_counter = 0;
static int free_counter = 0;
static int allocs_counter = 0;
static int max = 0;
static int current = 0;
static int overflow = 0;
static int failure_countdown = MEM_FAIL_AFTER;
static size_t memory = 0;
static size_t max_memory = 0;

#ifdef TRACK_ALLOCS
#define MAX_ALLOCS_COUNT 1000
static void *allocs[MAX_ALLOCS_COUNT];
static size_t sizes[MAX_ALLOCS_COUNT];
#endif

int find_alloc(void *ptr) {
#ifdef TRACK_ALLOCS
  int pos=0;
  while ((pos < current) && (allocs[pos] != ptr)) {
    pos++;
  }
  if (pos < current) {
    return pos;
  } else
    if (overflow)
      return -2;
    else
      return -1;
#else
  return 0;
#endif
}

void remove_alloc(int pos) {
#ifdef TRACK_ALLOCS
  if (pos >= 0) {
    memory -= sizes[pos];
    debug("Alloc %p (%lu bytes) removed from %d (replaced by %d), total size is %lu\n", allocs[pos], sizes[pos], pos, current-1, memory);
    sizes[pos] = sizes[current-1];
    allocs[pos] = allocs[current-1];
  } else {
    overflow--;
    current++;
  }
#endif
  free_counter++;
  current--;
}

int add_alloc(void *ptr, size_t size) {
#ifdef TRACK_ALLOCS
  if (current < MAX_ALLOCS_COUNT) {
    allocs[current] = ptr;
    sizes[current] = size;
    memory += size;
    debug("Alloc %p (%lu bytes) added at %d, total size is %lu\n", ptr, size, current, memory);
    if (memory > max_memory)
      max_memory = memory;
  }
#endif
    allocs_counter++;
    current++;
    if (current+overflow > max)
      max = current+overflow;
#ifdef TRACK_ALLOCS
  if (current > MAX_ALLOCS_COUNT) {
    alert("Maximum number of allocations reached, %d\n", MAX_ALLOCS_COUNT);
    overflow++;
    current--;
    return 0;
  }
#endif
  return 1;
}

static int soft_fail() {
  if (failure_countdown == 0)
    return 1;
  if (failure_countdown > 0)
    failure_countdown--;
  return 0;
}

void trigger_soft_fail(int countdown) {
  failure_countdown = countdown;
}

void *my_malloc(size_t size) {
  if (soft_fail()) {
    debug("Malloc artificial failure for size %lu\n", size);
    return NULL;
  }
  mallocs_counter++;
  void *result = malloc(size);
  if (result != NULL) {
    add_alloc(result, size);
  }
  return result;
}

void *my_calloc(size_t count, size_t size) {
  if (soft_fail()) {
    debug("Calloc artificial failure for count %lu and size %lu\n", count, size);
    return NULL;
  }
  callocs_counter++;
  void *result = calloc(count, size);
  if (result != NULL) {
    add_alloc(result, count*size);
  }
  return result;
}

void *my_realloc(void *p, size_t size) {
  if (soft_fail()) {
    debug("Realloc artificial failure on %p\n", p);
    return NULL;
  }
  reallocs_counter++;
  if (p == NULL) {
    void *result = realloc(p, size);
    if (result != NULL) {
      add_alloc(result, size);
    }
    return result;
  } else {
    int pos = find_alloc(p);
    if (pos != -1) {
      void *result = realloc(p, size);
      if (result != NULL) {
        remove_alloc(pos);
        add_alloc(result, size);
      }
      return result;
    } else {
      alert("Realloc called on an unknown pointer, %p\n", p);
      return NULL;
    }
  }
}

char *my_strdup(char *s) {
  if (soft_fail()) {
    debug("Strdup artificial failure for size %lu\n", strlen(s));
    return NULL;
  }
  strdup_counter++;
  char *result = strdup(s);
  if (result != NULL) {
    add_alloc(result, strlen(s)+1);
  }
  return result;
}

void my_free(void *ptr) {
  if (ptr != NULL) {
    int pos = find_alloc(ptr);
    if (pos != -1) {
      remove_alloc(pos);
      free(ptr);
    } else {
      alert("Free called on an unknown pointer, %p\n", ptr);
    }
  }
}

int current_allocs() {
  return current + overflow;
}

int total_allocs() {
  return allocs_counter;
}

size_t memory_used() {
  return memory;
}

size_t max_memory_used() {
  return max_memory;
}

int mallocs_done() {
  return mallocs_counter;
}

int callocs_done() {
  return callocs_counter;
}

int reallocs_done() {
  return reallocs_counter;
}
