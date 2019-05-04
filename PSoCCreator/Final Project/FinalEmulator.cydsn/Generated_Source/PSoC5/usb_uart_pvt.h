/***************************************************************************//**
* \file .h
* \version 3.20
*
* \brief
*  This file provides private function prototypes and constants for the 
*  USBFS component. It is not intended to be used in the user project.
*
********************************************************************************
* \copyright
* Copyright 2013-2016, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_USBFS_usb_uart_pvt_H)
#define CY_USBFS_usb_uart_pvt_H

#include "usb_uart.h"
   
#ifdef usb_uart_ENABLE_AUDIO_CLASS
    #include "usb_uart_audio.h"
#endif /* usb_uart_ENABLE_AUDIO_CLASS */

#ifdef usb_uart_ENABLE_CDC_CLASS
    #include "usb_uart_cdc.h"
#endif /* usb_uart_ENABLE_CDC_CLASS */

#if (usb_uart_ENABLE_MIDI_CLASS)
    #include "usb_uart_midi.h"
#endif /* (usb_uart_ENABLE_MIDI_CLASS) */

#if (usb_uart_ENABLE_MSC_CLASS)
    #include "usb_uart_msc.h"
#endif /* (usb_uart_ENABLE_MSC_CLASS) */

#if (usb_uart_EP_MANAGEMENT_DMA)
    #if (CY_PSOC4)
        #include <CyDMA.h>
    #else
        #include <CyDmac.h>
        #if ((usb_uart_EP_MANAGEMENT_DMA_AUTO) && (usb_uart_EP_DMA_AUTO_OPT == 0u))
            #include "usb_uart_EP_DMA_Done_isr.h"
            #include "usb_uart_EP8_DMA_Done_SR.h"
            #include "usb_uart_EP17_DMA_Done_SR.h"
        #endif /* ((usb_uart_EP_MANAGEMENT_DMA_AUTO) && (usb_uart_EP_DMA_AUTO_OPT == 0u)) */
    #endif /* (CY_PSOC4) */
#endif /* (usb_uart_EP_MANAGEMENT_DMA) */

#if (usb_uart_DMA1_ACTIVE)
    #include "usb_uart_ep1_dma.h"
    #define usb_uart_EP1_DMA_CH     (usb_uart_ep1_dma_CHANNEL)
#endif /* (usb_uart_DMA1_ACTIVE) */

#if (usb_uart_DMA2_ACTIVE)
    #include "usb_uart_ep2_dma.h"
    #define usb_uart_EP2_DMA_CH     (usb_uart_ep2_dma_CHANNEL)
#endif /* (usb_uart_DMA2_ACTIVE) */

#if (usb_uart_DMA3_ACTIVE)
    #include "usb_uart_ep3_dma.h"
    #define usb_uart_EP3_DMA_CH     (usb_uart_ep3_dma_CHANNEL)
#endif /* (usb_uart_DMA3_ACTIVE) */

#if (usb_uart_DMA4_ACTIVE)
    #include "usb_uart_ep4_dma.h"
    #define usb_uart_EP4_DMA_CH     (usb_uart_ep4_dma_CHANNEL)
#endif /* (usb_uart_DMA4_ACTIVE) */

#if (usb_uart_DMA5_ACTIVE)
    #include "usb_uart_ep5_dma.h"
    #define usb_uart_EP5_DMA_CH     (usb_uart_ep5_dma_CHANNEL)
#endif /* (usb_uart_DMA5_ACTIVE) */

#if (usb_uart_DMA6_ACTIVE)
    #include "usb_uart_ep6_dma.h"
    #define usb_uart_EP6_DMA_CH     (usb_uart_ep6_dma_CHANNEL)
#endif /* (usb_uart_DMA6_ACTIVE) */

#if (usb_uart_DMA7_ACTIVE)
    #include "usb_uart_ep7_dma.h"
    #define usb_uart_EP7_DMA_CH     (usb_uart_ep7_dma_CHANNEL)
#endif /* (usb_uart_DMA7_ACTIVE) */

