/*******************************************************************************
* File Name: spi_oled.h
* Version 2.50
*
* Description:
*  Contains the function prototypes, constants and register definition
*  of the SPI Master Component.
*
* Note:
*  None
*
********************************************************************************
* Copyright 2008-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_SPIM_spi_oled_H)
#define CY_SPIM_spi_oled_H

#include "cyfitter.h"
#include "cytypes.h"
#include "CyLib.h" /* For CyEnterCriticalSection() and CyExitCriticalSection() functions */


/***************************************
*   Conditional Compilation Parameters
***************************************/

#define spi_oled_INTERNAL_CLOCK             (1u)

#if(0u != spi_oled_INTERNAL_CLOCK)
    #include "spi_oled_IntClock.h"
#endif /* (0u != spi_oled_INTERNAL_CLOCK) */

#define spi_oled_MODE                       (1u)
#define spi_oled_DATA_WIDTH                 (8u)
#define spi_oled_MODE_USE_ZERO              (1u)
#define spi_oled_BIDIRECTIONAL_MODE         (0u)

/* Internal interrupt handling */
#define spi_oled_TX_BUFFER_SIZE             (4u)
#define spi_oled_RX_BUFFER_SIZE             (4u)
#define spi_oled_INTERNAL_TX_INT_ENABLED    (0u)
#define spi_oled_INTERNAL_RX_INT_ENABLED    (0u)

#define spi_oled_SINGLE_REG_SIZE            (8u)
#define spi_oled_USE_SECOND_DATAPATH        (spi_oled_DATA_WIDTH > spi_oled_SINGLE_REG_SIZE)

#define spi_oled_FIFO_SIZE                  (4u)
#define spi_oled_TX_SOFTWARE_BUF_ENABLED    ((0u != spi_oled_INTERNAL_TX_INT_ENABLED) && \
                                                     (spi_oled_TX_BUFFER_SIZE > spi_oled_FIFO_SIZE))

#define spi_oled_RX_SOFTWARE_BUF_ENABLED    ((0u != spi_oled_INTERNAL_RX_INT_ENABLED) && \
                                                     (spi_oled_RX_BUFFER_SIZE > spi_oled_FIFO_SIZE))


/***************************************
*        Data Struct Definition
***************************************/

/* Sleep Mode API Support */
typedef struct
{
    uint8 enableState;
    uint8 cntrPeriod;
} spi_oled_BACKUP_STRUCT;


/***************************************
*        Function Prototypes
***************************************/

void  spi_oled_Init(void)                           ;
void  spi_oled_Enable(void)                         ;
void  spi_oled_Start(void)                          ;
void  spi_oled_Stop(void)                           ;

void  spi_oled_EnableTxInt(void)                    ;
void  spi_oled_EnableRxInt(void)                    ;
void  spi_oled_DisableTxInt(void)                   ;
void  spi_oled_DisableRxInt(void)                   ;

void  spi_oled_Sleep(void)                          ;
void  spi_oled_Wakeup(void)                         ;
void  spi_oled_SaveConfig(void)                     ;
void  spi_oled_RestoreConfig(void)                  ;

void  spi_oled_SetTxInterruptMode(uint8 intSrc)     ;
void  spi_oled_SetRxInterruptMode(uint8 intSrc)     ;
uint8 spi_oled_ReadTxStatus(void)                   ;
uint8 spi_oled_ReadRxStatus(void)                   ;
void  spi_oled_WriteTxData(uint8 txData)  \
                                                            ;
uint8 spi_oled_ReadRxData(void) \
                                                            ;
uint8 spi_oled_GetRxBufferSize(void)                ;
uint8 spi_oled_GetTxBufferSize(void)                ;
void  spi_oled_ClearRxBuffer(void)                  ;
void  spi_oled_ClearTxBuffer(void)                  ;
void  spi_oled_ClearFIFO(void)                              ;
void  spi_oled_PutArray(const uint8 buffer[], uint8 byteCount) \
                                                            ;

#if(0u != spi_oled_BIDIRECTIONAL_MODE)
    void  spi_oled_TxEnable(void)                   ;
    void  spi_oled_TxDisable(void)                  ;
#endif /* (0u != spi_oled_BIDIRECTIONAL_MODE) */

CY_ISR_PROTO(spi_oled_TX_ISR);
CY_ISR_PROTO(spi_oled_RX_ISR);


/***************************************
*   Variable with external linkage
***************************************/

