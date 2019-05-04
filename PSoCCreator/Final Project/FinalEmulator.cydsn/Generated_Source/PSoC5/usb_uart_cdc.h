/***************************************************************************//**
* \file usb_uart_cdc.h
* \version 3.20
*
* \brief
*  This file provides function prototypes and constants for the USBFS component 
*  CDC class.
*
* Related Document:
*  Universal Serial Bus Class Definitions for Communication Devices Version 1.1
*
********************************************************************************
* \copyright
* Copyright 2012-2016, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_USBFS_usb_uart_cdc_H)
#define CY_USBFS_usb_uart_cdc_H

#include "usb_uart.h"


/*******************************************************************************
* Prototypes of the usb_uart_cdc API.
*******************************************************************************/
/**
* \addtogroup group_cdc
* @{
*/
#if (usb_uart_ENABLE_CDC_CLASS_API != 0u)
    uint8 usb_uart_CDC_Init(void)            ;
    void usb_uart_PutData(const uint8* pData, uint16 length) ;
    void usb_uart_PutString(const char8 string[])            ;
    void usb_uart_PutChar(char8 txDataByte) ;
    void usb_uart_PutCRLF(void)             ;
    uint16 usb_uart_GetCount(void)          ;
    uint8  usb_uart_CDCIsReady(void)        ;
    uint8  usb_uart_DataIsReady(void)       ;
    uint16 usb_uart_GetData(uint8* pData, uint16 length)     ;
    uint16 usb_uart_GetAll(uint8* pData)    ;
    uint8  usb_uart_GetChar(void)           ;
    uint8  usb_uart_IsLineChanged(void)     ;
    uint32 usb_uart_GetDTERate(void)        ;
    uint8  usb_uart_GetCharFormat(void)     ;
    uint8  usb_uart_GetParityType(void)     ;
    uint8  usb_uart_GetDataBits(void)       ;
    uint16 usb_uart_GetLineControl(void)    ;
    void usb_uart_SendSerialState (uint16 serialState) ;
    uint16 usb_uart_GetSerialState (void)   ;
    void usb_uart_SetComPort (uint8 comNumber) ;
    uint8 usb_uart_GetComPort (void)        ;
    uint8 usb_uart_NotificationIsReady(void) ;

#endif  /* (usb_uart_ENABLE_CDC_CLASS_API) */
/** @} cdc */

/*******************************************************************************
*  Constants for usb_uart_cdc API.
*******************************************************************************/

/* CDC Class-Specific Request Codes (CDC ver 1.2 Table 19) */
#define usb_uart_CDC_SET_LINE_CODING        (0x20u)
#define usb_uart_CDC_GET_LINE_CODING        (0x21u)
#define usb_uart_CDC_SET_CONTROL_LINE_STATE (0x22u)

/*PSTN Subclass Specific Notifications (CDC ver 1.2 Table 30)*/
#define usb_uart_SERIAL_STATE               (0x20u)

#define usb_uart_LINE_CODING_CHANGED        (0x01u)
#define usb_uart_LINE_CONTROL_CHANGED       (0x02u)

#define usb_uart_1_STOPBIT                  (0x00u)
#define usb_uart_1_5_STOPBITS               (0x01u)
#define usb_uart_2_STOPBITS                 (0x02u)

#define usb_uart_PARITY_NONE                (0x00u)
#define usb_uart_PARITY_ODD                 (0x01u)
#define usb_uart_PARITY_EVEN                (0x02u)
#define usb_uart_PARITY_MARK                (0x03u)
#define usb_uart_PARITY_SPACE               (0x04u)

#define usb_uart_LINE_CODING_SIZE           (0x07u)
#define usb_uart_LINE_CODING_RATE           (0x00u)
#define usb_uart_LINE_CODING_STOP_BITS      (0x04u)
#define usb_uart_LINE_CODING_PARITY         (0x05u)
#define usb_uart_LINE_CODING_DATA_BITS      (0x06u)

#define usb_uart_LINE_CONTROL_DTR           (0x01u)
#define usb_uart_LINE_CONTROL_RTS           (0x02u)

#define usb_uart_MAX_MULTI_COM_NUM          (2u) 

#define usb_uart_COM_PORT1                  (0u) 
#define usb_uart_COM_PORT2                  (1u) 

#define usb_uart_SUCCESS                    (0u)
#define usb_uart_FAILURE                    (1u)

#define usb_uart_SERIAL_STATE_SIZE          (10u)

/* SerialState constants*/
#define usb_uart_SERIAL_STATE_REQUEST_TYPE  (0xA1u)
#define usb_uart_SERIAL_STATE_LENGTH        (0x2u)

/*******************************************************************************
* External data references
*******************************************************************************/
/**
* \addtogroup group_cdc
* @{
*/
extern volatile uint8  usb_uart_linesCoding[usb_uart_MAX_MULTI_COM_NUM][usb_uart_LINE_CODING_SIZE];
extern volatile uint8  usb_uart_linesChanged[usb_uart_MAX_MULTI_COM_NUM];
extern volatile uint16 usb_uart_linesControlBitmap[usb_uart_MAX_MULTI_COM_NUM];
extern volatile uint16 usb_uart_serialStateBitmap[usb_uart_MAX_MULTI_COM_NUM];
extern volatile uint8  usb_uart_cdcDataInEp[usb_uart_MAX_MULTI_COM_NUM];
extern volatile uint8  usb_uart_cdcDataOutEp[usb_uart_MAX_MULTI_COM_NUM];
extern volatile uint8  usb_uart_cdcCommInInterruptEp[usb_uart_MAX_MULTI_COM_NUM];
/** @} cdc */

/*******************************************************************************
* The following code is DEPRECATED and
* must not be used.
*******************************************************************************/


#define usb_uart_lineCoding             usb_uart_linesCoding[0]
#define usb_uart_lineChanged            usb_uart_linesChanged[0]
#define usb_uart_lineControlBitmap      usb_uart_linesControlBitmap[0]
#define usb_uart_cdc_data_in_ep         usb_uart_cdcDataInEp[0]
#define usb_uart_cdc_data_out_ep        usb_uart_cdcDataOutEp[0]

#endif /* (CY_USBFS_usb_uart_cdc_H) */


/* [] END OF FILE */
