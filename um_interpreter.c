/**********************************************************************
 *
 *              um_interpreter.c 
 *
 *          File contains the implementation for the functions outlined in 
 *          um_interpreter.h This module parses 32 bit instructions into 
 *          its corresponding opcode and register usage.  
 *
 *          Written by: Ballard Blair and Siddharth Kapoor 
 *                  on: 4-11-16 
 *      
 ********************************************************************/

#include <bitpack.h>
#include <stdlib.h>
#include <assert.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include "um_interpreter.h"

#define op_width 4
#define reg_width 3
#define value_width 25 
#define op_lsb 28
#define value_lsb 0
#define reg_a_lsb1 6
#define reg_a_lsb2 25 
#define reg_b_lsb 3
#define reg_c_lsb 0 

struct instruction {
    uint32_t opcode;
    uint32_t register_a;
    uint32_t register_b;
    uint32_t register_c;
    uint32_t value;
};

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
