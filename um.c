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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
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


typedef struct Stack  
{
    int Length, capacity;
    uint64_t *elems;
} *Stack;

typedef struct Sequence  
{
    int Length, capacity;
    Array *elems;
} *Sequence;
typedef struct instruction {
    uint32_t opcode;
    uint32_t register_a;
    uint32_t register_b;
    uint32_t register_c;
    uint32_t value;
} *instruction;

typedef struct UM_Mem {
    Sequence memory;       /* A sequence of pointers to UArray_T segments */
    Stack mem_tracker;/* A stack of integer seg_id’s */
} *UM_Mem;

typedef enum Um_opcode {
        CMOV = 0, SLOAD, SSTORE, ADD, MUL, DIV,
        NAND, HALT, MAP, UNMAP, OUTPUT, INPUT, LOADP, LOADV
} Um_opcode;

typedef struct program_counter { 
    uint32_t *location;
    int offset;
} *program_counter;


static inline Sequence Seq_new (int hint);
static inline void Seq_addhi (Sequence s, Array a);
static inline void Seq_expand (Sequence s);
static inline Array Seq_get (Sequence s, int index);
static inline int Seq_length (Sequence s);
static inline void Seq_put (Sequence s, int index, Array a);
static inline void Seq_free(Sequence *s);
static inline Stack Stack_new ();
static inline void Stack_expand (Stack s);
static inline void Stack_push (Stack s, uint64_t a);
static inline uint64_t Stack_pop (Stack s);
static inline bool Stack_empty(Stack s);
static inline void Stack_free(Stack *s);
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
static inline UM_Mem new_memory();

/* frees all associated memory in the UM_mem */
static inline void free_memory(UM_Mem memory); 

/* creates a segment in UM_mem capable of holding num_words, 32-bit words and 
   returns seg_id */
static inline int map_seg(UM_Mem memory, int num_words);  

/* pushes segment id onto mem_tracker and frees segment memory */
static inline void unmap_seg(UM_Mem memory, int seg_id); 

/* returns address of a particular offset in a particular segment in memory */
static inline uint32_t* mem_address(UM_Mem memory, int seg_id, int offset); 

/* loads 32-bit word into segment 0 */
static inline void load_instruction(UM_Mem memory, const char* filename); 

/* mem segment at the seg_id is duplicated, and duplicate replaces segment 0*/
static inline void load_segment(UM_Mem memory, int seg_id); 

/* returns the length of the segment associated with seg_id */
static inline int segment_length(UM_Mem memory, int seg_id); 

/*prints out the sequence memory, and corresponding segments */
static inline void print_mem_map(UM_Mem memory);

/* parses 32-bit instrction into opcode and register usage */
static inline void parse_instruction (uint32_t encoded, instruction decoded);

/* returns initialized instruction struct */
static inline instruction new_instruction ();

/* Runs the execution of the UM and UM instructions */ 
static inline void execute (UM_Mem m); 

/* updates pc and returns the next 32 bit word instruction from segment 0 */ 
static inline uint32_t get_instruction(UM_Mem m, program_counter pc); 

/* function determines which instruction to execute based off of the opcode */
static inline void handle_instruction (UM_Mem m, instruction operation, uint32_t* registers, 
                         program_counter pc, bool *halt_flag); 

/* checks if register C is 0, if not then the contents of reg_b are 
   moved to reg_a */
static inline void conditional_move(uint32_t* registers, uint32_t reg_a, uint32_t reg_b, 
                      uint32_t reg_c); 

/* the contents in segment[reg_b][reg_c] are moved to reg_a */ 
static inline void segmented_load (UM_Mem m, uint32_t* registers, uint32_t reg_a, 
                               uint32_t reg_b, uint32_t reg_c); 

/* memory segment [reg_a][reg_b] gets the contents of reg_a */
static inline void segmented_store (UM_Mem m, uint32_t* registers, uint32_t reg_a, 
                                uint32_t reg_b, uint32_t reg_c); 

/* reg_a gets the sum of the contents of reg_b and reg_c */
static inline void add (uint32_t* registers, uint32_t reg_a, uint32_t reg_b, 
          uint32_t reg_c);  

/* reg_a gets the product of the contents of reg_b and reg_c */
static inline void multiply (uint32_t* registers, uint32_t reg_a, uint32_t reg_b, 
               uint32_t reg_c); 

/* reg_a gets the quotient of reg_b and reg_c */
static inline void divide (uint32_t* registers, uint32_t reg_a, uint32_t reg_b, 
             uint32_t reg_c); 

/* reg_a gets ~(reg_b & reg_c) */
static inline void bit_nand (uint32_t* registers, uint32_t reg_a, uint32_t reg_b, 
               uint32_t reg_c); 

