

#ifndef UNSAFEARRAY_H
#define UNSAFEARRAY_H 
#include <stdint.h> 


typedef struct Array *Array;


void Array_free (Array *a);

Array Array_copy (Array a, int length);

uint32_t* Array_at (Array a, int i);
Array Array_new (int length);

int Array_length (Array a);

#endif 