extern uint8 spi_oled_initVar;


/***************************************
*           API Constants
***************************************/

#define spi_oled_TX_ISR_NUMBER     ((uint8) (spi_oled_TxInternalInterrupt__INTC_NUMBER))
#define spi_oled_RX_ISR_NUMBER     ((uint8) (spi_oled_RxInternalInterrupt__INTC_NUMBER))

#define spi_oled_TX_ISR_PRIORITY   ((uint8) (spi_oled_TxInternalInterrupt__INTC_PRIOR_NUM))
#define spi_oled_RX_ISR_PRIORITY   ((uint8) (spi_oled_RxInternalInterrupt__INTC_PRIOR_NUM))


/***************************************
*    Initial Parameter Constants
***************************************/

#define spi_oled_INT_ON_SPI_DONE    ((uint8) (0u   << spi_oled_STS_SPI_DONE_SHIFT))
#define spi_oled_INT_ON_TX_EMPTY    ((uint8) (0u   << spi_oled_STS_TX_FIFO_EMPTY_SHIFT))
#define spi_oled_INT_ON_TX_NOT_FULL ((uint8) (0u << \
                                                                           spi_oled_STS_TX_FIFO_NOT_FULL_SHIFT))
#define spi_oled_INT_ON_BYTE_COMP   ((uint8) (0u  << spi_oled_STS_BYTE_COMPLETE_SHIFT))
#define spi_oled_INT_ON_SPI_IDLE    ((uint8) (0u   << spi_oled_STS_SPI_IDLE_SHIFT))

/* Disable TX_NOT_FULL if software buffer is used */
#define spi_oled_INT_ON_TX_NOT_FULL_DEF ((spi_oled_TX_SOFTWARE_BUF_ENABLED) ? \
                                                                        (0u) : (spi_oled_INT_ON_TX_NOT_FULL))

/* TX interrupt mask */
#define spi_oled_TX_INIT_INTERRUPTS_MASK    (spi_oled_INT_ON_SPI_DONE  | \
                                                     spi_oled_INT_ON_TX_EMPTY  | \
                                                     spi_oled_INT_ON_TX_NOT_FULL_DEF | \
                                                     spi_oled_INT_ON_BYTE_COMP | \
                                                     spi_oled_INT_ON_SPI_IDLE)

#define spi_oled_INT_ON_RX_FULL         ((uint8) (0u << \
                                                                          spi_oled_STS_RX_FIFO_FULL_SHIFT))
#define spi_oled_INT_ON_RX_NOT_EMPTY    ((uint8) (0u << \
                                                                          spi_oled_STS_RX_FIFO_NOT_EMPTY_SHIFT))
#define spi_oled_INT_ON_RX_OVER         ((uint8) (0u << \
                                                                          spi_oled_STS_RX_FIFO_OVERRUN_SHIFT))

/* RX interrupt mask */
#define spi_oled_RX_INIT_INTERRUPTS_MASK    (spi_oled_INT_ON_RX_FULL      | \
                                                     spi_oled_INT_ON_RX_NOT_EMPTY | \
                                                     spi_oled_INT_ON_RX_OVER)
/* Nubmer of bits to receive/transmit */
#define spi_oled_BITCTR_INIT            (((uint8) (spi_oled_DATA_WIDTH << 1u)) - 1u)


/***************************************
*             Registers
***************************************/
#if(CY_PSOC3 || CY_PSOC5)
    #define spi_oled_TXDATA_REG (* (reg8 *) \
                                                spi_oled_BSPIM_sR8_Dp_u0__F0_REG)
    #define spi_oled_TXDATA_PTR (  (reg8 *) \
                                                spi_oled_BSPIM_sR8_Dp_u0__F0_REG)
    #define spi_oled_RXDATA_REG (* (reg8 *) \
                                                spi_oled_BSPIM_sR8_Dp_u0__F1_REG)
    #define spi_oled_RXDATA_PTR (  (reg8 *) \
                                                spi_oled_BSPIM_sR8_Dp_u0__F1_REG)
