#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <inttypes.h>
#include <string.h>
#include "unsafearray.h"
#include "unsafesequence.h"

struct Sequence  
{
	int Length, capacity;
	Array *elems;
};


// int main()
// {
// 	Sequence s = Seq_new(10);

	
// 	for (int i = 1; i < 13; i++){
// 		Array a = Array_new(i);
// 		for (int j = 0; j <= i; j++){
// 			uint32_t *num = Array_at(a, j); 
// 			*num = j;
// 		}
// 		Seq_addhi(s, a);
// 	}

// 	Array temp = Array_new(3);
// 	uint32_t *val = Array_at(temp, 0); 
// 	*val = 404040;
// 	Array old = Seq_get(s, 9);
// 	Array_free(&old);
// 	Seq_put(s, 9, temp);

// 	printf("Sequence length is: %d \n", Seq_length(s));
// 	printf("Sequence capacity is: %d \n",s->capacity );

// 	for (int h = 0; h < 12; h++){
// 		Array test = Seq_get(s, h);
// 		for (int t = 0; t <= h; t++){
// 			printf("a %d [%d] = %"PRIu32" \n", h, t, *Array_at(test, t));
// 		}
// 		printf("\n");
// 	}

// 	Seq_free(&s);	
	
// 	return 0;
// }


Sequence Seq_new (int hint)
{
	assert (hint > 0); 
	Sequence s = malloc(sizeof(*s));
	s->capacity = hint;
	s->Length = 0;
	s->elems = malloc(hint * (sizeof(*(s->elems))));
	return s;
}

void Seq_expand (Sequence s)
{
	Array *new = malloc((s->capacity * 2) * (sizeof(*(s->elems))));
	
	for (int i = 0; i < s->Length; i++){
		new[i] = s->elems[i];
	}
	s->capacity = s->capacity * 2; 
	Array *temp = s->elems;
	free(temp);
	s->elems = new;
}

void Seq_addhi (Sequence s, Array a)
{
	if (s -> Length == s-> capacity){
		Seq_expand(s);
	}
	s->elems[s->Length] = a;
	s->Length++;
}

Array Seq_get (Sequence s, int index)
{
	assert (index < s->Length);
	return s->elems[index];
}

int Seq_length (Sequence s)
{
	return s->Length;
}

void Seq_put (Sequence s, int index, Array a)
{
	s->elems[index] = a;
}

void Seq_free(Sequence *s)
{
	for (int i = 0; i < (*s)->Length; i++){
		Array temp = (*s)->elems[i];
		Array_free(&temp);
	}
	free((*s)->elems);
	free(*s);

}