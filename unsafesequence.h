#ifndef UNSAFESEQUENCE_H
#define UNSAFESEQUENCE_H
#include <stdlib.h>
#include <stdio.h>
#include "unsafearray.h"
typedef struct Sequence *Sequence;

Sequence Seq_new (int hint);
void Seq_addhi (Sequence s, Array a);
void Seq_expand (Sequence s);
Array Seq_get (Sequence s, int index);
int Seq_length (Sequence s);
void Seq_put (Sequence s, int index, Array a);
void Seq_free(Sequence *s);


#endif 