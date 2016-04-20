

#ifndef UNSAFEARRAY_H
#define UNSAFEARRAY_H 
#include <stdint.h> 


typedef struct Array *Array;


static inline void Array_free (Array *a);

static inline Array Array_copy (Array a, int length);

static inline uint32_t* Array_at (Array a, int i);
static inline Array Array_new (int length);

static inline int Array_length (Array a);

#endif 