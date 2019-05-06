/* ============================================================================
 * CHIP8 Emulator
 * Written By Keshav Gupta, April 2019
 * Technical Reference: http://devernay.free.fr/hacks/chip8/C8TECH10.HTM
 * ============================================================================
*/

/*
TODO sound, UART ROM transfer

TODO position vga, space invaders

TODO update oled buffer on NEWLINE interrupt as well, maybe try writing an empty buffer to the OLED to ensure its not because of the interference from SPI clock
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
uint64_t vram[32];

/* The struct that holds info for the OLED */
u8g2_t u8g2;

/* 
   The following code snippet (concerning VGA) is courtesy of
   the 6.115 Staff. It has been modified to suit this application.

*/
uint8_t dma_chan, dma_td;
volatile uint8_t vga_update_flag = 1;

/* CHIP8 RAM */
uint8_t ram[4096] = CH8_FONT_DATA;

int main(void){
    /* Initialize the DMA for VGA */
    vga_initialize();
    
    /* Initialize the OLED */
    oled_initialize(&u8g2);

    /* Start the frame refresh interrupt */
    NEWLINE_StartEx(newline_handler);
    
    /* Start all of the timing counters */
    HORIZ_Start();
    VERT_Start();
    HSYNC_Start();
    VSYNC_Start();
        
    /* Start the Timer interrupt */
    isr_timer_StartEx(isr_tmr_handler);
    
    /* Enable global interrupts */
    CyGlobalIntEnable;
    
    /* Initialize Random Generator */
    // TODO make sure random still works
    random_Start();
        
    // TODO load the rom from eeprom
    eeprom_Start();
    //usb_uart_echo();
    for (uint16_t i = 0; i < (0x1000 - 0x200); i++){
        ram[0x200 + i] = eeprom_ReadByte(i);
    }
    
    /* Turn on LED to indicate execution */
    pin_led_Write(1);

    /* Start execution */
    execute();
    
    /* Normally we would never get here */
    /* Turn off LED if invalid instruction received */
    pin_led_Write(0);
    return 0;
}