#if (usb_uart_DMA8_ACTIVE)
    #include "usb_uart_ep8_dma.h"
    #define usb_uart_EP8_DMA_CH     (usb_uart_ep8_dma_CHANNEL)
#endif /* (usb_uart_DMA8_ACTIVE) */


/***************************************
*     Private Variables
***************************************/

/* Generated external references for descriptors. */
extern const uint8 CYCODE usb_uart_DEVICE0_DESCR[18u];
extern const uint8 CYCODE usb_uart_DEVICE0_CONFIGURATION0_DESCR[67u];
extern const T_usb_uart_EP_SETTINGS_BLOCK CYCODE usb_uart_DEVICE0_CONFIGURATION0_EP_SETTINGS_TABLE[3u];
extern const uint8 CYCODE usb_uart_DEVICE0_CONFIGURATION0_INTERFACE_CLASS[2u];
extern const T_usb_uart_LUT CYCODE usb_uart_DEVICE0_CONFIGURATION0_TABLE[5u];
extern const T_usb_uart_LUT CYCODE usb_uart_DEVICE0_TABLE[3u];
extern const T_usb_uart_LUT CYCODE usb_uart_TABLE[1u];
extern const uint8 CYCODE usb_uart_SN_STRING_DESCRIPTOR[2];
extern const uint8 CYCODE usb_uart_STRING_DESCRIPTORS[159u];


extern const uint8 CYCODE usb_uart_MSOS_DESCRIPTOR[usb_uart_MSOS_DESCRIPTOR_LENGTH];
extern const uint8 CYCODE usb_uart_MSOS_CONFIGURATION_DESCR[usb_uart_MSOS_CONF_DESCR_LENGTH];
#if defined(usb_uart_ENABLE_IDSN_STRING)
    extern uint8 usb_uart_idSerialNumberStringDescriptor[usb_uart_IDSN_DESCR_LENGTH];
#endif /* (usb_uart_ENABLE_IDSN_STRING) */

extern volatile uint8 usb_uart_interfaceNumber;
extern volatile uint8 usb_uart_interfaceSetting[usb_uart_MAX_INTERFACES_NUMBER];
extern volatile uint8 usb_uart_interfaceSettingLast[usb_uart_MAX_INTERFACES_NUMBER];
extern volatile uint8 usb_uart_deviceAddress;
extern volatile uint8 usb_uart_interfaceStatus[usb_uart_MAX_INTERFACES_NUMBER];
extern const uint8 CYCODE *usb_uart_interfaceClass;

extern volatile T_usb_uart_EP_CTL_BLOCK usb_uart_EP[usb_uart_MAX_EP];
extern volatile T_usb_uart_TD usb_uart_currentTD;

#if (usb_uart_EP_MANAGEMENT_DMA)
    #if (CY_PSOC4)
        extern const uint8 usb_uart_DmaChan[usb_uart_MAX_EP];
    #else
        extern uint8 usb_uart_DmaChan[usb_uart_MAX_EP];
        extern uint8 usb_uart_DmaTd  [usb_uart_MAX_EP];
    #endif /* (CY_PSOC4) */
#endif /* (usb_uart_EP_MANAGEMENT_DMA) */

#if (usb_uart_EP_MANAGEMENT_DMA_AUTO)
#if (CY_PSOC4)
    extern uint8  usb_uart_DmaEpBurstCnt   [usb_uart_MAX_EP];
    extern uint8  usb_uart_DmaEpLastBurstEl[usb_uart_MAX_EP];

    extern uint8  usb_uart_DmaEpBurstCntBackup  [usb_uart_MAX_EP];
    extern uint32 usb_uart_DmaEpBufferAddrBackup[usb_uart_MAX_EP];
    
    extern const uint8 usb_uart_DmaReqOut     [usb_uart_MAX_EP];    
    extern const uint8 usb_uart_DmaBurstEndOut[usb_uart_MAX_EP];
