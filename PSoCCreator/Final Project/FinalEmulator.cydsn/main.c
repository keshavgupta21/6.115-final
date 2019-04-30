/* ============================================================================
 * CHIP8 Emulator
 * Written By Keshav Gupta, April 2019
 * Technical Reference: http://devernay.free.fr/hacks/chip8/C8TECH10.HTM
 * ============================================================================
*/

/*
TODO

Implement Keyboard
Implement VGA

Replace SRAM with EEPROM (or other funcitonality to change roms nicely
*/

#include "project.h"
#include "ram_init_data.h"

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

/* 
   CHIP8 Keyboard 
   keys stores the current state of each key
       (1 = down, 0 = up)
   lastkey stores the last key that was pressed
   leyflag = 1 if new data available in lastkey
       must be reset to 0 when data used
*/
uint8_t keys[16] = {0};
uint8_t lastkey = 0, keyflag = 0;

/* 
   CHIP8 Video
   Each row is one 64bit number
*/
uint64_t vram[32];

/* 
   The following code snippet (concerning VGA) is courtesy of
   the 6.115 Staff. It has been modified to suit this application.

   Video buffer gets its own SRAM to prevent bus contention.
   The user-facing buffer is in regular system RAM.
   This is set in the custom linker script (custom.ld).
*/
uint8_t vbuf[96][128] __attribute__ ((section(".vram")));

/* 
   Now we set up the DMA to copy pixels from vbuf to the screen.
   For timing, we rely on the fact that it takes the DMA exactly
   8 clocks to move each byte. After each line the DMA is updated
   to point to the next line in vbuf.
*/
uint8_t dma_chan, dma_td;
volatile int vga_update_flag = 1;

CY_ISR(newline) {
    uint16 line = 805 - VERT_ReadCounter();
    if (line % 8 == 0) {
        if (line < 768) {
            CY_SET_REG16(CY_DMA_TDMEM_STRUCT_PTR[dma_td].TD1,
                         LO16((uint32) vbuf[line / 8]));
        } else if (line == 768 && vga_update_flag) {
            /* Refresh buffer during vsync */
            CyDmaChDisable(dma_chan);
            if (vga_update_flag) {
                uint8_t pixel;
                uint64_t temprow;
                for (uint8_t x = 0; x < 64; x++){
                    for (uint8_t y = 0; y < 48; y++){
                        if (y < 8 || y >= 40) {
                            pixel = 0;
                        } else {
                            temprow = vram[y - 8] & (((uint64_t)1) << (63-x));
                            pixel = (temprow) ? 1 : 0;
                        }
                        vbuf[2*y][2*x] = pixel;
                        vbuf[2*y+1][2*x] = pixel;
                        vbuf[2*y][2*x+1] = pixel;
                        vbuf[2*y+1][2*x+1] = pixel;
                    }
                }
                vga_update_flag = 0;
            }
            CyDmaChEnable(dma_chan, 1);
        }
    }
}

/* 
   CHIP8 Timers 
   Decrement each at 60Hz if not zero
*/
uint8_t snd = 0, tmr = 0;

CY_ISR(isr_tmr_handler){
    snd = (snd == 0) ? 0 : snd - 1;
    tmr = (tmr == 0) ? 0 : tmr - 1;
    if (snd) {
        pin_snd_Write(1);
    } else {
        pin_snd_Write(0);
    }
}

int main(void){
    /* Start the interrupts */
    isr_timer_StartEx(isr_tmr_handler);
    
    /* Initialize the DMA */
    dma_td = CyDmaTdAllocate();
    dma_chan = DMA_DmaInitialize(1, 0, HI16(CYDEV_SRAM_BASE),
                                 HI16(CYDEV_PERIPH_BASE));
    CyDmaTdSetConfiguration(dma_td, 128, dma_td, 
                            DMA__TD_TERMOUT_EN | TD_INC_SRC_ADR);
    CyDmaTdSetAddress(dma_td, 0, LO16((uint32) PIXEL_Control_PTR));
    CyDmaChSetInitialTd(dma_chan, dma_td);
    CyDmaChEnable(dma_chan, 1);
    
    /* Start all of the timing counters */
    HORIZ_Start();
    VERT_Start();
    HSYNC_Start();
    VSYNC_Start();
    NEWLINE_StartEx(newline);
    
    /* Enable global interrupts */
    CyGlobalIntEnable;
    
    /* CHIP8 RAM*/
    uint8_t ram[4096] = RAM_INITIAL_DATA;
    
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
    
    /* Initialize Random Generator */
    random_Enable();
    random_Init();
    random_Start();
    
    /* Turn on LED to indicate execution */
    pin_led_Write(1);
    
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
                    sp = (sp == 0x0) ? 0x0 : sp - 1;
                    pc = stack[sp];
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
            if (v[x] == kk) {
                pc += 2;
            }
            break;
        case OP3_SKIP_NEXT_NEQ_IM:
            /* Skip next instruction if Vx != kk */
            if (v[x] != kk) {
                pc += 2;
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
                v[0xf] = (diff & 0x100) ? 0 : 1;
                break;
            }
            case OP0_ARITH_SHR:
                /* Vx = Vx >> Vy */
                v[0xf] = (v[x] & 0x01) ? 1 : 0;
                v[x] >>= 1;
                break;
            case OP0_ARITH_SUBN:{
                /* Vx = Vy - Vx, Vf = not borrow */
                uint16_t vx = v[x];
                uint16_t vy = v[y];
                uint16_t diff = vy - vx;                                
                v[x] = diff;
                v[0xf] = (diff & 0x100) ? 0 : 1;
                break;
            }
            case OP0_ARITH_SHL:
                /* Vx = Vx << 1, Vf = carry */
                v[0xf] = (v[x] & 0x80) ? 1 : 0;
                v[x] <<= 1;
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
            v[x] = random_Read() & kk;
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
            vga_update_flag = 1;
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
                /* Store V0 to Vx in I to I+x */
                for (uint8_t j = 0; j <= x; j++){
                    ram[i + j] = v[j];
                }
                break;
            case OP10_SPECIAL_SET_AT_I:
                /* Restore V0 to Vx from I to I+x */
                for (uint8_t j = 0; j <= x; j++){
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
            /* Turn off LED if invalid instruction received */
            while(1){
                pin_led_Write(0);
            }
        }
        if (!freeze_pc){
            /* Increment PC */
            pc += 2;
        }
    }
    return 0;
}