#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <inttypes.h>
#include <string.h>
#include "unsafearray.h"


struct Stack  
{
	int Length, capacity;
	uint64_t *elems;
};


int main()
{
	
	return 0;
}


Stack Stack_new ()
{
	Stack s = malloc(sizeof(*s));
	s->capacity = hint;
	s->Length = 0;
	s->elems = malloc(hint * (sizeof(*(s->elems))));
	return s;
}

void Stack_expand (Stack s)
{
	Array *new = malloc((s->capacity * 2) * (sizeof(*(s->elems))));
	
	for (int i = 0; i < s->Length; i++){
		new[i] = s->elems[i];
	}
	s->capacity = s->capacity * 2; 
	uint64_t *temp = s->elems;
	free(temp);
	s->elems = new;
}

void Stack_push (Stack s, Array a)
{
	if (s -> Length == s-> capacity){
		Stack_expand(s);
	}
	s->elems[s->Length] = a;
	s->Length++;
}

uint64_t Stack_pop (Stack s)
{
	s->Length--;
	return s->elems[s->Length];

}

bool Stack_empty(Stack s)
{
	if (s->length == 0){
		return true;
	} else {
		return false;
	}
}

void Stack_free(Stack *s)
{
	for (int i = 0; i < (*s)->Length; i++){
		Array temp = (*s)->elems[i];
		Array_free(&temp);
	}
	free((*s)->elems);
	free(*s);

}
