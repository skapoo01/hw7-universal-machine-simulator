/**********************************************************************
 *
 *              um_mem.h 
 *
 *          This header file outlines the functions implemented in um_mem.c. 
 *          This module handles the segmented memory of the Universal Machine.
 *
 *          Written by: Ballard Blair and Siddharth Kapoor 
 *                  on: 4-14-16 
 *      
 *
 ********************************************************************/
#ifndef UM_MEM_INCLUDED
#define UM_MEM_INCLUDED
#include <stack.h>
#include <seq.h>
#include <stdio.h>
#include <stdlib.h> 

typedef struct UM_Mem *UM_Mem;

/* returns a new UM_Mem with an empty memory sequence capable of holding 
   UArray segments, and an empty mem_tracker stack */
extern UM_Mem new_memory();

/* frees all associated memory in the UM_mem */
extern void free_memory(UM_Mem memory); 

/* creates a segment in UM_mem capable of holding num_words, 32-bit words and 
   returns seg_id */
extern int map_seg(UM_Mem memory, int num_words);  

/* pushes segment id onto mem_tracker and frees segment memory */
extern void unmap_seg(UM_Mem memory, int seg_id); 

/* returns address of a particular offset in a particular segment in memory */
extern void* mem_address(UM_Mem memory, int seg_id, int offset); 

/* loads 32-bit word into segment 0 */
extern void load_instruction(UM_Mem memory, const char* filename); 

/* mem segment at the seg_id is duplicated, and duplicate replaces segment 0*/
extern void load_segment(UM_Mem memory, int seg_id); 

/* returns the length of the segment associated with seg_id */
extern int segment_length(UM_Mem memory, int seg_id); 

/*prints out the sequence memory, and corresponding segments */
extern void print_mem_map(UM_Mem memory);

#endif 