#else   /* PSOC4 */
    #if(spi_oled_USE_SECOND_DATAPATH)
        #define spi_oled_TXDATA_REG (* (reg16 *) \
                                          spi_oled_BSPIM_sR8_Dp_u0__16BIT_F0_REG)
        #define spi_oled_TXDATA_PTR (  (reg16 *) \
                                          spi_oled_BSPIM_sR8_Dp_u0__16BIT_F0_REG)
        #define spi_oled_RXDATA_REG (* (reg16 *) \
                                          spi_oled_BSPIM_sR8_Dp_u0__16BIT_F1_REG)
        #define spi_oled_RXDATA_PTR (  (reg16 *) \
                                          spi_oled_BSPIM_sR8_Dp_u0__16BIT_F1_REG)
    #else
        #define spi_oled_TXDATA_REG (* (reg8 *) \
                                                spi_oled_BSPIM_sR8_Dp_u0__F0_REG)
        #define spi_oled_TXDATA_PTR (  (reg8 *) \
                                                spi_oled_BSPIM_sR8_Dp_u0__F0_REG)
        #define spi_oled_RXDATA_REG (* (reg8 *) \
                                                spi_oled_BSPIM_sR8_Dp_u0__F1_REG)
        #define spi_oled_RXDATA_PTR (  (reg8 *) \
                                                spi_oled_BSPIM_sR8_Dp_u0__F1_REG)
    #endif /* (spi_oled_USE_SECOND_DATAPATH) */
#endif     /* (CY_PSOC3 || CY_PSOC5) */

#define spi_oled_AUX_CONTROL_DP0_REG (* (reg8 *) \
                                        spi_oled_BSPIM_sR8_Dp_u0__DP_AUX_CTL_REG)
#define spi_oled_AUX_CONTROL_DP0_PTR (  (reg8 *) \
                                        spi_oled_BSPIM_sR8_Dp_u0__DP_AUX_CTL_REG)

#if(spi_oled_USE_SECOND_DATAPATH)
    #define spi_oled_AUX_CONTROL_DP1_REG  (* (reg8 *) \
                                        spi_oled_BSPIM_sR8_Dp_u1__DP_AUX_CTL_REG)
    #define spi_oled_AUX_CONTROL_DP1_PTR  (  (reg8 *) \
                                        spi_oled_BSPIM_sR8_Dp_u1__DP_AUX_CTL_REG)
#endif /* (spi_oled_USE_SECOND_DATAPATH) */

#define spi_oled_COUNTER_PERIOD_REG     (* (reg8 *) spi_oled_BSPIM_BitCounter__PERIOD_REG)
#define spi_oled_COUNTER_PERIOD_PTR     (  (reg8 *) spi_oled_BSPIM_BitCounter__PERIOD_REG)
#define spi_oled_COUNTER_CONTROL_REG    (* (reg8 *) spi_oled_BSPIM_BitCounter__CONTROL_AUX_CTL_REG)
#define spi_oled_COUNTER_CONTROL_PTR    (  (reg8 *) spi_oled_BSPIM_BitCounter__CONTROL_AUX_CTL_REG)

#define spi_oled_TX_STATUS_REG          (* (reg8 *) spi_oled_BSPIM_TxStsReg__STATUS_REG)
#define spi_oled_TX_STATUS_PTR          (  (reg8 *) spi_oled_BSPIM_TxStsReg__STATUS_REG)
#define spi_oled_RX_STATUS_REG          (* (reg8 *) spi_oled_BSPIM_RxStsReg__STATUS_REG)
#define spi_oled_RX_STATUS_PTR          (  (reg8 *) spi_oled_BSPIM_RxStsReg__STATUS_REG)

#define spi_oled_CONTROL_REG            (* (reg8 *) \
                                      spi_oled_BSPIM_BidirMode_CtrlReg__CONTROL_REG)
#define spi_oled_CONTROL_PTR            (  (reg8 *) \
                                      spi_oled_BSPIM_BidirMode_CtrlReg__CONTROL_REG)

#define spi_oled_TX_STATUS_MASK_REG     (* (reg8 *) spi_oled_BSPIM_TxStsReg__MASK_REG)
#define spi_oled_TX_STATUS_MASK_PTR     (  (reg8 *) spi_oled_BSPIM_TxStsReg__MASK_REG)
#define spi_oled_RX_STATUS_MASK_REG     (* (reg8 *) spi_oled_BSPIM_RxStsReg__MASK_REG)
#define spi_oled_RX_STATUS_MASK_PTR     (  (reg8 *) spi_oled_BSPIM_RxStsReg__MASK_REG)

