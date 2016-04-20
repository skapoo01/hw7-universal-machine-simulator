/**********************************************************************
 *
 *              um.c 
 *
 *          This is the main() for the Universal Machine, initializes 
 *          the UM memory, loads .um program (32-bit instructions) then 
 *          runs an execution loop on the loaded program
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

//#include <uarray.h> 
//#include "unsafearray.h"

#include <assert.h>
#include <inttypes.h>
#include <sys/stat.h>
#include <except.h>
#include <stdbool.h>
#include <string.h>


#define op_width 4
#define reg_width 3
#define value_width 25 
#define op_lsb 28
#define value_lsb 0
#define reg_a_lsb1 6
#define reg_a_lsb2 25 
#define reg_b_lsb 3
#define reg_c_lsb 0 

Except_T Bitpack_Overflow = { "Overflow packing bits" };

typedef struct Array 
{
    int length;
    uint32_t elems[];
} *Array;


typedef struct instruction {
    uint32_t opcode;
    uint32_t register_a;
    uint32_t register_b;
    uint32_t register_c;
    uint32_t value;
} *instruction;

typedef struct UM_Mem {
    Seq_T memory;       /* A sequence of pointers to UArray_T segments */
    Stack_T mem_tracker;/* A stack of integer seg_id’s */
} *UM_Mem;

typedef enum Um_opcode {
        CMOV = 0, SLOAD, SSTORE, ADD, MUL, DIV,
        NAND, HALT, MAP, UNMAP, OUTPUT, INPUT, LOADP, LOADV
} Um_opcode;

typedef struct program_counter { 
    uint32_t *location;
    int offset;
} *program_counter;








static inline void Array_free (Array *a);

static inline Array Array_copy (Array a, int length);

static inline uint32_t* Array_at (Array a, int i);
static inline Array Array_new (int length);

static inline int Array_length (Array a);






static inline uint64_t Bitpack_getu(uint64_t word, unsigned width, unsigned lsb);
static inline uint64_t Bitpack_newu(uint64_t word, unsigned width, unsigned lsb,
                      uint64_t value);
static inline uint64_t shl(uint64_t word, unsigned bits);
static inline uint64_t shr(uint64_t word, unsigned bits);
static inline bool Bitpack_fitsu(uint64_t n, unsigned width);
/* function checks if the program counter is at last instruction */
static bool last_instruction (program_counter pc, UM_Mem m);

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
extern uint32_t* mem_address(UM_Mem memory, int seg_id, int offset); 

/* loads 32-bit word into segment 0 */
extern void load_instruction(UM_Mem memory, const char* filename); 

/* mem segment at the seg_id is duplicated, and duplicate replaces segment 0*/
extern void load_segment(UM_Mem memory, int seg_id); 

/* returns the length of the segment associated with seg_id */
extern int segment_length(UM_Mem memory, int seg_id); 

/*prints out the sequence memory, and corresponding segments */
extern void print_mem_map(UM_Mem memory);

/* parses 32-bit instrction into opcode and register usage */
void parse_instruction (uint32_t encoded, instruction decoded);

/* returns initialized instruction struct */
instruction new_instruction ();

/* returns reg_a in decoded */
uint32_t get_reg_a (instruction decoded);

/* returns reg_b in decoded */
uint32_t get_reg_b (instruction decoded);

/* returns reg_c in decoded */
uint32_t get_reg_c (instruction decoded);

/* returns opcode in decoded */
uint32_t get_opcode (instruction decoded);

/* returns value in decoded */
uint32_t get_value (instruction decoded);


/* Runs the execution of the UM and UM instructions */ 
void execute (UM_Mem m); 

/* updates pc and returns the next 32 bit word instruction from segment 0 */ 
uint32_t get_instruction(UM_Mem m, program_counter pc); 

/* function determines which instruction to execute based off of the opcode */
void handle_instruction (UM_Mem m, instruction operation, uint32_t* registers, 
                         program_counter pc, bool *halt_flag); 

/* checks if register C is 0, if not then the contents of reg_b are 
   moved to reg_a */
void conditional_move(uint32_t* registers, uint32_t reg_a, uint32_t reg_b, 
                      uint32_t reg_c); 