#else
    #if (usb_uart_EP_DMA_AUTO_OPT == 0u)
        extern uint8 usb_uart_DmaNextTd[usb_uart_MAX_EP];
        extern volatile uint16 usb_uart_inLength [usb_uart_MAX_EP];
        extern volatile uint8  usb_uart_inBufFull[usb_uart_MAX_EP];
        extern const uint8 usb_uart_epX_TD_TERMOUT_EN[usb_uart_MAX_EP];
        extern const uint8 *usb_uart_inDataPointer[usb_uart_MAX_EP];
    #endif /* (usb_uart_EP_DMA_AUTO_OPT == 0u) */
#endif /* CY_PSOC4 */
#endif /* (usb_uart_EP_MANAGEMENT_DMA_AUTO) */

extern volatile uint8 usb_uart_ep0Toggle;
extern volatile uint8 usb_uart_lastPacketSize;
extern volatile uint8 usb_uart_ep0Mode;
extern volatile uint8 usb_uart_ep0Count;
extern volatile uint16 usb_uart_transferByteCount;


/***************************************
*     Private Function Prototypes
***************************************/
void  usb_uart_ReInitComponent(void)            ;
void  usb_uart_HandleSetup(void)                ;
void  usb_uart_HandleIN(void)                   ;
void  usb_uart_HandleOUT(void)                  ;
void  usb_uart_LoadEP0(void)                    ;
uint8 usb_uart_InitControlRead(void)            ;
uint8 usb_uart_InitControlWrite(void)           ;
void  usb_uart_ControlReadDataStage(void)       ;
void  usb_uart_ControlReadStatusStage(void)     ;
void  usb_uart_ControlReadPrematureStatus(void) ;
uint8 usb_uart_InitControlWrite(void)           ;
uint8 usb_uart_InitZeroLengthControlTransfer(void) ;
void  usb_uart_ControlWriteDataStage(void)      ;
void  usb_uart_ControlWriteStatusStage(void)    ;
void  usb_uart_ControlWritePrematureStatus(void);
uint8 usb_uart_InitNoDataControlTransfer(void)  ;
void  usb_uart_NoDataControlStatusStage(void)   ;
void  usb_uart_InitializeStatusBlock(void)      ;
void  usb_uart_UpdateStatusBlock(uint8 completionCode) ;
uint8 usb_uart_DispatchClassRqst(void)          ;

void usb_uart_Config(uint8 clearAltSetting) ;
void usb_uart_ConfigAltChanged(void)        ;
void usb_uart_ConfigReg(void)               ;
void usb_uart_EpStateInit(void)             ;


const T_usb_uart_LUT CYCODE *usb_uart_GetConfigTablePtr(uint8 confIndex);
const T_usb_uart_LUT CYCODE *usb_uart_GetDeviceTablePtr(void)           ;
#if (usb_uart_BOS_ENABLE)
    const T_usb_uart_LUT CYCODE *usb_uart_GetBOSPtr(void)               ;
#endif /* (usb_uart_BOS_ENABLE) */
const uint8 CYCODE *usb_uart_GetInterfaceClassTablePtr(void)                    ;
uint8 usb_uart_ClearEndpointHalt(void)                                          ;
uint8 usb_uart_SetEndpointHalt(void)                                            ;
uint8 usb_uart_ValidateAlternateSetting(void)                                   ;

void usb_uart_SaveConfig(void)      ;
void usb_uart_RestoreConfig(void)   ;

#if (CY_PSOC3 || CY_PSOC5LP)
    #if (usb_uart_EP_MANAGEMENT_DMA_AUTO && (usb_uart_EP_DMA_AUTO_OPT == 0u))
        void usb_uart_LoadNextInEP(uint8 epNumber, uint8 mode)  ;
    #endif /* (usb_uart_EP_MANAGEMENT_DMA_AUTO && (usb_uart_EP_DMA_AUTO_OPT == 0u)) */
#endif /* (CY_PSOC3 || CY_PSOC5LP) */

#if defined(usb_uart_ENABLE_IDSN_STRING)
    void usb_uart_ReadDieID(uint8 descr[])  ;
#endif /* usb_uart_ENABLE_IDSN_STRING */

#if defined(usb_uart_ENABLE_HID_CLASS)
    uint8 usb_uart_DispatchHIDClassRqst(void) ;
