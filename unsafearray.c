#include <stdlib.h>
#include <stdio.h>


typedef struct Array 
{
	int length, size;
	uint32_t elems[];
} *Array;

static inline int Array_length (Array a)
{
	return a -> length;
}

static inline void* Array_at (Array a, int i)
{
	assert (a);
	assert (i >= 0 && i < a -> length);
	return a->elems + i*a->size;

} 
static inline void Array_free (Array *a)
{
	assert (a && *a);
	FREE((*a)->elems);
	FREE(*a);
} 

static inline Array Array_copy (Array a, int length){
	Array copy;
	assert (a);
	assert (length >= 0);
	copy = Array_new(length, a->length);
	if( copy->length >= a->length && a->length > 0){
		memcpy(copy->elems, a->elems, a->length*a->size);
	} else if (a->length > copy -> length && copy->length > 0){
		memcpy(copy->elems, a->elems, copy->length*a->size);
	}
	return copy;
} 

static inline Array Array_new (int length, int size)
{
	Array a = malloc(sizeof(*a) + length * sizeof(*a->elems));
	assert (a != NULL);
	a -> elems = calloc (length, size);
	assert (a -> elems != NULL && length > 0);
	a -> length = length;
	a -> size = size;
	return a;
}