/* the contents in segment[reg_b][reg_c] are moved to reg_a */ 
void segmented_load (UM_Mem m, uint32_t* registers, uint32_t reg_a, 
                               uint32_t reg_b, uint32_t reg_c); 

/* memory segment [reg_a][reg_b] gets the contents of reg_a */
void segmented_store (UM_Mem m, uint32_t* registers, uint32_t reg_a, 
                                uint32_t reg_b, uint32_t reg_c); 

/* reg_a gets the sum of the contents of reg_b and reg_c */
void add (uint32_t* registers, uint32_t reg_a, uint32_t reg_b, 
          uint32_t reg_c);  

/* reg_a gets the product of the contents of reg_b and reg_c */
void multiply (uint32_t* registers, uint32_t reg_a, uint32_t reg_b, 
               uint32_t reg_c); 

/* reg_a gets the quotient of reg_b and reg_c */
void divide (uint32_t* registers, uint32_t reg_a, uint32_t reg_b, 
             uint32_t reg_c); 

/* reg_a gets ~(reg_b & reg_c) */
void bit_nand (uint32_t* registers, uint32_t reg_a, uint32_t reg_b, 
               uint32_t reg_c); 

/* computation stops and the UM is shut down */
void halt (); 

/* a new segment is created with a number of words equal to the value of 
   reg_c, each words in the new segment is initialized to 0, bit pattern of 
   not all zeroes and does not identify and mapped segment is put in reg_b, 
   new segment is mapped to segment[reg_b] */
void map_segment (UM_Mem m, uint32_t* registers, uint32_t reg_b, 
                            uint32_t reg_c); 

/* segment[reg_c] is unmapped */
void unmap_segment (UM_Mem m, uint32_t* registers, uint32_t reg_c); 

/* value of reg_c is displayed on I/O device, 
   only values 0 to 255 are allowed */ 
void output (uint32_t* registers, uint32_t reg_c); 

/* UM waits for input on I/O device, reg_c is loaded with input which 
must be a value from 0 to 255, if the end of input is signaled, reg_c is 
loaded with a 32­bit word in which every bit is 1 */
void input (uint32_t* registers, uint32_t reg_c); 

/* segment [reg_b] is duplicated and duplicate replaces segment[0], 
program counter is set to point to segment[0][reg_c] */
void load_program (UM_Mem m, uint32_t* registers, uint32_t reg_b, 
                             uint32_t reg_c, program_counter pc);

/* value is loaded into register a  */
void load_value (uint32_t* registers, uint32_t reg_a, uint32_t value);


int main (int argc, char const *argv[])
{
    /* .um file must be second command line argument */
    if (argc != 2) {
        fprintf(stderr, "Incorrect input\n");
        return EXIT_FAILURE;
    }
    /* initialize UM memory */
    UM_Mem memory = new_memory();
    //printf("seq length %d\n",Seq_length(memory->memory));

    //printf("before load instr\n");
    /* load .um program */
    load_instruction(memory, argv[1]);
    //printf("after load instr\n");
    execute(memory);
    
    free_memory(memory);

    return 0;
}


/* returns a new UM_Mem with an empty memory sequence capable of holding 
   UArray segments, and an empty mem_tracker stack */
UM_Mem new_memory()
{
    UM_Mem mem = malloc (sizeof(*mem)); 

    mem -> memory = Seq_new(50);
    mem -> mem_tracker = Stack_new (); 

    return mem; 
}

