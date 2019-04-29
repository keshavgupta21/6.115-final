/* ============================================================================
 * CHIP8 Emulator
 * Written By Keshav Gupta, April 2019
 * Technical Reference: http://devernay.free.fr/hacks/chip8/C8TECH10.HTM
 * ============================================================================
*/

/* Instruction Opcodes */
#define OP3_SYSTEM 0x0
#define OP3_JUMP_ABSOLUTE 0x1
#define OP3_JUMP_OFFSET 0xb
#define OP3_CALL_SUBROUTINE 0x2
#define OP3_SKIP_NEXT_EQ_IM 0x3
#define OP3_SKIP_NEXT_NEQ_IM 0x4
#define OP3_SKIP_NEXT_EQ 0x5
#define OP3_SKIP_NEXT_NEQ 0x9
#define OP3_LOAD_V 0x6
#define OP3_ADD_IM 0x7
#define OP3_ARITH 0x8
#define OP3_LOAD_I 0xa
#define OP3_RANDOM 0xc
#define OP3_DRAW 0xd
#define OP3_KEYBOARD 0xe
#define OP3_SPECIAL 0xf

#define OP2_SYSTEM 0x0

#define OP10_SYSTEM_CLEAR 0xe0
#define OP10_SYSTEM_RETURN 0xee
#define OP10_KEYBOARD_EQ 0x9e
#define OP10_KEYBOARD_NEQ 0xa1
#define OP10_SPECIAL_LOAD_DELAY 0x07
#define OP10_SPECIAL_LOAD_KEYBOARD 0x0a
#define OP10_SPECIAL_SET_DELAY 0x15
#define OP10_SPECIAL_SET_SOUND 0x18
#define OP10_SPECIAL_ADD_TO_I 0x1e
#define OP10_SPECIAL_LOAD_FONT 0x29
#define OP10_SPECIAL_LOAD_BCD 0x33
#define OP10_SPECIAL_LOAD_AT_I 0x55
#define OP10_SPECIAL_SET_AT_I 0x65

#define OP0_SKIP_NEXT 0x0
#define OP0_ARITH_LOAD 0x0
#define OP0_ARITH_OR 0x1
#define OP0_ARITH_AND 0x2
#define OP0_ARITH_XOR 0x3
#define OP0_ARITH_ADD 0x4
#define OP0_ARITH_SUB 0x5
#define OP0_ARITH_SHR 0x6
#define OP0_ARITH_SUBN 0x7
#define OP0_ARITH_SHL 0xe

#define RANDOM_BYTE 0x69 // TODO

#include "project.h"

/* CHIP8 VRAM 
    Each row is one 64bit number
*/
uint64_t vram[32];

/* CHIP8 Timers 
    Decrement each at 60Hz if not zero
*/
uint8_t snd = 0, tmr = 0;

/* CHIP8 Keyboard 
    keys stores the current state of each key
        (1 = down, 0 = up)
    lastkey stores the last key that was pressed
    leyflag = 1 if new data available in lastkey
        must be reset to 0 when data used
*/
uint8_t keys[16] = {0};
uint8_t lastkey = 0, keyflag = 0;

