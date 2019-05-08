/* ============================================================================
 * CHIP8 Emulator
 * Written By Keshav Gupta, April 2019
 * Technical Reference: http://devernay.free.fr/hacks/chip8/C8TECH10.HTM
 * ============================================================================
*/

/*
Get help from Staff on:
    1. VGA position debug
    2. UART ROM debug
    
TODO on your own:
    2. Try other roms
    3. Test maze i.e. random number generator
    4. Start writing report (make case for general handheld device framework)
    5. 3D print case!!!! (if feeling adventurous)
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
   Initialize to all black
*/
uint64_t vram[32] = {};

/* The struct that holds info for the OLED */
u8g2_t u8g2;

/* 
   The code snippets concerning VGA derives heavily from 
   the 6.115 Staff implementation. All credit goes to them.

*/
uint8_t dma_chan, dma_td;
volatile uint8_t frame_update = 1;

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
        
    /* Start Sound and Timer */
    pwm_snd_Start();
    isr_timer_StartEx(isr_tmr_handler);
    
    /* Initialize Random Generator */
    random_Start();
    
    /* Enable global interrupts */
    CyGlobalIntEnable;
    
    /* Start EEPROM Memory */
    eeprom_Start();
    if (pin_prog_Read()){
        /* If pin_prog is high, go into program mode */
        usb_uart_echo();
    } else {
        /* Load ROM from EEPROM */
        for (uint16_t i = 0; i < (0x1000 - 0x200); i++){
            ram[0x200 + i] = eeprom_ReadByte(i);
        }
        
        /* Turn on LED to indicate execution */
        // TODO
        pin_led_Write(1);

        /* Start execution */
        execute();
        
        /* Normally we would never get here */
        /* Turn off LED if invalid instruction received */
        pin_led_Write(0);
    }
    return 0;
}