#endif /* (usb_uart_ENABLE_HID_CLASS) */

#if defined(usb_uart_ENABLE_AUDIO_CLASS)
    uint8 usb_uart_DispatchAUDIOClassRqst(void) ;
#endif /* (usb_uart_ENABLE_AUDIO_CLASS) */

#if defined(usb_uart_ENABLE_CDC_CLASS)
    uint8 usb_uart_DispatchCDCClassRqst(void) ;
#endif /* (usb_uart_ENABLE_CDC_CLASS) */

#if (usb_uart_ENABLE_MSC_CLASS)
    #if (usb_uart_HANDLE_MSC_REQUESTS)
        uint8 usb_uart_DispatchMSCClassRqst(void) ;
    #endif /* (usb_uart_HANDLE_MSC_REQUESTS) */
#endif /* (usb_uart_ENABLE_MSC_CLASS */

CY_ISR_PROTO(usb_uart_EP_0_ISR);
CY_ISR_PROTO(usb_uart_BUS_RESET_ISR);

#if (usb_uart_SOF_ISR_ACTIVE)
    CY_ISR_PROTO(usb_uart_SOF_ISR);
#endif /* (usb_uart_SOF_ISR_ACTIVE) */

#if (usb_uart_EP1_ISR_ACTIVE)
    CY_ISR_PROTO(usb_uart_EP_1_ISR);
#endif /* (usb_uart_EP1_ISR_ACTIVE) */

#if (usb_uart_EP2_ISR_ACTIVE)
    CY_ISR_PROTO(usb_uart_EP_2_ISR);
#endif /* (usb_uart_EP2_ISR_ACTIVE) */

#if (usb_uart_EP3_ISR_ACTIVE)
    CY_ISR_PROTO(usb_uart_EP_3_ISR);
#endif /* (usb_uart_EP3_ISR_ACTIVE) */

#if (usb_uart_EP4_ISR_ACTIVE)
    CY_ISR_PROTO(usb_uart_EP_4_ISR);
#endif /* (usb_uart_EP4_ISR_ACTIVE) */

#if (usb_uart_EP5_ISR_ACTIVE)
    CY_ISR_PROTO(usb_uart_EP_5_ISR);
#endif /* (usb_uart_EP5_ISR_ACTIVE) */

#if (usb_uart_EP6_ISR_ACTIVE)
    CY_ISR_PROTO(usb_uart_EP_6_ISR);
#endif /* (usb_uart_EP6_ISR_ACTIVE) */

#if (usb_uart_EP7_ISR_ACTIVE)
    CY_ISR_PROTO(usb_uart_EP_7_ISR);
#endif /* (usb_uart_EP7_ISR_ACTIVE) */

#if (usb_uart_EP8_ISR_ACTIVE)
    CY_ISR_PROTO(usb_uart_EP_8_ISR);
#endif /* (usb_uart_EP8_ISR_ACTIVE) */

#if (usb_uart_EP_MANAGEMENT_DMA)
    CY_ISR_PROTO(usb_uart_ARB_ISR);
#endif /* (usb_uart_EP_MANAGEMENT_DMA) */

#if (usb_uart_DP_ISR_ACTIVE)
    CY_ISR_PROTO(usb_uart_DP_ISR);
#endif /* (usb_uart_DP_ISR_ACTIVE) */

#if (CY_PSOC4)
    CY_ISR_PROTO(usb_uart_INTR_HI_ISR);
    CY_ISR_PROTO(usb_uart_INTR_MED_ISR);
    CY_ISR_PROTO(usb_uart_INTR_LO_ISR);
    #if (usb_uart_LPM_ACTIVE)
        CY_ISR_PROTO(usb_uart_LPM_ISR);
    #endif /* (usb_uart_LPM_ACTIVE) */
#endif /* (CY_PSOC4) */

