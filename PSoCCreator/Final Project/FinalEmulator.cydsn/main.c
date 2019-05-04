/* ============================================================================
 * CHIP8 Emulator
 * Written By Keshav Gupta, April 2019
 * Technical Reference: http://devernay.free.fr/hacks/chip8/C8TECH10.HTM
 * ============================================================================
*/

/*
TODO sound, UART ROM transfer

TODO debug vga, space invaders
*/

#include "project.h"
#include "font.h"
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

/* CHIP8 RAM */
uint8_t ram[4096] = CH8_FONT_DATA;

void load_ram_eeprom();

int main(void){
    // TODO move this to video.c file
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
    // TODO move following line to oled_initilize
    isr_oled_StartEx(isr_oled_handler);
    
    /* Start the Timer interrupt */
    isr_timer_StartEx(isr_tmr_handler);
    
    /* Enable global interrupts */
    CyGlobalIntEnable;
    
    /* Initialize Random Generator */
    // TODO make sure random still works
    random_Start();
        
    /* Turn on LED to indicate execution */
   
    // TODO
    eeprom_Start();
    //usb_uart_echo();
    load_ram_eeprom();
    pin_led_Write(1);

    /* Start execution */
    execute();
    
    /* Normally we would never get here */
    /* Turn off LED if invalid instruction received */
    pin_led_Write(0);
    return 0;
}

void load_ram_eeprom(){
    uint16_t len = eeprom_ReadByte(0);
    len <<= 8;
    len |= eeprom_ReadByte(1);
    for (uint16_t i = 0; i < len; i++){
        ram[0x200 + i] = eeprom_ReadByte(i + 2);
    }
}