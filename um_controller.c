/**********************************************************************
 *
 *              um_controller.c 
 *
 *          File contains the implementation for the functions outlined in 
 *          um_controller.h. Module runs the execution of the UM and 
 *          instructions. Utilizes the UM interpreter module to decode 
 *          each 32-bit UM instruction. 
 *
 *          Written by: Ballard Blair and Siddharth Kapoor 
 *                  on: 4-14-16 
 *      
 ********************************************************************/ 


#include "um_mem.h"
#include "um_interpreter.h"
#include "um_controller.h"
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h> 
#include <stdbool.h>

#define seg_0 0 

/* function checks if the program counter is at last instruction */
static bool last_instruction (program_counter pc, UM_Mem m);

typedef enum Um_opcode {
        CMOV = 0, SLOAD, SSTORE, ADD, MUL, DIV,
        NAND, HALT, MAP, UNMAP, OUTPUT, INPUT, LOADP, LOADV
} Um_opcode;

struct program_counter { 
    uint32_t *location;
    int offset;
};

void execute (UM_Mem m)
{
    uint32_t registers [8] = {0, 0, 0, 0, 0, 0, 0, 0};
    struct program_counter pc = {NULL, 0};
    uint32_t command;
    bool halt_called = false;

    instruction decoded = new_instruction(); 

    //pc.location = mem_address (m, 0, 0); 

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
    if (pc -> offset == segment_length(m, seg_0)){
        return true;
    } else {
        return false;
    }
}

uint32_t get_instruction(UM_Mem m, program_counter pc)
{
    /* get next instruction */
    uint32_t instruction = *(uint32_t *)mem_address(m, seg_0, pc -> offset);
    
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

    registers[reg_a] = *((uint32_t *)(mem_address(m, registers[reg_b], 
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