int main(void){
    /* Enable global interrupts. */
    CyGlobalIntEnable;
    
    /* CHIP8 RAM*/
    uint8_t ram[4096];
    // TODO fonts
    
    /* CHIP8 Register File*/
    uint8_t v[16];
    uint16_t i = 0;
    uint16_t pc = 0x0200;
    uint8_t sp = 0x00;
    uint16_t stack[16] = {0};
    
    /* Clear screen */
    for (uint8_t j = 0; j < 32; j++){
        vram[j] = 0;
    }
    
    /* Emulate away.. */
    while(1){
        /* Get the instruction at the current address */
        uint16_t instr = (ram[pc] << 8) | ram[pc+1];
        
        /* Decode the opcodes */
        uint8_t op3 = (instr & 0xf000) >> 12;
        uint8_t op2 = (instr & 0x0f00) >> 8;
        uint8_t op10 = instr & 0x00ff;
        uint8_t op0 = instr & 0x000f;
        
        /* Decode the immediates */
        uint16_t nnn = instr & 0x0fff;
        uint8_t x = (instr & 0x0f00) >> 8;
        uint8_t y = (instr & 0x00f0) >> 4;
        uint8_t kk = instr & 0x00ff;
        uint8_t n = instr & 0x000f;
        
        /* This flag is set if an invalid OP is encountered */
        uint8_t invalid_op = 0;
        
        /* This flag is set if a new PC was set and must not be incremented */
        uint8_t freeze_pc = 0;
        
        /* Branch on opcodes and execute */
        switch(op3){
        case OP3_SYSTEM:
            /* System operations */
            if (op2 != OP2_SYSTEM){
                invalid_op = 1;
            } else {
                switch(op10){
                case OP10_SYSTEM_CLEAR:
                    /* Clear screen */
                    for (int j = 0; j < 32; j++){
                        vram[j] = 0;
                    }
                    break;
                case OP10_SYSTEM_RETURN:
                    /* Return from instruction */
                    pc = stack[sp];
                    sp = (sp == 0x0) ? 0x0 : sp - 1;
                    freeze_pc = 1;
                    break;
                default:
                    invalid_op = 1;
                    break;
                }
            }
            break;
        case OP3_JUMP_ABSOLUTE:
            /* Jump to absolute address nnn */
            pc = nnn;
            freeze_pc = 1;
            break;
        case OP3_JUMP_OFFSET:
            /* Jump to absolute address nnn + offset V0 */
            pc = nnn + v[0];
            freeze_pc = 1;
            break;
        case OP3_CALL_SUBROUTINE:
            /* Call subroutine at nnn */
            stack[sp] = pc;
            sp = (sp == 0xf) ? 0xf : sp + 1;
            pc = nnn;
            freeze_pc = 1;
            break;
        case OP3_SKIP_NEXT_EQ_IM:
            /* Skip next instruction if Vx == kk */
            if (op0 == OP0_SKIP_NEXT){
                if (v[x] == kk) {
                    pc += 2;
                }
            } else {
                invalid_op = 1;
            }
            break;
        case OP3_SKIP_NEXT_NEQ_IM:
            /* Skip next instruction if Vx != kk */
            if (op0 == OP0_SKIP_NEXT){
                if (v[x] != kk) {
                    pc += 2;
                }
            } else {
                invalid_op = 1;
            }
            break;
        case OP3_SKIP_NEXT_EQ:
            /* Skip next instruction if Vx == Vy */
            if (op0 == OP0_SKIP_NEXT){
                if (v[x] == v[y]) {
                    pc += 2;
                }
            } else {
                invalid_op = 1;
            }
            break;
        case OP3_SKIP_NEXT_NEQ:
            /* Skip next instruction if Vx != Vy */
            if (op0 == OP0_SKIP_NEXT){
                if (v[x] != v[y]) {
                    pc += 2;
                }
            } else {
                invalid_op = 1;
            }
            break;
        case OP3_LOAD_V:
            /* Vx = kk */
            v[x] = kk;
            break;
        case OP3_ADD_IM:
            /* Vx += kk */
            v[x] += kk;
            break;
        case OP3_ARITH:
            /* Arithmetic operations */
            switch(op0) {
            case OP0_ARITH_LOAD:
                /* Vx = Vy */
                v[x] = v[y];
                break;
            case OP0_ARITH_OR:
                /* Vx = Vx | Vy */
                v[x] |= v[y];
                break;
            case OP0_ARITH_AND:
                /* Vx = Vx & Vy */
                v[x] &= v[y];
                break;
            case OP0_ARITH_XOR:
                /* Vx = Vx ^ Vy */
                v[x] ^= v[y];
                break;
            case OP0_ARITH_ADD:
                /* Vx = Vx + Vy */
                v[x] += v[y];
                break;
            case OP0_ARITH_SUB:{
                /* Vx = Vx - Vy, Vf = not borrow */
                uint16_t vx = v[x];
                uint16_t vy = v[y];
                uint16_t diff = vx - vy;                                
                v[x] = diff;
                v[0xf] = (diff & 0x100) ? 1 : 0;
                break;
            }
            case OP0_ARITH_SHR:
                /* Vx = Vx >> Vy */
                v[x] >>= v[y];
                break;
            case OP0_ARITH_SUBN:{
                /* Vx = Vy - Vx, Vf = not borrow */
                uint16_t vx = v[x];
                uint16_t vy = v[y];
                uint16_t diff = vy - vx;                                
                v[x] = diff;
                v[0xf] = (diff & 0x100) ? 1 : 0;
                break;
            }
            case OP0_ARITH_SHL:
                /* Vx = Vx << Vy */
                v[x] <<= v[y];
                break;
            default:
                invalid_op = 1;
                break;
            }
            break;
        case OP3_LOAD_I:
            /* I = nnn */
            i = nnn;
            break;
        case OP3_RANDOM:
            /* Vx = (random byte) & kk */
            v[x] = RANDOM_BYTE & kk;
            break;
        case OP3_DRAW:{
            uint64_t collision = 0;
            /* Draw sprite from I at (Vx, Vy), vf = collision, see reference */
            for (int j = 0; j < n; j++){
                uint64_t row = 0;
                if (v[x] <= 56){
                    row = ((uint64_t)ram[i + j]) << (56 - v[x]);
                } else {
                    row = ((uint64_t)ram[i + j]) >> (v[x] - 56)
                        | ((uint64_t)ram[i + j]) << (120 - v[x]);
                }
                collision |= vram[(v[y] + j) % 32] & row;
                vram[(v[y] + j) % 32] ^= row;
            }
            v[0xf] = collision ? 1 : 0;
            break;
        }
        case OP3_KEYBOARD:
            /* Keyboard operations */
            switch(op10){
            case OP10_KEYBOARD_EQ:
                /* Skip next instruction if Vx pressed */
                if (keys[v[x] & 0xf] == 1){
                    pc += 2;
                }
                break;
            case OP10_KEYBOARD_NEQ: 
                /* Skip next instruction if Vx not pressed */
                if (keys[v[x] & 0xf] == 0){
                    pc += 2;
                }
                break;
            default:
                invalid_op = 1;
                break;
            }
            break;
        case OP3_SPECIAL:
            /* Special instructions, see reference for each */
            switch(op10){
            case OP10_SPECIAL_LOAD_DELAY:
                /* Vx = delay timer */
                v[x] = tmr;
                break;
            case OP10_SPECIAL_LOAD_KEYBOARD:
                /* Vx = keypress, blocks till something pressed */
                while (!keyflag);
                v[x] = lastkey;
                keyflag = 0;
                break;
            case OP10_SPECIAL_SET_DELAY:
                /* delay timer = Vx */
                tmr = v[x];
                break;
            case OP10_SPECIAL_SET_SOUND:
                /* sound timer = Vx */
                snd = v[x];
                break;
            case OP10_SPECIAL_ADD_TO_I:
                /* I += Vx */
                i += v[x];
                break;
            case OP10_SPECIAL_LOAD_FONT:
                /* I = location of sprite for hex digit Vx */
                i = (v[x] & 0xf) * 5;
                break;
            case OP10_SPECIAL_LOAD_BCD:
                /* BCD hundreds to units of Vx in I to I+2 */
                ram[i] = (uint8_t)(v[x] / 100) % 10;
                ram[i+1] = (uint8_t)(v[x] / 10) % 10;
                ram[i+2] = (uint8_t)(v[x]) % 10;
                break;
            case OP10_SPECIAL_LOAD_AT_I:
                /* Store V0 to Vx in I to I+x-1 */
                for (uint8_t j = 0; j < x; j++){
                    ram[i + j] = v[j];
                }
                break;
            case OP10_SPECIAL_SET_AT_I:
                /* Restore V0 to Vx from I to I+x-1 */
                for (uint8_t j = 0; j < x; j++){
                    v[j] = ram[i + j];
                }
                break;
            default:
                invalid_op = 1;
                break;
            }
            break;
        default:
            invalid_op = 1;
            break;
        }
        if (invalid_op) {
            /* Halt and dump core if invalid instruction received */
            // TODO
        }
        if (!freeze_pc){
            /* Increment PC */
            // TODO replace with continue
            pc += 2;
        }
    }
    return 0;
}
