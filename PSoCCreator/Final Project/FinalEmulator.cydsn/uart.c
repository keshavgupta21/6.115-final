#include "project.h"
#include "chip8.h"

void usb_uart_echo()
{
    uint16 count;
    uint8 buffer[64];

    /* Start USBFS operation with 5-V operation. */
    usb_uart_Start(0, usb_uart_5V_OPERATION);
    
    /* 
        Receiver FSM.
        States:
            
    */
    uint8_t state = 0;
    uint16_t counter = 0, length = 0;    
    
    for(;;){
        /* Host can send double SET_INTERFACE request. */
        if (usb_uart_IsConfigurationChanged() != 0){
            /* Initialize IN endpoints when device is configured. */
            if (usb_uart_GetConfiguration() != 0){
                /* Enumeration is done, enable OUT endpoint to receive data 
                 * from host. */
                usb_uart_CDC_Init();
            }
        }
        /* Service USB CDC when device is configured. */
        if (usb_uart_GetConfiguration() != 0){
            /* Check for input data from host. */
            if (usb_uart_DataIsReady() != 0){
                /* Read received data and re-enable OUT endpoint. */
                count = usb_uart_GetAll(buffer);
                if (count != 0){
                    for (uint8_t i = 0; i < count; i++){
                        switch(state){
                        case 0:
                            if (buffer[i] == 0x00){
                                state = 1;
                            }
                            break;
                        case 1:
                            if (buffer[i] == 0x01){
                                state = 2;
                            }
                            break;
                        case 2:
                            length = (uint16_t)buffer[i] << 8;
                            state = 3;
                            break;
                        case 3:
                            length |= (uint16_t)buffer[i];
                            state = 4;
                            break;
                        case 4:
                            if (counter < length){
                                eeprom_WriteByte(buffer[i], counter++);
                            } else {
                                state = 5;
                            }
                            break;
                        case 5:
                            while (usb_uart_CDCIsReady() == 0){}
                            uint8_t tbuf[] = {((uint16_t)(counter&0xff00) >> 8), counter & 0xff};
                            usb_uart_PutData(tbuf, 2);
                            state = 6;
                            break;
                        case 6:
                            if (buffer[i] == 0x04){
                                /* All good, go ahead */
                                while (1){
                                    pin_led_Write(1);
                                }
                            } else {
                                while (1){
                                    pin_led_Write(1);
                                    CyDelay(100);
                                    pin_led_Write(0);
                                    CyDelay(100);
                                }
                            }
                            break;
                        default:
                            state = 0;
                            break;
                        }
                    }
                }
            }
        }
    }
}