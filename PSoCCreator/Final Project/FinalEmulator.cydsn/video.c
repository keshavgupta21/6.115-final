#include "project.h"
#include "chip8.h"

/*
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

extern uint8_t dma_chan;
extern uint8_t dma_td;
extern volatile uint8_t vga_update_flag;
extern uint64_t vram_vga[32];

CY_ISR(newline_handler) {
    uint16 line = 805 - VERT_ReadCounter();
    if (line % 8 == 0) {
        if (line < 768) {
            CY_SET_REG16(CY_DMA_TDMEM_STRUCT_PTR[dma_td].TD1,
                         LO16((uint32) vbuf[line / 8]));
        } else if (line == 768 && vga_update_flag) {
            /* Refresh buffer during vsync */
            CyDmaChDisable(dma_chan);
            if (vga_update_flag) {
                for (uint8_t y = 0; y < 96; y += 2){
                    for (uint8_t x = 0; x < 128; x += 2){
                        uint8_t pix;
                        if (y < 16 || y >= 80){
                            pix = 0;
                        } else {
                            uint64_t temprow = vram_vga[(y-16)/2];
                            temprow &= (uint64_t)1 << (63-x/2);
                            pix = temprow ? 1 : 0;
                        }
                        vbuf[y][x] = pix;
                        vbuf[y+1][x] = pix;
                        vbuf[y][x+1] = pix;
                        vbuf[y+1][x+1] = pix;
                    }
                }
                vga_update_flag = 0;
            }
            CyDmaChEnable(dma_chan, 1);
        }
    }
}
