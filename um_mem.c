/**********************************************************************
 *
 *              um_mem.c 
 *          This file contains the implementations for all functions outlined
 *          in um_mem.h, Module handles the segmented memory of 
 *          the Universal Machine
 *
 *          Written by: Ballard Blair and Siddharth Kapoor 
 *                  on: 4-14-16 
 *      
 *
 ********************************************************************/ 

#include <stack.h>
#include <seq.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <uarray.h> 
#include "um_mem.h"
#include <assert.h>
#include <inttypes.h>
#include <sys/stat.h>
#include <bitpack.h>

#define estimate 10
#define byte 8


struct UM_Mem {
    Seq_T memory;       /* A sequence of pointers to UArray_T segments */
    Stack_T mem_tracker;/* A stack of integer seg_id’s */
};

/* returns a new UM_Mem with an empty memory sequence capable of holding 
   UArray segments, and an empty mem_tracker stack */
UM_Mem new_memory()
{
    UM_Mem mem = malloc (sizeof(*mem)); 

    mem -> memory = Seq_new(estimate);
    mem -> mem_tracker = Stack_new (); 

    return mem; 
}

/* frees all associated memory in the UM_mem */
extern void free_memory(UM_Mem m)
{
    UArray_T temp = NULL;
    int i = 0;

    int length = Seq_length(m -> memory);
    /* free every element of the sequence until it is empty */
    for (i = 0; i < length; i++){
            temp = Seq_get (m -> memory, i);
            UArray_free (&temp);
    }

    Seq_free (&(m -> memory));
    Stack_free (&(m -> mem_tracker));
    free(m);
}

/* creates a segment in UM_mem capable of holding num_words, 32-bit words and 
   returns seg_id */
int map_seg(UM_Mem m, int num_words)
{
    uint64_t index;
    /* creates new UArray to hold num_words and size of a uint32_t */
    UArray_T segment = UArray_new(num_words, sizeof(uint32_t));

    /* checks if stack of unmapped segments is empty */     
    if (Stack_empty (m -> mem_tracker) == 1){
            /* add segment to sequence */
            Seq_addhi (m -> memory, segment);
            return (Seq_length (m -> memory) - 1);
     } else {
            index = (uint64_t)Stack_pop (m -> mem_tracker);
            UArray_T old = Seq_get(m -> memory, (int)index);
            UArray_free(&old);
            Seq_put (m -> memory, (int)index, segment);
            return (int)index;
    }
} 

/* pushes segment id onto mem_tracker to be reused */
void unmap_seg(UM_Mem m, int seg_id)
{
    assert (seg_id < Seq_length(m -> memory));

    UArray_T segment = Seq_get (m -> memory, seg_id);
    uint64_t seg_index = seg_id; 

    if (segment != NULL){   
            Stack_push (m -> mem_tracker, (void *)seg_index);
    }
} 

/* returns address of a particular offset in a particular segment in memory */
void* mem_address(UM_Mem m, int seg_id, int offset)
{       
    UArray_T segment = Seq_get (m -> memory, seg_id);
    return UArray_at(segment, offset);
} 

/* loads 32-bit word into segment 0 */
void load_instruction(UM_Mem m, const char* filename) 
{ 
    FILE *fp = fopen (filename, "r");
    assert (fp != NULL);

    uint32_t codeword = 0;
    uint32_t *pointer;
    int seg_index = 0;
    int counter = 0;
        
    struct stat file_info; 
    assert (stat(filename, &file_info) == 0);
    int num_instructions = file_info.st_size / 4; 

    UArray_T segment_0 = UArray_new(num_instructions, sizeof(uint32_t));
    int c = 0;
        
    while (counter < num_instructions) {
        for (int i = 3; i >= 0; i--) {
            c = getc(fp);
            if (c == EOF) { 
                break;
            }
            codeword = Bitpack_newu(codeword, byte, byte * i, (uint64_t) c);
        }
        pointer = UArray_at (segment_0, seg_index);
        *pointer = codeword;
        counter++;
        seg_index++;
    }
        
    Seq_addlo (m -> memory, segment_0);
    fclose(fp);
}

/* mem segment at the seg_id is duplicated, and duplicate replaces segment 0*/
void load_segment(UM_Mem m, int seg_id) 
{
    UArray_T to_copy = Seq_get(m -> memory, seg_id);
    UArray_T segment = UArray_copy(to_copy, UArray_length(to_copy));

    UArray_T seg_0 = Seq_get(m -> memory, 0);       
    UArray_free(&seg_0);

    Seq_put(m->memory, 0, segment);
}

/* returns the length of the segment associated with seg_id */
int segment_length(UM_Mem m, int seg_id)
{
    UArray_T segment = Seq_get (m -> memory, seg_id);
    int length = UArray_length(segment);
    return length;
} 

/* prints out the sequence memory, and corresponding segments */
void print_mem_map(UM_Mem m)
{
    UArray_T segment;
    uint32_t *word;
    for (int i= 0; i < Seq_length(m -> memory); i++) {
            segment = Seq_get(m -> memory, i);
            printf("Segment_%d[%d] : | ", i, segment_length(m, i));
                
            for (int j = 0; j < UArray_length(segment); j++) {
                    word = UArray_at(segment, j);
                    printf("{%d} %"PRIu32" |", j, *word);
            }        
            printf("\n");
    }
}