#if (usb_uart_EP_MANAGEMENT_DMA_AUTO)
#if (CY_PSOC4)
    #if (usb_uart_DMA1_ACTIVE)
        void usb_uart_EP1_DMA_DONE_ISR(void);
    #endif /* (usb_uart_DMA1_ACTIVE) */

    #if (usb_uart_DMA2_ACTIVE)
        void usb_uart_EP2_DMA_DONE_ISR(void);
    #endif /* (usb_uart_DMA2_ACTIVE) */

    #if (usb_uart_DMA3_ACTIVE)
        void usb_uart_EP3_DMA_DONE_ISR(void);
    #endif /* (usb_uart_DMA3_ACTIVE) */

    #if (usb_uart_DMA4_ACTIVE)
        void usb_uart_EP4_DMA_DONE_ISR(void);
    #endif /* (usb_uart_DMA4_ACTIVE) */

    #if (usb_uart_DMA5_ACTIVE)
        void usb_uart_EP5_DMA_DONE_ISR(void);
    #endif /* (usb_uart_DMA5_ACTIVE) */

    #if (usb_uart_DMA6_ACTIVE)
        void usb_uart_EP6_DMA_DONE_ISR(void);
    #endif /* (usb_uart_DMA6_ACTIVE) */

    #if (usb_uart_DMA7_ACTIVE)
        void usb_uart_EP7_DMA_DONE_ISR(void);
    #endif /* (usb_uart_DMA7_ACTIVE) */

    #if (usb_uart_DMA8_ACTIVE)
        void usb_uart_EP8_DMA_DONE_ISR(void);
    #endif /* (usb_uart_DMA8_ACTIVE) */

#else
    #if (usb_uart_EP_DMA_AUTO_OPT == 0u)
        CY_ISR_PROTO(usb_uart_EP_DMA_DONE_ISR);
    #endif /* (usb_uart_EP_DMA_AUTO_OPT == 0u) */
#endif /* (CY_PSOC4) */
#endif /* (usb_uart_EP_MANAGEMENT_DMA_AUTO) */


/***************************************
*         Request Handlers
***************************************/

uint8 usb_uart_HandleStandardRqst(void) ;
uint8 usb_uart_DispatchClassRqst(void)  ;
uint8 usb_uart_HandleVendorRqst(void)   ;


/***************************************
*    HID Internal references
***************************************/

#if defined(usb_uart_ENABLE_HID_CLASS)
    void usb_uart_FindReport(void)            ;
    void usb_uart_FindReportDescriptor(void)  ;
    void usb_uart_FindHidClassDecriptor(void) ;
#endif /* usb_uart_ENABLE_HID_CLASS */


/***************************************
*    MIDI Internal references
***************************************/

#if defined(usb_uart_ENABLE_MIDI_STREAMING)
    void usb_uart_MIDI_IN_EP_Service(void)  ;
#endif /* (usb_uart_ENABLE_MIDI_STREAMING) */


/***************************************
*    CDC Internal references
***************************************/

#if defined(usb_uart_ENABLE_CDC_CLASS)

    typedef struct
    {
        uint8  bRequestType;
        uint8  bNotification;
        uint8  wValue;
        uint8  wValueMSB;
        uint8  wIndex;
        uint8  wIndexMSB;
        uint8  wLength;
        uint8  wLengthMSB;
        uint8  wSerialState;
        uint8  wSerialStateMSB;
    } t_usb_uart_cdc_notification;

    uint8 usb_uart_GetInterfaceComPort(uint8 interface) ;
    uint8 usb_uart_Cdc_EpInit( const T_usb_uart_EP_SETTINGS_BLOCK CYCODE *pEP, uint8 epNum, uint8 cdcComNums) ;

    extern volatile uint8  usb_uart_cdc_dataInEpList[usb_uart_MAX_MULTI_COM_NUM];
    extern volatile uint8  usb_uart_cdc_dataOutEpList[usb_uart_MAX_MULTI_COM_NUM];
    extern volatile uint8  usb_uart_cdc_commInEpList[usb_uart_MAX_MULTI_COM_NUM];
#endif /* (usb_uart_ENABLE_CDC_CLASS) */


#endif /* CY_USBFS_usb_uart_pvt_H */


/* [] END OF FILE */