/* computation stops and the UM is shut down */
static inline void halt (); 

/* a new segment is created with a number of words equal to the value of 
   reg_c, each words in the new segment is initialized to 0, bit pattern of 
   not all zeroes and does not identify and mapped segment is put in reg_b, 
   new segment is mapped to segment[reg_b] */
static inline void map_segment (UM_Mem m, uint32_t* registers, uint32_t reg_b, 
                            uint32_t reg_c); 

/* segment[reg_c] is unmapped */
static inline void unmap_segment (UM_Mem m, uint32_t* registers, uint32_t reg_c); 

/* value of reg_c is displayed on I/O device, 
   only values 0 to 255 are allowed */ 
static inline void output (uint32_t* registers, uint32_t reg_c); 

/* UM waits for input on I/O device, reg_c is loaded with input which 
must be a value from 0 to 255, if the end of input is signaled, reg_c is 
loaded with a 32­bit word in which every bit is 1 */
static inline void input (uint32_t* registers, uint32_t reg_c); 

/* segment [reg_b] is duplicated and duplicate replaces segment[0], 
program counter is set to point to segment[0][reg_c] */
static inline void load_program (UM_Mem m, uint32_t* registers, uint32_t reg_b, 
                             uint32_t reg_c, program_counter pc);

/* value is loaded into register a  */
static inline void load_value (uint32_t* registers, uint32_t reg_a, uint32_t value);


int main (int argc, char const *argv[])
{
    /* .um file must be second command line argument */
    if (argc != 2) {
        fprintf(stderr, "Incorrect input\n");
        return EXIT_FAILURE;
    }
    /* initialize UM memory */
    UM_Mem memory = new_memory();

    /* load .um program */
    load_instruction(memory, argv[1]);
    execute(memory);
    free_memory(memory);
    return 0;
}


/* returns a new UM_Mem with an empty memory sequence capable of holding 
   UArray segments, and an empty mem_tracker stack */
static inline UM_Mem new_memory()
{
    UM_Mem mem = malloc (sizeof(*mem)); 

    mem -> memory = Seq_new(50);
    mem -> mem_tracker = Stack_new (); 

    return mem; 
}

/* frees all associated memory in the UM_mem */
static inline void free_memory(UM_Mem m)
{
    Array temp = NULL;
    int length = Seq_length(m -> memory);
    /* free every element of the sequence until it is empty */
    for (int i = 0; i < length; i++){
            temp = Seq_get (m -> memory, i);
            Array_free (&temp);
    }

    Seq_free (&(m -> memory));
    Stack_free (&(m -> mem_tracker));
    free(m);
}

/* creates a segment in UM_mem capable of holding num_words, 32-bit words and 
   returns seg_id */
static inline int map_seg(UM_Mem m, int num_words)
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
            index = Stack_pop (m -> mem_tracker);
            Array old = Seq_get(m -> memory, index);
            Array_free(&old);
            Seq_put (m -> memory, index, segment);
            return (int)index;
    }
} 

/* pushes segment id onto mem_tracker to be reused */
static inline void unmap_seg(UM_Mem m, int seg_id)
{
    assert (seg_id < Seq_length(m -> memory));

    Array segment = Seq_get (m -> memory, seg_id);
    uint64_t seg_index = seg_id; 

    if (segment != NULL){   
            Stack_push (m -> mem_tracker, seg_index);
    }
} 

/* returns address of a particular offset in a particular segment in memory */
static inline uint32_t* mem_address(UM_Mem m, int seg_id, int offset)
{       
    Array segment = Seq_get (m -> memory, seg_id);
    return Array_at(segment, offset);
} 

/* loads 32-bit word into segment 0 */
static inline void load_instruction(UM_Mem m, const char* filename) 
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
    Seq_addhi (m -> memory, segment_0);
    fclose(fp);
}

/* mem segment at the seg_id is duplicated, and duplicate replaces segment 0*/
static inline void load_segment(UM_Mem m, int seg_id) 
{
    Array to_copy = Seq_get(m -> memory, seg_id);
    Array segment = Array_copy(to_copy, Array_length(to_copy));

    Array seg_0 = Seq_get(m -> memory, 0);       
    Array_free(&seg_0);

    Seq_put(m->memory, 0, segment);
}

/* returns the length of the segment associated with seg_id */
static inline int segment_length(UM_Mem m, int seg_id)
{
    Array segment = Seq_get (m -> memory, seg_id);
    int length = Array_length(segment);
    return length;
} 

/* prints out the sequence memory, and corresponding segments */
static inline void print_mem_map(UM_Mem m)
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
}

static inline void parse_instruction (uint32_t encoded, instruction decoded)
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