#define spi_oled_TX_STATUS_ACTL_REG     (* (reg8 *) spi_oled_BSPIM_TxStsReg__STATUS_AUX_CTL_REG)
#define spi_oled_TX_STATUS_ACTL_PTR     (  (reg8 *) spi_oled_BSPIM_TxStsReg__STATUS_AUX_CTL_REG)
#define spi_oled_RX_STATUS_ACTL_REG     (* (reg8 *) spi_oled_BSPIM_RxStsReg__STATUS_AUX_CTL_REG)
#define spi_oled_RX_STATUS_ACTL_PTR     (  (reg8 *) spi_oled_BSPIM_RxStsReg__STATUS_AUX_CTL_REG)

#if(spi_oled_USE_SECOND_DATAPATH)
    #define spi_oled_AUX_CONTROLDP1     (spi_oled_AUX_CONTROL_DP1_REG)
#endif /* (spi_oled_USE_SECOND_DATAPATH) */


/***************************************
*       Register Constants
***************************************/

/* Status Register Definitions */
#define spi_oled_STS_SPI_DONE_SHIFT             (0x00u)
#define spi_oled_STS_TX_FIFO_EMPTY_SHIFT        (0x01u)
#define spi_oled_STS_TX_FIFO_NOT_FULL_SHIFT     (0x02u)
#define spi_oled_STS_BYTE_COMPLETE_SHIFT        (0x03u)
#define spi_oled_STS_SPI_IDLE_SHIFT             (0x04u)
#define spi_oled_STS_RX_FIFO_FULL_SHIFT         (0x04u)
#define spi_oled_STS_RX_FIFO_NOT_EMPTY_SHIFT    (0x05u)
#define spi_oled_STS_RX_FIFO_OVERRUN_SHIFT      (0x06u)

#define spi_oled_STS_SPI_DONE           ((uint8) (0x01u << spi_oled_STS_SPI_DONE_SHIFT))
#define spi_oled_STS_TX_FIFO_EMPTY      ((uint8) (0x01u << spi_oled_STS_TX_FIFO_EMPTY_SHIFT))
#define spi_oled_STS_TX_FIFO_NOT_FULL   ((uint8) (0x01u << spi_oled_STS_TX_FIFO_NOT_FULL_SHIFT))
#define spi_oled_STS_BYTE_COMPLETE      ((uint8) (0x01u << spi_oled_STS_BYTE_COMPLETE_SHIFT))
#define spi_oled_STS_SPI_IDLE           ((uint8) (0x01u << spi_oled_STS_SPI_IDLE_SHIFT))
#define spi_oled_STS_RX_FIFO_FULL       ((uint8) (0x01u << spi_oled_STS_RX_FIFO_FULL_SHIFT))
#define spi_oled_STS_RX_FIFO_NOT_EMPTY  ((uint8) (0x01u << spi_oled_STS_RX_FIFO_NOT_EMPTY_SHIFT))
#define spi_oled_STS_RX_FIFO_OVERRUN    ((uint8) (0x01u << spi_oled_STS_RX_FIFO_OVERRUN_SHIFT))

/* TX and RX masks for clear on read bits */
#define spi_oled_TX_STS_CLR_ON_RD_BYTES_MASK    (0x09u)
#define spi_oled_RX_STS_CLR_ON_RD_BYTES_MASK    (0x40u)

/* StatusI Register Interrupt Enable Control Bits */
/* As defined by the Register map for the AUX Control Register */
#define spi_oled_INT_ENABLE     (0x10u) /* Enable interrupt from statusi */
#define spi_oled_TX_FIFO_CLR    (0x01u) /* F0 - TX FIFO */
#define spi_oled_RX_FIFO_CLR    (0x02u) /* F1 - RX FIFO */
#define spi_oled_FIFO_CLR       (spi_oled_TX_FIFO_CLR | spi_oled_RX_FIFO_CLR)

/* Bit Counter (7-bit) Control Register Bit Definitions */
/* As defined by the Register map for the AUX Control Register */
#define spi_oled_CNTR_ENABLE    (0x20u) /* Enable CNT7 */

/* Bi-Directional mode control bit */
#define spi_oled_CTRL_TX_SIGNAL_EN  (0x01u)

/* Datapath Auxillary Control Register definitions */
#define spi_oled_AUX_CTRL_FIFO0_CLR         (0x01u)
#define spi_oled_AUX_CTRL_FIFO1_CLR         (0x02u)
#define spi_oled_AUX_CTRL_FIFO0_LVL         (0x04u)
#define spi_oled_AUX_CTRL_FIFO1_LVL         (0x08u)
#define spi_oled_STATUS_ACTL_INT_EN_MASK    (0x10u)

