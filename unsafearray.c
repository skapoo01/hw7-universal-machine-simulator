#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <inttypes.h>
#include <string.h>
#include "unsafearray.h"

struct Array 
{
	int length;
	uint32_t elems[];
};


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


int Array_length (Array a)
{
	return a -> length;
}
uint32_t* Array_at (Array a, int i)
{
	return &(a->elems[i]);
} 
void Array_free (Array *a)
{

	free(*a);
} 
Array Array_copy (Array a, int length)
{
	Array copy;

	copy = Array_new(length);
	if( copy->length >= a->length && a->length > 0){
		memcpy(copy->elems, a->elems, a->length*sizeof(*a->elems));
	} else if (a->length > copy -> length && copy->length > 0){
		memcpy(copy->elems, a->elems, copy->length*sizeof(*a->elems));
	}
	return copy;
} 

Array Array_new (int length)
{
	assert (length > 0);
	Array a = malloc((sizeof(*a) + length) * (sizeof(*a->elems)));
	assert(a != NULL);
	a -> length = length;
	for (int i = 0; i < length; i++){
		a->elems[i] = 0;
	}
	return a;
}