#include "project.h"
#include "chip8.h"

/* Column Write Functions */
void (*col_wr[4])(uint8_t) = 
            {&Col_1_Write, &Col_2_Write, &Col_3_Write, &Col_4_Write};

/* Row Read Functions */
uint8_t (*row_rd[4])() = 
            {&Row_1_Read, &Row_2_Read, &Row_3_Read, &Row_4_Read};
            
uint8_t p2v[] = {0x1, 0x2, 0x3, 0xc,
                 0x4, 0x5, 0x6, 0xd,
                 0x7, 0x8, 0x9, 0xe,
                 0xa, 0x0, 0xb, 0xf};

uint8_t v2p[] = {0xd, 0x0, 0x1, 0x2,
                 0x4, 0x5, 0x6, 0x8,
                 0x9, 0xa, 0xc, 0xe,
                 0x3, 0x7, 0xb, 0xf};
            
uint8_t key_pressed(uint8_t vkey){
    /* Determine row and column numbers */
    uint8_t key = v2p[vkey];
    uint8_t row = (key & 0xc) >> 2;
    uint8_t col = (key & 0x3);
    
    uint8_t pressed = 0;
    
    /* Assert column line and read row line */
    (*col_wr[col])(1);
    CyDelay(1);
    pressed = (*row_rd[row])() ? 1 : 0;
    (*col_wr[col])(0);
    CyDelay(1);
    return pressed;
}

uint8_t wait_key(){
    while(1){
        for (uint8_t row = 0; row < 4; row++){
            for (uint8_t col = 0; col < 4; col++){
                uint8_t key = p2v[row*4 + col];
                if (key_pressed(key)){
                    return key;
                }
            }
        }
    }
}
