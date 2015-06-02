#ifndef _UTIL_H
#define _UTIL_H

#include <stddef.h>

typedef void** vec_t;

#ifdef __GNUC__
#define AUTO(f) __attribute__((cleanup(f)))
#else
#define AUTO(f)
#endif


#ifdef __GNUC__
#define NORETURN __attribute__((noreturn))
#else
#define NORETURN
#endif

#define AUTO_VEC AUTO(vec_ptr_free) vec_t
#define VEC_PUSH(v, e)                          \
  do {                                          \
    void* x = (void*) (size_t) e;                \
    vec_push(&v, x);                            \
  } while (0)

void mem_error() NORETURN;
void* malloc_s(size_t size);
void* realloc_s(void*, size_t);
vec_t vec_empty();
vec_t vec_of_size(size_t);
void vec_free(vec_t);
void vec_ptr_free(vec_t*);
vec_t vec_dup(vec_t);

size_t vec_size(vec_t);
size_t vec_capacity(vec_t);

void vec_push(vec_t* v, void* e);
void vec_pop(vec_t*);

#endif // _UTIL_H
