#include "project.h"
#include "chip8.h"

#define MAGIC_START 'S'
#define MAGIC_END 'E'

#define RECV_MGSH 0x00
#define RECV_ESLB 0x01
#define RECV_ESHB 0x02
#define RECV_DATA 0x03
#define RECV_MGEH 0x04

void usb_uart_echo()
{
    usb_uart_Start(0, usb_uart_5V_OPERATION);
    while(!usb_uart_GetConfiguration());
    usb_uart_CDC_Init();
    
    /*
        Every received character is acknowledged by returning character + 1
        Magic Start Header = 0x69
        Magic End Header = 0x31
    
        Receiver FSM
        States:
            RECV_MGSH = Waiting for Magic Start Header
            RECV_ESLB = Received Magic Start Header, waiting for expected low byte
            RECV_ESHB = Received low byte, waiting for expected size high byte
            RECV_DATA = Received expected size, waiting for data
            RECV_MGEH = Data received, waiting for Magic End Header
    */
    
    uint8_t state = RECV_MGSH;
    uint16_t expected = 0;
    uint16_t received = 0;
    uint8_t buffer[0x1000];
    while (1){
        if(usb_uart_DataIsReady()){
            uint8_t ch = usb_uart_GetChar();
            switch(state){
            case RECV_MGSH:
                if (ch == MAGIC_START){
                    while (!usb_uart_CDCIsReady());
                    usb_uart_PutChar(ch + 1);
                    state = RECV_ESHB;
                }
                break;
            case RECV_ESHB:
                expected = ch;
                expected <<= 8;
                while (!usb_uart_CDCIsReady());
                usb_uart_PutChar(ch + 1);
                state = RECV_ESLB;
                break;
            case RECV_ESLB:
                expected |= ch;
                while (!usb_uart_CDCIsReady());
                usb_uart_PutChar(ch + 1);
                state = RECV_DATA;
                break;
            case RECV_DATA:
                buffer[received++] = ch;
                while (!usb_uart_CDCIsReady());
                usb_uart_PutChar(ch + 1);
                if (received == expected){
                    state = RECV_MGEH;
                }
                break;
            case RECV_MGEH:
                if (ch == MAGIC_END){
                    while (!usb_uart_CDCIsReady());
                    usb_uart_PutChar(ch + 1);
                    for (uint16_t i = 0; i < received; i++){
                        eeprom_WriteByte(buffer[i], i);
                    }
                    while(1){
                        pin_led_Write(1);
                    }
                } else {
                    while(1){
                        pin_led_Write(1);
                        CyDelay(100);
                        pin_led_Write(0);
                        CyDelay(100);
                    }
                }
                break;
            default:
                state = RECV_MGSH;
            }
        }
    }
}