static inline instruction new_instruction ()
{
    instruction decoded = malloc (sizeof(*decoded));

    decoded -> opcode = 0;
    decoded -> register_a = 0; 
    decoded -> register_b = 0; 
    decoded -> register_c = 0; 
    decoded -> value = 0;
    return decoded; 
}


static inline void execute (UM_Mem m)
{
    uint32_t registers [8] = {0, 0, 0, 0, 0, 0, 0, 0};
    struct program_counter pc = {NULL, 0};
    uint32_t command;
    bool halt_called = false;

    instruction decoded = new_instruction(); 
    while (!last_instruction(&pc, m)) {
        if (halt_called == true) {
            break;
        }
        command = get_instruction(m, &pc);
        parse_instruction (command, decoded);
        handle_instruction(m, decoded, registers, &pc, &halt_called);
    }
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

static inline uint32_t get_instruction(UM_Mem m, program_counter pc)
{
    /* get next instruction */
    uint32_t instruction = *mem_address(m, 0, pc -> offset);
    
    /* update program counter */
    pc -> offset++;
    return instruction;
}

static inline void handle_instruction (UM_Mem m, instruction decoded, uint32_t* registers, 
                        program_counter pc, bool *halt_flag)
{
    uint32_t opcode = decoded->opcode;
    uint32_t register_a = decoded->register_a;
    uint32_t register_b = decoded->register_b;
    uint32_t register_c = decoded->register_c;
    uint32_t value = decoded->value;

    switch (opcode) {
        case CMOV :
            conditional_move (registers, register_a, register_b, register_c);
            break;
        case SLOAD:
            segmented_load (m, registers, register_a, register_b, register_c);
            break;
        case SSTORE:
            segmented_store (m, registers, register_a, register_b, 
                                register_c);
            break;
        case ADD:
            add(registers, register_a, register_b, register_c); 
            break;
        case MUL:
            multiply(registers, register_a, register_b, register_c);
            break;
        case DIV:
            divide(registers, register_a, register_b, register_c);
            break;
        case NAND:
            bit_nand(registers, register_a, register_b, register_c);
            break; 
        case HALT:
            halt(halt_flag);
            break;
        case MAP:
            map_segment(m, registers, register_b, register_c);
            break;
        case UNMAP:
            unmap_segment(m, registers, register_c);
            break;
        case OUTPUT:
            output(registers, register_c);
            break;
        case INPUT:
            input(registers, register_c); 
            break;
        case LOADP:
            load_program(m, registers, register_b, register_c, pc);
            break;
        case LOADV:
            load_value(registers, register_a, value);
            break;
        default:
            exit(1);
    }
}

static inline void conditional_move(uint32_t* registers, uint32_t reg_a, uint32_t reg_b, 
                      uint32_t reg_c)
{
    if (registers[reg_c] != 0) {
        registers[reg_a] = registers[reg_b];
    }
}

static inline void segmented_load (UM_Mem m, uint32_t* registers, uint32_t reg_a, 
                               uint32_t reg_b, uint32_t reg_c)
{

    registers[reg_a] = *((mem_address(m, registers[reg_b], 
                                                     registers[reg_c])));
} 

static inline void segmented_store (UM_Mem m, uint32_t* registers, uint32_t reg_a, 
                                uint32_t reg_b, uint32_t reg_c)
{

    uint32_t *mem_loc = mem_address(m, registers[reg_a], registers[reg_b]);
    
    *mem_loc = registers[reg_c];
    
} 

static inline void add (uint32_t* registers, uint32_t reg_a, uint32_t reg_b, uint32_t reg_c)
{
    registers[reg_a] = (registers[reg_b] + registers[reg_c]);
}  

static inline void multiply (uint32_t* registers, uint32_t reg_a, uint32_t reg_b, 
               uint32_t reg_c)
{
    registers[reg_a] = (registers[reg_b] * registers[reg_c]);

}

static inline void divide (uint32_t* registers, uint32_t reg_a, uint32_t reg_b, 
             uint32_t reg_c)
{
    registers[reg_a] = (registers[reg_b] / registers[reg_c]);
} 

static inline void bit_nand (uint32_t* registers, uint32_t reg_a, uint32_t reg_b, 
              uint32_t reg_c)
{
    registers[reg_a] = (uint32_t) ~((uint32_t)(registers[reg_b] & 
                                               registers[reg_c]));

} 

static inline void halt (bool* halt_flag)
{
    *halt_flag = true;
} 

static inline void map_segment (UM_Mem m, uint32_t* registers, uint32_t reg_b, 
                            uint32_t reg_c)
{
    int num_words = registers[reg_c];
    registers[reg_b] = map_seg(m, num_words);
}


static inline void unmap_segment (UM_Mem m, uint32_t* registers, uint32_t reg_c)
{
    unmap_seg(m, registers[reg_c]);
}

static inline void output (uint32_t* registers, uint32_t reg_c)
{
    fputc(registers[reg_c], stdout);
} 

static inline void input (uint32_t* registers, uint32_t reg_c)
{
    int c = fgetc(stdin);
    if (c < 0 || c > 255) {
        registers[reg_c] = UINT32_MAX;
    } else {
        registers[reg_c] = (uint32_t) c;
    }
}

static inline void load_program (UM_Mem m, uint32_t* registers, uint32_t reg_b, 
                             uint32_t reg_c, program_counter pc)
{
    if (registers[reg_b] != 0) {
        load_segment(m, registers[reg_b]);
    }
    /* update program counter */
    pc -> offset = registers[reg_c];
}

static inline void load_value (uint32_t* registers, uint32_t reg_a, uint32_t value)
{   
    registers[reg_a] = value;
}

/* ------------------------ Optimization Functions------------------ */

static inline uint64_t Bitpack_getu(uint64_t word, unsigned width, unsigned lsb)
{
        unsigned hi = lsb + width; /* one beyond the most significant bit */
        /* different type of right shift */
        return shr(shl(word, 64 - hi), 64 - width); 
}

static inline uint64_t Bitpack_newu(uint64_t word, unsigned width, unsigned lsb,
                      uint64_t value)
{
        unsigned hi = lsb + width; /* one beyond the most significant bit */
        return shl(shr(word, hi), hi)                 /* high part */
                | shr(shl(word, 64 - lsb), 64 - lsb)  /* low part  */
                | (value << lsb);                     /* new part  */
}


static inline uint64_t shl(uint64_t word, unsigned bits)
{
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
/* ------------------------ Optimization Functions round 2 with our own SEQ impl------------------ */
/* ------------------------ Optimization Functions round 2 with our own SEQ impl------------------ */
/* ------------------------ Optimization Functions round 2 with our own SEQ impl------------------ */
/* ------------------------ Optimization Functions round 2 with our own SEQ impl------------------ */
static inline Sequence Seq_new (int hint)
{
    Sequence s = malloc(sizeof(*s));
    s->capacity = hint;
    s->Length = 0;
    s->elems = malloc(hint * (sizeof(*(s->elems))));
    return s;
}

static inline void Seq_expand (Sequence s)
{
    Array *new = malloc((s->capacity * 2) * (sizeof(*(s->elems))));
    for (int i = 0; i < s->Length; i++){
        new[i] = s->elems[i];
    }
    Array *temp = s->elems;
    free(temp);
    s->capacity = s->capacity * 2; 
    s->elems = new;
}

static inline void Seq_addhi (Sequence s, Array a)
{
    if (s -> Length == s-> capacity){
        Seq_expand(s);
    }
    s->elems[s->Length] = a;
    s->Length++;
}

static inline Array Seq_get (Sequence s, int index)
{
    return s->elems[index];
}

static inline int Seq_length (Sequence s)
{
    return s->Length;
}

static inline void Seq_put (Sequence s, int index, Array a)
{
    s->elems[index] = a;
}

static inline void Seq_free(Sequence *s)
{
    free((*s)->elems);
    free(*s);

}
/* ------------------------ Optimization Functions round 2 with our own SEQ impl------------------ */
/* ------------------------ Optimization Functions round 2 with our own SEQ impl------------------ */
/* ------------------------ Optimization Functions round 2 with our own SEQ impl------------------ */
/* ------------------------ Optimization Functions round 2 with our own SEQ impl------------------ */

static inline Stack Stack_new ()
{
    Stack s = malloc(sizeof(*s));
    s->capacity = 50;
    s->Length = 0;
    s->elems = malloc(50 * (sizeof(*(s->elems))));
    return s;
}

static inline void Stack_expand (Stack s)
{
    uint64_t *new = malloc((s->capacity * 2) * (sizeof(*(s->elems))));
    for (int i = 0; i < s->Length; i++){
        new[i] = s->elems[i];
    }
    s->capacity = s->capacity * 2; 
    uint64_t *temp = s->elems;
    free(temp);
    s->elems = new;
}

static inline void Stack_push (Stack s, uint64_t a)
{
    if (s -> Length == s-> capacity){
        Stack_expand(s);
    }
    s->elems[s->Length] = a;
    s->Length++;
}

static inline uint64_t Stack_pop (Stack s)
{
    s->Length--;
    return s->elems[s->Length];

}

static inline bool Stack_empty(Stack s)
{
    if (s->Length == 0){
        return true;
    } else {
        return false;
    }
}

static inline void Stack_free(Stack *s)
{

    free((*s)->elems);
    free(*s);

}