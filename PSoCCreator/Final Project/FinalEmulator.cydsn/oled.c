#include "project.h"
#include "chip8.h"
#include "u8g2.h"

uint8_t gp_dl_cb(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t hw_spi_cb(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);

#define HW_SPI

void oled_initialize(){
    /* The structure that will hold the OLED info */
    u8g2_t u8g2;
    
    /* Initialize the struct */
    u8x8_msg_cb comm_cb;
    #ifdef SW_SPI
        comm_cb = u8x8_byte_4wire_sw_spi;
    #endif
    #ifdef HW_SPI
        comm_cb = hw_spi_cb;
    #endif
    u8g2_Setup_sh1106_128x64_noname_f(&u8g2, U8G2_R0, comm_cb, gp_dl_cb);
    
    /* Send init sequence to the display */
    u8g2_InitDisplay(&u8g2);
    
    /* Wake up display */
    u8g2_SetPowerSave(&u8g2, 0);
    
    // TODO
    uint8_t k = 0;
    while(1){
        u8g2_DrawBox(&u8g2, 0, 0, 127, 63);
        u8g2_UpdateDisplay(&u8g2);
        CyDelay(1000);
        k++;
    }
}

#ifdef HW_SPI
uint8_t hw_spi_cb(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
    switch(msg) {
    case U8X8_MSG_BYTE_SEND:{
        /* Send a sequence of bytes to the OLED */
        uint8_t *data = (uint8_t*) arg_ptr;
        while(arg_int > 0) {
            spi_oled_WriteByte(*data);
            data++;
            arg_int--;
        }  
        break;
    }
    case U8X8_MSG_BYTE_INIT:
        /* Initialize the SPI for the OLED */
        spi_oled_Start();
        break;
    case U8X8_MSG_BYTE_SET_DC:
        /* Set the Data/Command bit */
        u8x8_gpio_SetDC(u8x8, arg_int);
        break;
    case U8X8_MSG_BYTE_START_TRANSFER:
        /* Prepare for byte sequence transfer */
        // TODO Check if something needs to be done here
        break;
    case U8X8_MSG_BYTE_END_TRANSFER:
        /* Conclude byte sequence transfer */
        break;
    default:
        return 0;
    }  
    return 1;
}
#endif

uint8_t gp_dl_cb(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr){
    switch(msg){
    case U8X8_MSG_DELAY_NANO:			
        /* Delay arg_int * 1 nano second */
        CyDelayCycles((uint32_t)((BCLK__BUS_CLK__HZ * (double)arg_int)/1e9));
        break;    
    case U8X8_MSG_DELAY_100NANO:
        /* Delay arg_int * 1 nano second */
        CyDelayCycles((uint32_t)((BCLK__BUS_CLK__HZ * (double)arg_int)/1e7));
        break;
    case U8X8_MSG_DELAY_10MICRO:
        /* Delay arg_int * 10 micro seconds */
        CyDelayUs(10*arg_int);
        break;
    case U8X8_MSG_DELAY_MILLI:
        /* Delay arg_int * 1 milli seconds */
        CyDelay(arg_int);
        break; 
    #ifdef SW_SPI
    case U8X8_MSG_GPIO_SPI_CLOCK:
        /* Set the CLK bit */
        pin_oled_clk_Write(arg_int ? 1 : 0);
        break;	
    case U8X8_MSG_GPIO_SPI_DATA:
        /* Set the MOSI bit */
        pin_oled_mosi_Write(arg_int ? 1 : 0);
        break;
    case U8X8_MSG_GPIO_CS:
        /* Set the Chip Select bit */
        pin_oled_cs_Write(arg_int ? 1 : 0);
        break;
    #endif
    case U8X8_MSG_GPIO_DC:
        /* Set the Data/Command bit */
        pin_oled_dc_Write(arg_int ? 1 : 0);
        break;
    case U8X8_MSG_GPIO_RESET:
        /* Set the value for the Reset pin */
        pin_oled_rst_Write(arg_int ? 1 : 0);
        break;
    default:
        u8x8_SetGPIOResult(u8x8, 0);			// default return value
        break;
    }
    return 1;
}