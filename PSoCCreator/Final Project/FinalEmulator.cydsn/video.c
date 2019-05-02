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

void newline_handler(uint8_t* dma_chan, uint8_t* dma_td, 
    volatile uint8_t* vga_update_flag, uint64_t vram[]) {
    uint16 line = 805 - VERT_ReadCounter();
    if (line % 8 == 0) {
        if (line < 768) {
            CY_SET_REG16(CY_DMA_TDMEM_STRUCT_PTR[*dma_td].TD1,
                         LO16((uint32) vbuf[line / 8]));
        } else if (line == 768 && *vga_update_flag) {
            /* Refresh buffer during vsync */
            CyDmaChDisable(*dma_chan);
            if (*vga_update_flag) {
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
                *vga_update_flag = 0;
            }
            CyDmaChEnable(*dma_chan, 1);
        }
    }
}
