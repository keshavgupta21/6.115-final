/* ============================================================================
 * CHIP8 Emulator
 * Written By Keshav Gupta, April 2019
 * Technical Reference: http://devernay.free.fr/hacks/chip8/C8TECH10.HTM
 * ============================================================================
*/

/*
TODO sound, keyboard debounce, UART ROM transfer

TODO debug vga, space invaders
*/

#include "project.h"
#include "ram_init_data.h"
#include "chip8.h"

/* 
   CHIP8 Timers 
   Decrement each at 60Hz if not zero
*/
uint8_t snd = 0, tmr = 0;

/* 
   CHIP8 Video
   Each row is one 64bit number
*/
uint64_t vram_vga[32];
uint64_t vram_oled[32];

/* The struct that holds info for the OLED */
u8g2_t u8g2;
volatile uint8_t oled_update_flag = 1;

/* 
   The following code snippet (concerning VGA) is courtesy of
   the 6.115 Staff. It has been modified to suit this application.

*/
uint8_t dma_chan, dma_td;
volatile uint8_t vga_update_flag = 1;

int main(void){
    /* Initialize the DMA */
    dma_td = CyDmaTdAllocate();
    dma_chan = DMA_DmaInitialize(1, 0, HI16(CYDEV_SRAM_BASE),
                                 HI16(CYDEV_PERIPH_BASE));
    CyDmaTdSetConfiguration(dma_td, 128, dma_td, 
                            DMA__TD_TERMOUT_EN | TD_INC_SRC_ADR);
    CyDmaTdSetAddress(dma_td, 0, LO16((uint32) PIXEL_Control_PTR));
    CyDmaChSetInitialTd(dma_chan, dma_td);
    CyDmaChEnable(dma_chan, 1);
    
    /* Start the VGA interrupt */
    NEWLINE_StartEx(newline_handler);
    
    /* Start all of the timing counters */
    HORIZ_Start();
    VERT_Start();
    HSYNC_Start();
    VSYNC_Start();
    
    /* Start the OLED and its refresh interrupt */
    oled_initialize(&u8g2);
    isr_oled_StartEx(isr_oled_handler);
    
    /* Start the Timer interrupt */
    isr_timer_StartEx(isr_tmr_handler);
    
    /* Enable global interrupts */
    CyGlobalIntEnable;
    
    /* Initialize Random Generator */
    // TODO make sure random still works
    random_Start();
    
    /* CHIP8 RAM */
    uint8_t ram[4096] = RAM_INITIAL_DATA;
        
    /* Turn on LED to indicate execution */
    pin_led_Write(1);

    /* Start execution */
    execute(ram);
    
    /* Normally we would never get here */
    /* Turn off LED if invalid instruction received */
    pin_led_Write(0);
    return 0;
}