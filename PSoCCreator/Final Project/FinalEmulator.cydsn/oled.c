#include "project.h"
#include "chip8.h"

uint8_t gp_dl_cb(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t hw_spi_cb(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);

extern u8g2_t u8g2;
extern uint64_t vram_oled[32];
extern uint8_t oled_update_flag;

#define HW_SPI

void oled_initialize(u8g2_t *u8g2){
    /* Initialize the struct */
    u8x8_msg_cb comm_cb;
    #ifdef SW_SPI
        comm_cb = u8x8_byte_4wire_sw_spi;
    #endif
    #ifdef HW_SPI
        comm_cb = hw_spi_cb;
    #endif
    u8g2_Setup_sh1106_128x64_noname_f(u8g2, U8G2_R0, comm_cb, gp_dl_cb);
    
    /* Send init sequence to the display */
    u8g2_InitDisplay(u8g2);
    
    /* Wake up display */
    u8g2_SetPowerSave(u8g2, 0);
}

void oled_update_frame(){
    for(uint8_t page = 0; page < 8; page++){
        for (uint8_t col = 0; col < 128; col += 2){
            uint8_t pcol = 0x00;
            for (uint8_t row = 0; row < 8; row += 2){
                uint8_t pix = vram_oled[page*4 + row/2] & (((uint64_t)1) << (63 - col/2)) ? 3 : 0;
                pcol |= (pix << row);
            }
            u8g2.tile_buf_ptr[page*128 + col] = pcol;
            u8g2.tile_buf_ptr[page*128 + col + 1] = pcol;
        }
    }
    u8g2_UpdateDisplay(&u8g2);
}

uint8_t oled_divide_count = 0;

CY_ISR(isr_oled_handler){
    oled_divide_count = (oled_divide_count == 0) ? 100 : (oled_divide_count - 1);
    if (oled_divide_count == 0 && oled_update_flag){
        oled_update_frame();
        oled_update_flag = 0;
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
    case U8X8_MSG_GPIO_DC:{
        /* Set the Data/Command bit */
        pin_oled_dc_Write(arg_int ? 1 : 0);
        break;
    }
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