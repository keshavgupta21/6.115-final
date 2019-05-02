/*******************************************************************************
* File Name: spi_oled_INT.c
* Version 2.50
*
* Description:
*  This file provides all Interrupt Service Routine (ISR) for the SPI Master
*  component.
*
* Note:
*  None.
*
********************************************************************************
* Copyright 2008-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "spi_oled_PVT.h"
#include "cyapicallbacks.h"

/* User code required at start of ISR */
/* `#START spi_oled_ISR_START_DEF` */

/* `#END` */


/*******************************************************************************
* Function Name: spi_oled_TX_ISR
********************************************************************************
*
* Summary:
*  Interrupt Service Routine for TX portion of the SPI Master.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Global variables:
*  spi_oled_txBufferWrite - used for the account of the bytes which
*  have been written down in the TX software buffer.
*  spi_oled_txBufferRead - used for the account of the bytes which
*  have been read from the TX software buffer, modified when exist data to
*  sending and FIFO Not Full.
*  spi_oled_txBuffer[spi_oled_TX_BUFFER_SIZE] - used to store
*  data to sending.
*  All described above Global variables are used when Software Buffer is used.
*
*******************************************************************************/
CY_ISR(spi_oled_TX_ISR)
{
    #if(spi_oled_TX_SOFTWARE_BUF_ENABLED)
        uint8 tmpStatus;
    #endif /* (spi_oled_TX_SOFTWARE_BUF_ENABLED) */

    #ifdef spi_oled_TX_ISR_ENTRY_CALLBACK
        spi_oled_TX_ISR_EntryCallback();
    #endif /* spi_oled_TX_ISR_ENTRY_CALLBACK */

    /* User code required at start of ISR */
    /* `#START spi_oled_TX_ISR_START` */

    /* `#END` */
    
    #if(spi_oled_TX_SOFTWARE_BUF_ENABLED)
        /* Check if TX data buffer is not empty and there is space in TX FIFO */
        while(spi_oled_txBufferRead != spi_oled_txBufferWrite)
        {
            tmpStatus = spi_oled_GET_STATUS_TX(spi_oled_swStatusTx);
            spi_oled_swStatusTx = tmpStatus;

            if(0u != (spi_oled_swStatusTx & spi_oled_STS_TX_FIFO_NOT_FULL))
            {
                if(0u == spi_oled_txBufferFull)
                {
                   spi_oled_txBufferRead++;

                    if(spi_oled_txBufferRead >= spi_oled_TX_BUFFER_SIZE)
                    {
                        spi_oled_txBufferRead = 0u;
                    }
                }
                else
                {
                    spi_oled_txBufferFull = 0u;
                }

                /* Put data element into the TX FIFO */
                CY_SET_REG8(spi_oled_TXDATA_PTR, 
                                             spi_oled_txBuffer[spi_oled_txBufferRead]);
            }
            else
            {
                break;
            }
        }

        if(spi_oled_txBufferRead == spi_oled_txBufferWrite)
        {
            /* TX Buffer is EMPTY: disable interrupt on TX NOT FULL */
            spi_oled_TX_STATUS_MASK_REG &= ((uint8) ~spi_oled_STS_TX_FIFO_NOT_FULL);
        }

    #endif /* (spi_oled_TX_SOFTWARE_BUF_ENABLED) */

    /* User code required at end of ISR (Optional) */
    /* `#START spi_oled_TX_ISR_END` */

    /* `#END` */
    
    #ifdef spi_oled_TX_ISR_EXIT_CALLBACK
        spi_oled_TX_ISR_ExitCallback();
    #endif /* spi_oled_TX_ISR_EXIT_CALLBACK */
}


/*******************************************************************************
* Function Name: spi_oled_RX_ISR
********************************************************************************
*
* Summary:
*  Interrupt Service Routine for RX portion of the SPI Master.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Global variables:
*  spi_oled_rxBufferWrite - used for the account of the bytes which
*  have been written down in the RX software buffer modified when FIFO contains
*  new data.
*  spi_oled_rxBufferRead - used for the account of the bytes which
*  have been read from the RX software buffer, modified when overflow occurred.
*  spi_oled_rxBuffer[spi_oled_RX_BUFFER_SIZE] - used to store
*  received data, modified when FIFO contains new data.
*  All described above Global variables are used when Software Buffer is used.
*
*******************************************************************************/
CY_ISR(spi_oled_RX_ISR)
{
    #if(spi_oled_RX_SOFTWARE_BUF_ENABLED)
        uint8 tmpStatus;
        uint8 rxData;
    #endif /* (spi_oled_RX_SOFTWARE_BUF_ENABLED) */

    #ifdef spi_oled_RX_ISR_ENTRY_CALLBACK
        spi_oled_RX_ISR_EntryCallback();
    #endif /* spi_oled_RX_ISR_ENTRY_CALLBACK */

    /* User code required at start of ISR */
    /* `#START spi_oled_RX_ISR_START` */

    /* `#END` */
    
    #if(spi_oled_RX_SOFTWARE_BUF_ENABLED)

        tmpStatus = spi_oled_GET_STATUS_RX(spi_oled_swStatusRx);
        spi_oled_swStatusRx = tmpStatus;

        /* Check if RX data FIFO has some data to be moved into the RX Buffer */
        while(0u != (spi_oled_swStatusRx & spi_oled_STS_RX_FIFO_NOT_EMPTY))
        {
            rxData = CY_GET_REG8(spi_oled_RXDATA_PTR);

            /* Set next pointer. */
            spi_oled_rxBufferWrite++;
            if(spi_oled_rxBufferWrite >= spi_oled_RX_BUFFER_SIZE)
            {
                spi_oled_rxBufferWrite = 0u;
            }

            if(spi_oled_rxBufferWrite == spi_oled_rxBufferRead)
            {
                spi_oled_rxBufferRead++;
                if(spi_oled_rxBufferRead >= spi_oled_RX_BUFFER_SIZE)
                {
                    spi_oled_rxBufferRead = 0u;
                }

                spi_oled_rxBufferFull = 1u;
            }

            /* Move data from the FIFO to the Buffer */
            spi_oled_rxBuffer[spi_oled_rxBufferWrite] = rxData;

            tmpStatus = spi_oled_GET_STATUS_RX(spi_oled_swStatusRx);
            spi_oled_swStatusRx = tmpStatus;
        }

    #endif /* (spi_oled_RX_SOFTWARE_BUF_ENABLED) */

    /* User code required at end of ISR (Optional) */
    /* `#START spi_oled_RX_ISR_END` */

    /* `#END` */
    
    #ifdef spi_oled_RX_ISR_EXIT_CALLBACK
        spi_oled_RX_ISR_ExitCallback();
    #endif /* spi_oled_RX_ISR_EXIT_CALLBACK */
}

/* [] END OF FILE */
