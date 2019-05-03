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
*/

#include "project.h"
#include "ram_init_data.h"
#include "chip8.h"

/* 
   CHIP8 Video
   Each row is one 64bit number
*/
uint64_t vram[32];

/* The struct that holds info for the OLED */
u8g2_t u8g2;

/* OLED Refresh Interrupt Handler */
CY_ISR(_isr_oled_handler){
    isr_oled_handler(&u8g2, vram);
}

/* 
   CHIP8 Timers 
   Decrement each at 60Hz if not zero
*/
uint8_t snd = 0, tmr = 0;

CY_ISR(_isr_tmr_handler){
    isr_tmr_handler(&snd, &tmr);
};

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
    NEWLINE_StartEx(_newline_handler);
    
    /* Start all of the timing counters */
    HORIZ_Start();
    VERT_Start();
    HSYNC_Start();
    VSYNC_Start();
    
    /* Start the OLED and its refresh interrupt */
    oled_initialize(&u8g2);
    isr_oled_StartEx(_isr_oled_handler);
    
    /* Start the Timer interrupt */
    isr_timer_StartEx(_isr_tmr_handler);
    
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
    execute(ram, vram, &snd, &tmr, &vga_update_flag);
    
    /* Normally we would never get here */
    /* Turn off LED if invalid instruction received */
    pin_led_Write(0);
    return 0;
}