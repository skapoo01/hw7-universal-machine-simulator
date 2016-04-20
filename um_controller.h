/**********************************************************************
 *
 *              um_controller.h 
 *          Header file for um controller module which runs the execution of 
 *          the UM and instructions, utilizes the UM interpreter module to 
 *          decode each 32-bit UM instruction. 
 *   
 *          Written by: Ballard Blair and Siddharth Kapoor 
 *                  on: 4-14-16 
 *      
 ********************************************************************/ 

#ifndef UM_CONTROLLER_INCLUDED
#define UM_CONTROLLER_INCLUDED

#include "um_interpreter.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct program_counter *program_counter; 

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



#endif