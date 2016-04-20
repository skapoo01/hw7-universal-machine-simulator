#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <inttypes.h>
#include <string.h>
typedef struct Array 
{
	int length;
	uint32_t elems[];
} *Array;

static inline int Array_length (Array a);


static inline void Array_free (Array *a);

static inline Array Array_copy (Array a, int length);

static inline uint32_t* Array_at (Array a, int i);
static inline Array Array_new (int length);

// int main()
// {
// 	Array test = Array_new(5);
// 	uint32_t *a = Array_at(test, 0) , *b = Array_at(test, 1), *c = Array_at(test, 2), *d = Array_at(test, 3), *e=Array_at(test, 4);
// 	*a=0, *b =1, *c = 2, *d=3, *e=4; 
// 	uint32_t* var;
// 	for (int i = 0; i < 5; i++){
// 		var = Array_at(test, i);
// 		printf("value at index[%d] is : %"PRIu32"\n", i, *var);
// 	}

// 	Array copy = Array_copy(test, 5);

// 	for (int i = 0; i < 5; i++){
// 		var = Array_at(copy, i);
// 		printf("value at copy array index[%d] is : %"PRIu32"\n", i, *var);
// 	}

// 	Array_free(&test);
// 	Array_free(&copy);
// 	(void)test;
// 	return 0;
// }



static inline int Array_length (Array a)
{
	return a -> length;
}

static inline uint32_t* Array_at (Array a, int i)
{
	assert (a);
	assert (i >= 0 && i < a -> length);
	return &(a->elems[i]);

} 

static inline void Array_free (Array *a)
{
	assert (a && *a);
	free(*a);
} 

static inline Array Array_copy (Array a, int length)
{
	Array copy;
	assert (a);
	assert (length >= 0);
	copy = Array_new(length);
	if( copy->length >= a->length && a->length > 0){
		memcpy(copy->elems, a->elems, a->length*sizeof(*a->elems));
	} else if (a->length > copy -> length && copy->length > 0){
		memcpy(copy->elems, a->elems, copy->length*sizeof(*a->elems));
	}
	return copy;
} 


static inline Array Array_new (int length)
{
	Array a = malloc(sizeof(*a) + length * sizeof(*a->elems));
	assert (a != NULL && length > 0);
	a -> length = length;
	return a;
}