/* Component disabled */
#define spi_oled_DISABLED   (0u)


/***************************************
*       Macros
***************************************/

/* Returns true if componentn enabled */
#define spi_oled_IS_ENABLED (0u != (spi_oled_TX_STATUS_ACTL_REG & spi_oled_INT_ENABLE))

/* Retuns TX status register */
#define spi_oled_GET_STATUS_TX(swTxSts) ( (uint8)(spi_oled_TX_STATUS_REG | \
                                                          ((swTxSts) & spi_oled_TX_STS_CLR_ON_RD_BYTES_MASK)) )
/* Retuns RX status register */
#define spi_oled_GET_STATUS_RX(swRxSts) ( (uint8)(spi_oled_RX_STATUS_REG | \
                                                          ((swRxSts) & spi_oled_RX_STS_CLR_ON_RD_BYTES_MASK)) )


/***************************************
* The following code is DEPRECATED and 
* should not be used in new projects.
***************************************/

#define spi_oled_WriteByte   spi_oled_WriteTxData
#define spi_oled_ReadByte    spi_oled_ReadRxData
void  spi_oled_SetInterruptMode(uint8 intSrc)       ;
uint8 spi_oled_ReadStatus(void)                     ;
void  spi_oled_EnableInt(void)                      ;
void  spi_oled_DisableInt(void)                     ;

#define spi_oled_TXDATA                 (spi_oled_TXDATA_REG)
#define spi_oled_RXDATA                 (spi_oled_RXDATA_REG)
#define spi_oled_AUX_CONTROLDP0         (spi_oled_AUX_CONTROL_DP0_REG)
#define spi_oled_TXBUFFERREAD           (spi_oled_txBufferRead)
#define spi_oled_TXBUFFERWRITE          (spi_oled_txBufferWrite)
#define spi_oled_RXBUFFERREAD           (spi_oled_rxBufferRead)
#define spi_oled_RXBUFFERWRITE          (spi_oled_rxBufferWrite)

#define spi_oled_COUNTER_PERIOD         (spi_oled_COUNTER_PERIOD_REG)
#define spi_oled_COUNTER_CONTROL        (spi_oled_COUNTER_CONTROL_REG)
#define spi_oled_STATUS                 (spi_oled_TX_STATUS_REG)
#define spi_oled_CONTROL                (spi_oled_CONTROL_REG)
#define spi_oled_STATUS_MASK            (spi_oled_TX_STATUS_MASK_REG)
#define spi_oled_STATUS_ACTL            (spi_oled_TX_STATUS_ACTL_REG)

#define spi_oled_INIT_INTERRUPTS_MASK  (spi_oled_INT_ON_SPI_DONE     | \
                                                spi_oled_INT_ON_TX_EMPTY     | \
                                                spi_oled_INT_ON_TX_NOT_FULL_DEF  | \
                                                spi_oled_INT_ON_RX_FULL      | \
                                                spi_oled_INT_ON_RX_NOT_EMPTY | \
                                                spi_oled_INT_ON_RX_OVER      | \
                                                spi_oled_INT_ON_BYTE_COMP)
                                                
#define spi_oled_DataWidth                  (spi_oled_DATA_WIDTH)
#define spi_oled_InternalClockUsed          (spi_oled_INTERNAL_CLOCK)
#define spi_oled_InternalTxInterruptEnabled (spi_oled_INTERNAL_TX_INT_ENABLED)
#define spi_oled_InternalRxInterruptEnabled (spi_oled_INTERNAL_RX_INT_ENABLED)
#define spi_oled_ModeUseZero                (spi_oled_MODE_USE_ZERO)
#define spi_oled_BidirectionalMode          (spi_oled_BIDIRECTIONAL_MODE)
#define spi_oled_Mode                       (spi_oled_MODE)
#define spi_oled_DATAWIDHT                  (spi_oled_DATA_WIDTH)
#define spi_oled_InternalInterruptEnabled   (0u)

#define spi_oled_TXBUFFERSIZE   (spi_oled_TX_BUFFER_SIZE)
#define spi_oled_RXBUFFERSIZE   (spi_oled_RX_BUFFER_SIZE)

#define spi_oled_TXBUFFER       spi_oled_txBuffer
#define spi_oled_RXBUFFER       spi_oled_rxBuffer

#endif /* (CY_SPIM_spi_oled_H) */


/* [] END OF FILE */
