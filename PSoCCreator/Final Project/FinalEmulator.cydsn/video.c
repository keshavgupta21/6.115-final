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
extern volatile uint8_t frame_update;
extern uint64_t vram[32];
extern u8g2_t u8g2;

void vga_initialize(){
    dma_td = CyDmaTdAllocate();
    //dma_chan = DMA_DmaInitialize(1, 0, HI16(CYDEV_SRAM_BASE),
    dma_chan = DMA_DmaInitialize(1, 0, HI16((uint32)vbuf), HI16(CYDEV_PERIPH_BASE));
    CyDmaTdSetConfiguration(dma_td, 128, dma_td, DMA__TD_TERMOUT_EN | TD_INC_SRC_ADR);
    CyDmaTdSetAddress(dma_td, LO16((uint32)vbuf), LO16((uint32) PIXEL_Control_PTR));
    CyDmaChSetInitialTd(dma_chan, dma_td);
    CyDmaChEnable(dma_chan, 1);
}

CY_ISR(newline_handler) {
    uint16 line = 805 - VERT_ReadCounter();
    if (line % 8 == 0) {
        if (line < 768) {
            CY_SET_REG16(CY_DMA_TDMEM_STRUCT_PTR[dma_td].TD1,
                         LO16((uint32) vbuf[line / 8]));
        } else if (line == 768 && frame_update) {
            CyDmaChDisable(dma_chan);
            if (frame_update) {
                /* Update the OLED and VGA frame buffers */
                for(uint8_t page = 0; page < 12; page++){
                    for (uint8_t col = 0; col < 128; col += 2){
                        if (page >= 2 && page < 10){
                            uint8_t pcol = 0x00;
                            for (uint8_t row = 0; row < 8; row += 2){
                                /* See project report for explanation */
                                uint8_t pix = vram[(page-2)*4 + row/2] & (((uint64_t)1) << (63 - col/2)) ? 3 : 0;
                                uint8_t y = page * 8 + row;
                                uint8_t x = col;
                                vbuf[y][x] = pix;
                                vbuf[y+1][x] = pix;
                                vbuf[y][x+1] = pix;
                                vbuf[y+1][x+1] = pix;
                                pcol |= (pix << row);
                            }
                            /* Access the internal frame buffer in the struct */
                            /* Hacky, but works */
                            u8g2.tile_buf_ptr[(page-2)*128 + col] = pcol;
                            u8g2.tile_buf_ptr[(page-2)*128 + col + 1] = pcol;
                        } else {
                            for (uint8_t row = 0; row < 8; row += 2){
                                uint8_t y = page * 8 + row;
                                uint8_t x = col;
                                vbuf[y][x] = 0;
                                vbuf[y+1][x] = 0;
                                vbuf[y][x+1] = 0;
                                vbuf[y+1][x+1] = 0;
                            }
                        }
                    }
                }
                u8g2_UpdateDisplay(&u8g2);
                frame_update = 0;
            }
            CyDmaChEnable(dma_chan, 1);
        }
    }
}
