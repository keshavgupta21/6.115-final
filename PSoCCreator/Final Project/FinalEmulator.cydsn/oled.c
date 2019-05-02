#include "project.h"
#include "chip8.h"
#include "u8g2.h"

uint8_t gp_dl_cb(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t hw_spi_cb(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);

void oled_initialize(){
    /* The structure that will hold the OLED info */
    u8g2_t u8g2;
    
    /* Initialize the struct */
    u8g2_Setup_sh1106_128x64_noname_f(&u8g2, U8G2_R0, u8x8_byte_4wire_sw_spi, gp_dl_cb);
    
    /* Send init sequence to the display */
    u8g2_InitDisplay(&u8g2);
    
    /* Wake up display */
    u8g2_SetPowerSave(&u8g2, 0);
    
    u8g2_UpdateDisplay(&u8g2);
    uint8_t k =0 ;
    while(1){
        for (uint8_t i = 0; i < 128; i++){
            for (uint8_t j = 0; j < 8; j++){
                u8g2.tile_buf_ptr[j*128 + i] = k;
            }
        }
        u8g2_UpdateDisplay(&u8g2);
        CyDelay(10);
        k++;
    }
}

uint8_t hw_spi_cb(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
    switch(msg) {
    case U8X8_MSG_BYTE_SEND:{
        uint8_t *data = (uint8_t*) arg_ptr;
        while(arg_int > 0) {
            //spi_oled_WriteByte(*data);
            data++;
            arg_int--;
        }  
        break;
    }
    case U8X8_MSG_BYTE_INIT:
        break;
    case U8X8_MSG_BYTE_SET_DC:
        u8x8_gpio_SetDC(u8x8, arg_int);
        break;
    case U8X8_MSG_BYTE_START_TRANSFER:
        break;
    case U8X8_MSG_BYTE_END_TRANSFER:
        break;
    default:
        return 0;
    }  
    return 1;
}

uint8_t gp_dl_cb(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr){
    switch(msg){
    case U8X8_MSG_DELAY_NANO:			// delay arg_int * 1 nano second
        CyDelayCycles((uint32_t)((BCLK__BUS_CLK__HZ * (double)arg_int)/1e9));
        break;    
    case U8X8_MSG_DELAY_100NANO:		// delay arg_int * 100 nano seconds
        CyDelayCycles((uint32_t)((BCLK__BUS_CLK__HZ * (double)arg_int)/1e7));
        break;
    case U8X8_MSG_DELAY_10MICRO:		// delay arg_int * 10 micro seconds
        CyDelayUs(10*arg_int);
        break;
    case U8X8_MSG_DELAY_MILLI:			// delay arg_int * 1 milli second
        CyDelay(arg_int);
        break;    
    case U8X8_MSG_GPIO_SPI_CLOCK:// D0 or SPI clock pin: Output level in arg_int
        pin_oled_clk_Write(arg_int ? 1 : 0);
        break;	
    case U8X8_MSG_GPIO_SPI_DATA:// D1 or SPI data pin: Output level in arg_int
        pin_oled_mosi_Write(arg_int ? 1 : 0);
        break;
    case U8X8_MSG_GPIO_CS:				// CS (chip select) pin: Output level in arg_int
        pin_oled_cs_Write(arg_int ? 1 : 0);
        break;
    case U8X8_MSG_GPIO_DC:				// DC (data/cmd, A0, register select) pin: Output level in arg_int
        pin_oled_dc_Write(arg_int ? 1 : 0);
        break;
    case U8X8_MSG_GPIO_RESET:			// Reset pin: Output level in arg_int
        pin_oled_rst_Write(arg_int ? 1 : 0);
        break;
    default:
        u8x8_SetGPIOResult(u8x8, 0);			// default return value
        break;
    }
    return 1;
}