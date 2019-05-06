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
extern uint64_t vram[32];
extern u8g2_t u8g2;

void vga_initialize(){
    dma_td = CyDmaTdAllocate();
    dma_chan = DMA_DmaInitialize(1, 0, HI16(CYDEV_SRAM_BASE),
                                 HI16(CYDEV_PERIPH_BASE));
    CyDmaTdSetConfiguration(dma_td, 128, dma_td, 
                            DMA__TD_TERMOUT_EN | TD_INC_SRC_ADR);
    CyDmaTdSetAddress(dma_td, 0, LO16((uint32) PIXEL_Control_PTR));
    CyDmaChSetInitialTd(dma_chan, dma_td);
    CyDmaChEnable(dma_chan, 1);
}


CY_ISR(newline_handler) {
    uint16 line = 805 - VERT_ReadCounter();
    if (line % 8 == 0) {
        if (line < 768) {
            CY_SET_REG16(CY_DMA_TDMEM_STRUCT_PTR[dma_td].TD1,
                         LO16((uint32) vbuf[line / 8]));
        } else if (line == 768 && vga_update_flag) {
            /* Refresh buffer during vsync */
            if (vga_update_flag) {
                CyDmaChDisable(dma_chan);
                for (uint8_t y = 0; y < 96; y += 2){
                    for (uint8_t x = 0; x < 128; x += 2){
                        uint8_t pix;
                        if (y < 16 || y >= 80){
                            pix = 0;
                        } else {
                            uint64_t temprow = vram[(y-16)/2];
                            temprow &= (uint64_t)1 << (63-x/2);
                            pix = temprow ? 1 : 0;
                        }
                        vbuf[y][x] = pix;
                        vbuf[y+1][x] = pix;
                        vbuf[y][x+1] = pix;
                        vbuf[y+1][x+1] = pix;                        
                    }
                }
                CyDmaChEnable(dma_chan, 1);
                for(uint8_t page = 0; page < 8; page++){
                    for (uint8_t col = 0; col < 128; col += 2){
                        uint8_t pcol = 0x00;
                        for (uint8_t row = 0; row < 8; row += 2){
                            uint8_t pix = vram[page*4 + row/2] & (((uint64_t)1) << (63 - col/2)) ? 3 : 0;
                            pcol |= (pix << row);
                        }
                        u8g2.tile_buf_ptr[page*128 + col] = pcol;
                        u8g2.tile_buf_ptr[page*128 + col + 1] = pcol;
                    }
                }
                u8g2_UpdateDisplay(&u8g2);
                vga_update_flag = 0;
            }
        }
    }
}
