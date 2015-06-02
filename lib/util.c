#include "util.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

void mem_error()
{
  fprintf(stderr, "Not enough memory, aborting!\n");
  abort();
}

void* malloc_s(size_t size)
{
  void* res = malloc(size);
  if (!res) {
    mem_error();
  }
  return res;
}

void* realloc_s(void* ptr, size_t size)
{
  ptr = realloc(ptr, size);
  if (!ptr) {
    mem_error();
  }
  return ptr;
}

static const size_t DEFAULT_CAPACITY = 4;

vec_t vec_of_size(size_t size)
{
  size_t capacity = size > 0 ? size : DEFAULT_CAPACITY;
  size_t* res = malloc_s(capacity * sizeof(void*)
                         + 2 * sizeof(size_t));
  res[0] = capacity;
  res[1] = size;
  vec_t v = (vec_t) (res + 2);
  memset(v, 0, size * sizeof(void*));
  return v;
}

vec_t vec_empty()
{
  return vec_of_size(0);
}

vec_t vec_dup(vec_t v)
{
  size_t size = vec_size(v);
  vec_t res = vec_of_size(size);
  memcpy(res, v, sizeof(void*) * size);
  return res;
}

size_t vec_size(vec_t v)
{
  return ((size_t*) v)[-1];
}

size_t vec_capacity(vec_t v)
{
  return ((size_t*) v)[-2];
}

static void set_capacity(vec_t v, size_t capacity)
{
  ((size_t*) v)[-2] = capacity;
}

static void set_size(vec_t v, size_t size)
{
  ((size_t*) v)[-1] = size;
}

static void vec_ensure_capacity(vec_t* v, size_t new_size)
{
  size_t cap = vec_capacity(*v);
  if (cap < new_size + 1) {
    cap <<= 1;
    *v = realloc_s(((size_t*) *v - 2), cap * sizeof(void*) + 2 * sizeof(size_t));
    *v = (void*) ((size_t*) *v + 2);
    set_capacity(*v, cap);
  }
}

void vec_push(vec_t* v, void* e)
{
  size_t size = vec_size(*v);
  vec_ensure_capacity(v, size + 1);
  (*v)[size] = e;
  set_size(*v, size + 1);
}

void vec_pop(vec_t* v)
{
  size_t size = vec_size(*v);
  assert(size > 0);
  v[size - 1] = NULL;
  set_size(*v, size - 1);
}

void vec_free(vec_t v)
{
  free((size_t*) v - 2);
}

void vec_ptr_free(vec_t* v)
{
  vec_free(*v);
}
