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
            0: Expecting a 0x00 bytes (initial state)
            1: Expecting a 0x01 byte
               acknowledge transfer by sending 0x02 and 0x03
            2: Epecting the length of bytes to receive (high 8 bits)
            3: Epecting the length of bytes to receive (low 8 bits)
               acknowledge transfer by sending the high and low 8 bits of size
            4: Set counter = 0 and receive the bytes
            5: acknowledge transfer by sending 0x04 and 0x05
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
                    /* Receive and save. */
                    for (uint8_t i = 0; i < count; i++){
                        switch (state){
                            case 0:
                                if (buffer[i] == 0x00){
                                    state = 1;
                                }
                                break;
                            case 1:
                                if (buffer[i] == 0x01){
                                    /* Wait until component is ready to send data to host. */
                                    while (usb_uart_CDCIsReady() == 0){};
                                    uint8_t tbuf[] = {0x02, 0x03};
                                    usb_uart_PutData(tbuf, 2);
                                    state = 2;
                                }
                                break;
                            case 2:
                                length |= ((uint16_t)(buffer[i])) << 8;
                                state = 3;
                                break;
                            case 3:{
                                length |= buffer[i];
                                counter = 0;
                                /* Wait until component is ready to send data to host. */
                                while (0u == usb_uart_CDCIsReady()){};
                                uint8_t tbuf[] = {(length & 0xff00) >> 8, length & 0xff};
                                usb_uart_PutData(tbuf, 2);
                                eeprom_WriteByte(tbuf[0], 0);
                                eeprom_WriteByte(tbuf[1], 1);
                                state = 4;
                                break;
                            }
                            case 4:
                                eeprom_WriteByte(buffer[i], 2 + counter++);
                                if (counter == length) {
                                    state = 5;
                                }
                                break;
                            case 5:{
                                /* Wait until component is ready to send data to host. */
                                while (usb_uart_CDCIsReady() == 0){};
                                uint8_t tbuf[] = {0x04, 0x05};
                                usb_uart_PutData(tbuf, 2);
                                /* Wait until component is ready to send data to host. */
                                while (usb_uart_CDCIsReady() == 0){};
                                pin_led_Write(1);
                                state = 0;
                                //return;
                            }
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