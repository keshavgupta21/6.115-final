/***************************************************************************//**
* \file usb_uart_audio.h
* \version 3.20
*
* \brief
*  This file provides function prototypes and constants for the USBFS component 
*  Audio class.
*
* Related Document:
*  Universal Serial Bus Device Class Definition for Audio Devices Release 1.0
*
********************************************************************************
* \copyright
* Copyright 2008-2016, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_USBFS_usb_uart_audio_H)
#define CY_USBFS_usb_uart_audio_H

#include "usb_uart.h"


/***************************************
* Custom Declarations
***************************************/

/* `#START CUSTOM_CONSTANTS` Place your declaration here */

/* `#END` */


/***************************************
*  Constants for usb_uart_audio API.
***************************************/

/* Audio Class-Specific Request Codes (AUDIO Table A-9) */
#define usb_uart_REQUEST_CODE_UNDEFINED     (0x00u)
#define usb_uart_SET_CUR                    (0x01u)
#define usb_uart_GET_CUR                    (0x81u)
#define usb_uart_SET_MIN                    (0x02u)
#define usb_uart_GET_MIN                    (0x82u)
#define usb_uart_SET_MAX                    (0x03u)
#define usb_uart_GET_MAX                    (0x83u)
#define usb_uart_SET_RES                    (0x04u)
#define usb_uart_GET_RES                    (0x84u)
#define usb_uart_SET_MEM                    (0x05u)
#define usb_uart_GET_MEM                    (0x85u)
#define usb_uart_GET_STAT                   (0xFFu)

/* point Control Selectors (AUDIO Table A-19) */
#define usb_uart_EP_CONTROL_UNDEFINED       (0x00u)
#define usb_uart_SAMPLING_FREQ_CONTROL      (0x01u)
#define usb_uart_PITCH_CONTROL              (0x02u)

/* Feature Unit Control Selectors (AUDIO Table A-11) */
#define usb_uart_FU_CONTROL_UNDEFINED       (0x00u)
#define usb_uart_MUTE_CONTROL               (0x01u)
#define usb_uart_VOLUME_CONTROL             (0x02u)
#define usb_uart_BASS_CONTROL               (0x03u)
#define usb_uart_MID_CONTROL                (0x04u)
#define usb_uart_TREBLE_CONTROL             (0x05u)
#define usb_uart_GRAPHIC_EQUALIZER_CONTROL  (0x06u)
#define usb_uart_AUTOMATIC_GAIN_CONTROL     (0x07u)
#define usb_uart_DELAY_CONTROL              (0x08u)
#define usb_uart_BASS_BOOST_CONTROL         (0x09u)
#define usb_uart_LOUDNESS_CONTROL           (0x0Au)

#define usb_uart_SAMPLE_FREQ_LEN            (3u)
#define usb_uart_VOLUME_LEN                 (2u)

#if !defined(USER_SUPPLIED_DEFAULT_VOLUME_VALUE)
    #define usb_uart_VOL_MIN_MSB            (0x80u)
    #define usb_uart_VOL_MIN_LSB            (0x01u)
    #define usb_uart_VOL_MAX_MSB            (0x7Fu)
    #define usb_uart_VOL_MAX_LSB            (0xFFu)
    #define usb_uart_VOL_RES_MSB            (0x00u)
    #define usb_uart_VOL_RES_LSB            (0x01u)
#endif /* USER_SUPPLIED_DEFAULT_VOLUME_VALUE */


/***************************************
* External data references
***************************************/
/**
* \addtogroup group_audio
* @{
*/
extern volatile uint8 usb_uart_currentSampleFrequency[usb_uart_MAX_EP][usb_uart_SAMPLE_FREQ_LEN];
extern volatile uint8 usb_uart_frequencyChanged;
extern volatile uint8 usb_uart_currentMute;
extern volatile uint8 usb_uart_currentVolume[usb_uart_VOLUME_LEN];
/** @} audio */

extern volatile uint8 usb_uart_minimumVolume[usb_uart_VOLUME_LEN];
extern volatile uint8 usb_uart_maximumVolume[usb_uart_VOLUME_LEN];
extern volatile uint8 usb_uart_resolutionVolume[usb_uart_VOLUME_LEN];

#endif /*  CY_USBFS_usb_uart_audio_H */


/* [] END OF FILE */
