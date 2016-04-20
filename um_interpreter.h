/**********************************************************************
 *
 *              um_interpreter.h 
 *
 *          This header file for the um_interpreter module parses 32 bit 
 *          instructions into its corresponding opcode and register usage.  
 *
 *          Written by: Ballard Blair and Siddharth Kapoor 
 *                  on: 4-14-16 
 *
 ********************************************************************/
#ifndef UM_INTERPRETER_INCLUDED
#define UM_INTERPRETER_INCLUDED
#include <stdint.h>

typedef struct instruction *instruction;

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


#endif