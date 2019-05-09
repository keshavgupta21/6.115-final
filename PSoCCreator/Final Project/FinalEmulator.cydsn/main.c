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
uint64_t vram[32] = {
    0x0,
    0x3fb0befe0000,
    0x208118820000,
    0x2ea78aba0000,
    0x2eb57cba0000,
    0x2ebd44ba0000,
    0x20931e820000,
    0x3faaaafe0000,
    0x147e000000,
    0x3ca19d3a0000,
    0xb50a6e20000,
    0x3ea12aec0000,
    0x1647abe20000,
    0x1d95f4180000,
    0x372cb6ce0000,
    0x2b9aa76e0000,
    0x3622e2e40000,
    0x2e8c0b340000,
    0x1a72cb5c0000,
    0x24be60480000,
    0xf02b7080000,
    0x14fedff80000,
    0x3a0a3e0000,
    0x3f9f86b40000,
    0x2091ca340000,
    0x2e8af3ee0000,
    0x2ea667320000,
    0x2ead014a0000,
    0x20a4ae540000,
    0x3fa0e6140000,
    0x0,
    0x0
};

/* The struct that holds info for the OLED */
u8g2_t u8g2;

/* 
   The code snippets concerning VGA derives heavily from 
   the 6.115 Staff implementation. All credit goes to them.

*/
uint8_t dma_chan, dma_td;
volatile uint8_t frame_update = 1;

/* CHIP8 RAM */
uint8_t ram[0x1000];

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
        for (uint16_t i = 0; i < 0x1000; i++){
            ram[i] = eeprom_ReadByte(i);
        }
        
        /* Turn on LED to indicate execution */
        pin_led_Write(1);

        /* Start execution */
        execute();
        
        /* Normally we would never get here */
        /* Turn off LED if invalid instruction received */
        pin_led_Write(0);
    }
    return 0;
}