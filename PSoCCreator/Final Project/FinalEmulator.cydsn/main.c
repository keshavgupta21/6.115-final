/* ============================================================================
 * CHIP8 Emulator
 * Written By Keshav Gupta, April 2019
 * Technical Reference: http://devernay.free.fr/hacks/chip8/C8TECH10.HTM
 * ============================================================================
*/

/*
TODO Check last column on vga is always black

TODO Space Invaders debug

TODO sound

TODO Correct video position

Replace SRAM with EEPROM (or other funcitonality to change roms nicely)

Implement 1.3" OLED
*/

#include "project.h"
#include "ram_init_data.h"
#include "chip8.h"

/* 
   CHIP8 Timers 
   Decrement each at 60Hz if not zero
*/
uint8_t snd = 0, tmr = 0;

CY_ISR(_isr_tmr_handler){
    isr_tmr_handler(&snd, &tmr);
};

/* 
   CHIP8 Video
   Each row is one 64bit number
*/
uint64_t vram[32];

/* 
   The following code snippet (concerning VGA) is courtesy of
   the 6.115 Staff. It has been modified to suit this application.

*/
uint8_t dma_chan, dma_td;
volatile uint8_t vga_update_flag = 1;

CY_ISR(_newline_handler){
    newline_handler(&dma_chan, &dma_td,
      &vga_update_flag, vram);
};

int main(void){
    /* Start the interrupts */
    isr_timer_StartEx(_isr_tmr_handler);
    NEWLINE_StartEx(_newline_handler);
    
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
    
    /* Initialize Random Generator */
    random_Enable();
    random_Init();
    random_Start();
    
    /* Start the OLED */
    //spi_oled_Start();
    oled_initialize();
    
    /* Enable global interrupts */
    CyGlobalIntEnable;
    
    /* CHIP8 RAM */
    uint8_t ram[4096] = RAM_INITIAL_DATA;
        
    /* Turn on LED to indicate execution */
    pin_led_Write(1);

    /* Start execution */
    execute(ram, vram, &snd, &tmr, &vga_update_flag);
    
    /* Normally we would never get here */
    /* Turn off LED if invalid instruction received */
    pin_led_Write(0);
    return 0;
}