/* frees all associated memory in the UM_mem */
extern void free_memory(UM_Mem m)
{
    Array temp = NULL;
    int i = 0;

    int length = Seq_length(m -> memory);
    /* free every element of the sequence until it is empty */
    for (i = 0; i < length; i++){
            temp = Seq_get (m -> memory, i);
            Array_free (&temp);
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
    Array segment = Array_new(num_words);

    /* checks if stack of unmapped segments is empty */     
    if (Stack_empty (m -> mem_tracker) == 1){
            /* add segment to sequence */
            Seq_addhi (m -> memory, segment);
            return (Seq_length (m -> memory) - 1);
     } else {
            index = (uint64_t)Stack_pop (m -> mem_tracker);
            Array old = Seq_get(m -> memory, (int)index);
            Array_free(&old);
            Seq_put (m -> memory, (int)index, segment);
            return (int)index;
    }
} 

/* pushes segment id onto mem_tracker to be reused */
void unmap_seg(UM_Mem m, int seg_id)
{
    assert (seg_id < Seq_length(m -> memory));

    Array segment = Seq_get (m -> memory, seg_id);
    uint64_t seg_index = seg_id; 

    if (segment != NULL){   
            Stack_push (m -> mem_tracker, (void *)seg_index);
    }
} 

/* returns address of a particular offset in a particular segment in memory */
uint32_t* mem_address(UM_Mem m, int seg_id, int offset)
{       
    Array segment = Seq_get (m -> memory, seg_id);
    return Array_at(segment, offset);
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

    Array segment_0 = Array_new(num_instructions);
    int c = 0;
 
    while (counter < num_instructions) {
        for (int i = 3; i >= 0; i--) {
            c = getc(fp);
            if (c == EOF) { 
                break;
            }
            codeword = Bitpack_newu(codeword, 8, 8 * i, (uint64_t) c);
        }
        pointer = Array_at (segment_0, seg_index);
        *pointer = codeword;
        counter++;
        seg_index++;
    }
        

    Seq_addlo (m -> memory, segment_0);
            //printf("seq length after adding %d\n",Seq_length(m->memory));
               // print_mem_map(m);
    fclose(fp);
}

/* mem segment at the seg_id is duplicated, and duplicate replaces segment 0*/
void load_segment(UM_Mem m, int seg_id) 
{
    Array to_copy = Seq_get(m -> memory, seg_id);
    Array segment = Array_copy(to_copy, Array_length(to_copy));

    Array seg_0 = Seq_get(m -> memory, 0);       
    Array_free(&seg_0);

    Seq_put(m->memory, 0, segment);
}

/* returns the length of the segment associated with seg_id */
int segment_length(UM_Mem m, int seg_id)
{
    Array segment = Seq_get (m -> memory, seg_id);
    int length = Array_length(segment);
    return length;
} 

/* prints out the sequence memory, and corresponding segments */
void print_mem_map(UM_Mem m)
{
    Array segment;
    uint32_t *word;
    printf("Seq_length is %d\n", Seq_length(m->memory));
    for (int i= 0; i < Seq_length(m -> memory); i++) {
            segment = Seq_get(m -> memory, i);
            printf("Segment_%d[%d] : | ", i, segment_length(m, i));
                
            for (int j = 0; j < Array_length(segment); j++) {
                    word = Array_at(segment, j);
                    printf("{%d} %"PRIu32" |", j, *word);
            }        
            printf("\n");
    }
    printf("outta print mem\n");
}

void parse_instruction (uint32_t encoded, instruction decoded)
{
    uint32_t opcode = Bitpack_getu(encoded, op_width, op_lsb);

    if (opcode == 13){
        decoded -> opcode = opcode; 
        decoded -> register_a = Bitpack_getu(encoded, reg_width, reg_a_lsb2);
        decoded -> value =      Bitpack_getu(encoded, value_width, value_lsb);
    } else {
        decoded -> opcode = opcode;
        decoded -> register_a = Bitpack_getu(encoded, reg_width, reg_a_lsb1);
        decoded -> register_b = Bitpack_getu(encoded, reg_width, reg_b_lsb);
        decoded -> register_c = Bitpack_getu(encoded, reg_width, reg_c_lsb);
    }
}

instruction new_instruction ()
{
    instruction decoded = malloc (sizeof(*decoded));

    decoded -> opcode = 0;
    decoded -> register_a = 0; 
    decoded -> register_b = 0; 
    decoded -> register_c = 0; 
    decoded -> value = 0;
    return decoded; 
}

uint32_t get_reg_a (instruction decoded)
{
    uint32_t register_a = decoded -> register_a;
    return register_a;
}

uint32_t get_reg_b (instruction decoded)
{
    uint32_t register_b = decoded -> register_b;
    return register_b;
}

uint32_t get_reg_c (instruction decoded)
{
    uint32_t register_c = decoded -> register_c;
    return register_c;
}

uint32_t get_opcode (instruction decoded)
{
    int opcode = decoded -> opcode;
    return opcode;
}

uint32_t get_value (instruction decoded)
{
    int value = decoded -> value;
    return value;
}


void execute (UM_Mem m)
{
    uint32_t registers [8] = {0, 0, 0, 0, 0, 0, 0, 0};
    struct program_counter pc = {NULL, 0};
    uint32_t command;
    bool halt_called = false;

    instruction decoded = new_instruction(); 
   // print_mem_map(m);
    while (!last_instruction(&pc, m)) {
        if (halt_called == true) {
            break;
        }
        command = get_instruction(m, &pc);
        parse_instruction (command, decoded);
        handle_instruction(m, decoded, registers, &pc, &halt_called);
    }
   // print_mem_map(m);
    free(decoded);
}

static bool last_instruction (program_counter pc, UM_Mem m)
{
    if (pc -> offset == segment_length(m, 0)){
        return true;
    } else {
        return false;
    }
}

uint32_t get_instruction(UM_Mem m, program_counter pc)
{
    /* get next instruction */
    uint32_t instruction = *mem_address(m, 0, pc -> offset);
    
    /* update program counter */
    pc -> offset++;
    return instruction;
}

void handle_instruction (UM_Mem m, instruction decoded, uint32_t* registers, 
                        program_counter pc, bool *halt_flag)
{
    uint32_t opcode = get_opcode (decoded);
    uint32_t register_a = get_reg_a (decoded);
    uint32_t register_b = get_reg_b (decoded);
    uint32_t register_c = get_reg_c (decoded);
    uint32_t value = get_value (decoded);

    switch (opcode) {
        case CMOV :
            //printf("CMOV\n");
            conditional_move (registers, register_a, register_b, register_c);
            break;

        case SLOAD:

            //printf("SLOAD\n");
            segmented_load (m, registers, register_a, register_b, register_c);
            break;

        case SSTORE:

            //printf("SSTORE\n");
            segmented_store (m, registers, register_a, register_b, 
                                register_c);
            break;

        case ADD:

            //printf("ADD\n");
            add(registers, register_a, register_b, register_c); 
            break;

        case MUL:

            //printf("MUL\n");
            multiply(registers, register_a, register_b, register_c);
            break;

        case DIV:

            //printf("DIV\n");
            divide(registers, register_a, register_b, register_c);
            break;

        case NAND:
            //printf("NAND\n");

            bit_nand(registers, register_a, register_b, register_c);
            break; 

        case HALT:
            //printf("HALT\n");

            halt(halt_flag);
            break;

        case MAP:
            //printf("MAPPING\n");

            map_segment(m, registers, register_b, register_c);
            //print_mem_map(m);
            break;

        case UNMAP:
            //printf("UNMAPPING\n");

            unmap_segment(m, registers, register_c);
            break;

        case OUTPUT:
            //printf("OUTPUT\n");

            output(registers, register_c);
            break;

        case INPUT:
            //printf("INPUT\n");
            input(registers, register_c); 
            break; 

        case LOADP:
            //printf("LOADP\n");

            load_program(m, registers, register_b, register_c, pc);
            break;

        case LOADV:
            //printf("LOADV\n");

            load_value(registers, register_a, value);
            break;

        default:
            exit(1);

    }

}

void conditional_move(uint32_t* registers, uint32_t reg_a, uint32_t reg_b, 
                      uint32_t reg_c)
{
    if (registers[reg_c] != 0) {
        registers[reg_a] = registers[reg_b];
    }
}

void segmented_load (UM_Mem m, uint32_t* registers, uint32_t reg_a, 
                               uint32_t reg_b, uint32_t reg_c)
{

    registers[reg_a] = *((mem_address(m, registers[reg_b], 
                                                     registers[reg_c])));
} 

void segmented_store (UM_Mem m, uint32_t* registers, uint32_t reg_a, 
                                uint32_t reg_b, uint32_t reg_c)
{

    uint32_t *mem_loc = mem_address(m, registers[reg_a], registers[reg_b]);
    
    *mem_loc = registers[reg_c];
    
} 

void add (uint32_t* registers, uint32_t reg_a, uint32_t reg_b, uint32_t reg_c)
{
    registers[reg_a] = (registers[reg_b] + registers[reg_c]);
}  

void multiply (uint32_t* registers, uint32_t reg_a, uint32_t reg_b, 
               uint32_t reg_c)
{
    registers[reg_a] = (registers[reg_b] * registers[reg_c]);

}

void divide (uint32_t* registers, uint32_t reg_a, uint32_t reg_b, 
             uint32_t reg_c)
{
    registers[reg_a] = (registers[reg_b] / registers[reg_c]);
} 

void bit_nand (uint32_t* registers, uint32_t reg_a, uint32_t reg_b, 
              uint32_t reg_c)
{
    registers[reg_a] = (uint32_t) ~((uint32_t)(registers[reg_b] & 
                                               registers[reg_c]));

} 

void halt (bool* halt_flag)
{
    *halt_flag = true;
} 

void map_segment (UM_Mem m, uint32_t* registers, uint32_t reg_b, 
                            uint32_t reg_c)
{
    int num_words = registers[reg_c];
    registers[reg_b] = map_seg(m, num_words);
}


void unmap_segment (UM_Mem m, uint32_t* registers, uint32_t reg_c)
{
    unmap_seg(m, registers[reg_c]);
}

void output (uint32_t* registers, uint32_t reg_c)
{
    fputc(registers[reg_c], stdout);
} 

void input (uint32_t* registers, uint32_t reg_c)
{
    int c = fgetc(stdin);
    if (c < 0 || c > 255) {
        registers[reg_c] = UINT32_MAX;
    } else {
        registers[reg_c] = (uint32_t) c;
    }
}

void load_program (UM_Mem m, uint32_t* registers, uint32_t reg_b, 
                             uint32_t reg_c, program_counter pc)
{
    if (registers[reg_b] != 0) {
        load_segment(m, registers[reg_b]);
    }
    /* update program counter */
    pc -> offset = registers[reg_c];
}

void load_value (uint32_t* registers, uint32_t reg_a, uint32_t value)
{   
    registers[reg_a] = value;
}

/* ------------------------ Optimization Functions------------------ */

static inline uint64_t Bitpack_getu(uint64_t word, unsigned width, unsigned lsb)
{
        unsigned hi = lsb + width; /* one beyond the most significant bit */
        assert(hi <= 64);
        /* different type of right shift */
        return shr(shl(word, 64 - hi), 64 - width); 
}

static inline uint64_t Bitpack_newu(uint64_t word, unsigned width, unsigned lsb,
                      uint64_t value)
{
        unsigned hi = lsb + width; /* one beyond the most significant bit */
        assert(hi <= 64);
        if (!Bitpack_fitsu(value, width))
                RAISE(Bitpack_Overflow);
        return shl(shr(word, hi), hi)                 /* high part */
                | shr(shl(word, 64 - lsb), 64 - lsb)  /* low part  */
                | (value << lsb);                     /* new part  */
}


static inline uint64_t shl(uint64_t word, unsigned bits)
{
        assert(bits <= 64);
        if (bits == 64)
                return 0;
        else
                return word << bits;
}

/*
 * shift R logical
 */
static inline uint64_t shr(uint64_t word, unsigned bits)
{
        assert(bits <= 64);
        if (bits == 64)
                return 0;
        else
                return word >> bits;
}

static inline bool Bitpack_fitsu(uint64_t n, unsigned width)
{
        if (width >= 64)
                return true;
        /* thanks to Jai Karve and John Bryan */
        return shr(n, width) == 0; // clever shortcut instead of 2 shifts
}

/* ------------------------ Optimization Functions round 2 with our own array impl------------------ */
/* ------------------------ Optimization Functions round 2 with our own array impl------------------ */
/* ------------------------ Optimization Functions round 2 with our own array impl------------------ */
/* ------------------------ Optimization Functions round 2 with our own array impl------------------ */

static inline int Array_length (Array a)
{
    return a -> length;
}

static inline uint32_t* Array_at (Array a, int i)
{
    return &(a->elems[i]);

} 

static inline void Array_free (Array *a)
{

    free(*a);
} 

static inline Array Array_copy (Array a, int length)
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


static inline Array Array_new (int length)
{
    Array a = malloc(sizeof(*a) + length * sizeof(*a->elems));
    a -> length = length;
    for (int i = 0; i < length; i++){
        a->elems[i] = 0;
    }
    return a;
}