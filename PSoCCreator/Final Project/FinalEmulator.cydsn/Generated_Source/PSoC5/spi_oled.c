/*******************************************************************************
* File Name: spi_oled.c
* Version 2.50
*
* Description:
*  This file provides all API functionality of the SPI Master component.
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

#if(spi_oled_TX_SOFTWARE_BUF_ENABLED)
    volatile uint8 spi_oled_txBuffer[spi_oled_TX_BUFFER_SIZE];
    volatile uint8 spi_oled_txBufferFull;
    volatile uint8 spi_oled_txBufferRead;
    volatile uint8 spi_oled_txBufferWrite;
#endif /* (spi_oled_TX_SOFTWARE_BUF_ENABLED) */

#if(spi_oled_RX_SOFTWARE_BUF_ENABLED)
    volatile uint8 spi_oled_rxBuffer[spi_oled_RX_BUFFER_SIZE];
    volatile uint8 spi_oled_rxBufferFull;
    volatile uint8 spi_oled_rxBufferRead;
    volatile uint8 spi_oled_rxBufferWrite;
#endif /* (spi_oled_RX_SOFTWARE_BUF_ENABLED) */

uint8 spi_oled_initVar = 0u;

volatile uint8 spi_oled_swStatusTx;
volatile uint8 spi_oled_swStatusRx;


/*******************************************************************************
* Function Name: spi_oled_Init
********************************************************************************
*
* Summary:
*  Inits/Restores default SPIM configuration provided with customizer.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Side Effects:
*  When this function is called it initializes all of the necessary parameters
*  for execution. i.e. setting the initial interrupt mask, configuring the
*  interrupt service routine, configuring the bit-counter parameters and
*  clearing the FIFO and Status Register.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void spi_oled_Init(void) 
{
    /* Initialize the Bit counter */
    spi_oled_COUNTER_PERIOD_REG = spi_oled_BITCTR_INIT;

    /* Init TX ISR  */
    #if(0u != spi_oled_INTERNAL_TX_INT_ENABLED)
        CyIntDisable         (spi_oled_TX_ISR_NUMBER);
        CyIntSetPriority     (spi_oled_TX_ISR_NUMBER,  spi_oled_TX_ISR_PRIORITY);
        (void) CyIntSetVector(spi_oled_TX_ISR_NUMBER, &spi_oled_TX_ISR);
    #endif /* (0u != spi_oled_INTERNAL_TX_INT_ENABLED) */

    /* Init RX ISR  */
    #if(0u != spi_oled_INTERNAL_RX_INT_ENABLED)
        CyIntDisable         (spi_oled_RX_ISR_NUMBER);
        CyIntSetPriority     (spi_oled_RX_ISR_NUMBER,  spi_oled_RX_ISR_PRIORITY);
        (void) CyIntSetVector(spi_oled_RX_ISR_NUMBER, &spi_oled_RX_ISR);
    #endif /* (0u != spi_oled_INTERNAL_RX_INT_ENABLED) */

    /* Clear any stray data from the RX and TX FIFO */
    spi_oled_ClearFIFO();

    #if(spi_oled_RX_SOFTWARE_BUF_ENABLED)
        spi_oled_rxBufferFull  = 0u;
        spi_oled_rxBufferRead  = 0u;
        spi_oled_rxBufferWrite = 0u;
    #endif /* (spi_oled_RX_SOFTWARE_BUF_ENABLED) */

    #if(spi_oled_TX_SOFTWARE_BUF_ENABLED)
        spi_oled_txBufferFull  = 0u;
        spi_oled_txBufferRead  = 0u;
        spi_oled_txBufferWrite = 0u;
    #endif /* (spi_oled_TX_SOFTWARE_BUF_ENABLED) */

    (void) spi_oled_ReadTxStatus(); /* Clear Tx status and swStatusTx */
    (void) spi_oled_ReadRxStatus(); /* Clear Rx status and swStatusRx */

    /* Configure TX and RX interrupt mask */
    spi_oled_TX_STATUS_MASK_REG = spi_oled_TX_INIT_INTERRUPTS_MASK;
    spi_oled_RX_STATUS_MASK_REG = spi_oled_RX_INIT_INTERRUPTS_MASK;
}


/*******************************************************************************
* Function Name: spi_oled_Enable
********************************************************************************
*
* Summary:
*  Enable SPIM component.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
*******************************************************************************/
void spi_oled_Enable(void) 
{
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();
    spi_oled_COUNTER_CONTROL_REG |= spi_oled_CNTR_ENABLE;
    spi_oled_TX_STATUS_ACTL_REG  |= spi_oled_INT_ENABLE;
    spi_oled_RX_STATUS_ACTL_REG  |= spi_oled_INT_ENABLE;
    CyExitCriticalSection(enableInterrupts);

    #if(0u != spi_oled_INTERNAL_CLOCK)
        spi_oled_IntClock_Enable();
    #endif /* (0u != spi_oled_INTERNAL_CLOCK) */

    spi_oled_EnableTxInt();
    spi_oled_EnableRxInt();
}


/*******************************************************************************
* Function Name: spi_oled_Start
********************************************************************************
*
* Summary:
*  Initialize and Enable the SPI Master component.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Global variables:
*  spi_oled_initVar - used to check initial configuration, modified on
*  first function call.
*
* Theory:
*  Enable the clock input to enable operation.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void spi_oled_Start(void) 
{
    if(0u == spi_oled_initVar)
    {
        spi_oled_Init();
        spi_oled_initVar = 1u;
    }

    spi_oled_Enable();
}


/*******************************************************************************
* Function Name: spi_oled_Stop
********************************************************************************
*
* Summary:
*  Disable the SPI Master component.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Theory:
*  Disable the clock input to enable operation.
*
*******************************************************************************/
void spi_oled_Stop(void) 
{
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();
    spi_oled_TX_STATUS_ACTL_REG &= ((uint8) ~spi_oled_INT_ENABLE);
    spi_oled_RX_STATUS_ACTL_REG &= ((uint8) ~spi_oled_INT_ENABLE);
    CyExitCriticalSection(enableInterrupts);

    #if(0u != spi_oled_INTERNAL_CLOCK)
        spi_oled_IntClock_Disable();
    #endif /* (0u != spi_oled_INTERNAL_CLOCK) */

    spi_oled_DisableTxInt();
    spi_oled_DisableRxInt();
}


/*******************************************************************************
* Function Name: spi_oled_EnableTxInt
********************************************************************************
*
* Summary:
*  Enable internal Tx interrupt generation.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Theory:
*  Enable the internal Tx interrupt output -or- the interrupt component itself.
*
*******************************************************************************/
void spi_oled_EnableTxInt(void) 
{
    #if(0u != spi_oled_INTERNAL_TX_INT_ENABLED)
        CyIntEnable(spi_oled_TX_ISR_NUMBER);
    #endif /* (0u != spi_oled_INTERNAL_TX_INT_ENABLED) */
}


/*******************************************************************************
* Function Name: spi_oled_EnableRxInt
********************************************************************************
*
* Summary:
*  Enable internal Rx interrupt generation.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Theory:
*  Enable the internal Rx interrupt output -or- the interrupt component itself.
*
*******************************************************************************/
void spi_oled_EnableRxInt(void) 
{
    #if(0u != spi_oled_INTERNAL_RX_INT_ENABLED)
        CyIntEnable(spi_oled_RX_ISR_NUMBER);
    #endif /* (0u != spi_oled_INTERNAL_RX_INT_ENABLED) */
}


/*******************************************************************************
* Function Name: spi_oled_DisableTxInt
********************************************************************************
*
* Summary:
*  Disable internal Tx interrupt generation.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Theory:
*  Disable the internal Tx interrupt output -or- the interrupt component itself.
*
*******************************************************************************/
void spi_oled_DisableTxInt(void) 
{
    #if(0u != spi_oled_INTERNAL_TX_INT_ENABLED)
        CyIntDisable(spi_oled_TX_ISR_NUMBER);
    #endif /* (0u != spi_oled_INTERNAL_TX_INT_ENABLED) */
}


/*******************************************************************************
* Function Name: spi_oled_DisableRxInt
********************************************************************************
*
* Summary:
*  Disable internal Rx interrupt generation.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Theory:
*  Disable the internal Rx interrupt output -or- the interrupt component itself.
*
*******************************************************************************/
void spi_oled_DisableRxInt(void) 
{
    #if(0u != spi_oled_INTERNAL_RX_INT_ENABLED)
        CyIntDisable(spi_oled_RX_ISR_NUMBER);
    #endif /* (0u != spi_oled_INTERNAL_RX_INT_ENABLED) */
}


/*******************************************************************************
* Function Name: spi_oled_SetTxInterruptMode
********************************************************************************
*
* Summary:
*  Configure which status bits trigger an interrupt event.
*
* Parameters:
*  intSrc: An or'd combination of the desired status bit masks (defined in the
*  header file).
*
* Return:
*  None.
*
* Theory:
*  Enables the output of specific status bits to the interrupt controller.
*
*******************************************************************************/
void spi_oled_SetTxInterruptMode(uint8 intSrc) 
{
    spi_oled_TX_STATUS_MASK_REG = intSrc;
}


/*******************************************************************************
* Function Name: spi_oled_SetRxInterruptMode
********************************************************************************
*
* Summary:
*  Configure which status bits trigger an interrupt event.
*
* Parameters:
*  intSrc: An or'd combination of the desired status bit masks (defined in the
*  header file).
*
* Return:
*  None.
*
* Theory:
*  Enables the output of specific status bits to the interrupt controller.
*
*******************************************************************************/
void spi_oled_SetRxInterruptMode(uint8 intSrc) 
{
    spi_oled_RX_STATUS_MASK_REG  = intSrc;
}


/*******************************************************************************
* Function Name: spi_oled_ReadTxStatus
********************************************************************************
*
* Summary:
*  Read the Tx status register for the component.
*
* Parameters:
*  None.
*
* Return:
*  Contents of the Tx status register.
*
* Global variables:
*  spi_oled_swStatusTx - used to store in software status register,
*  modified every function call - resets to zero.
*
* Theory:
*  Allows the user and the API to read the Tx status register for error
*  detection and flow control.
*
* Side Effects:
*  Clear Tx status register of the component.
*
* Reentrant:
*  No.
*
*******************************************************************************/
uint8 spi_oled_ReadTxStatus(void) 
{
    uint8 tmpStatus;

    #if(spi_oled_TX_SOFTWARE_BUF_ENABLED)
        /* Disable TX interrupt to protect global veriables */
        spi_oled_DisableTxInt();

        tmpStatus = spi_oled_GET_STATUS_TX(spi_oled_swStatusTx);
        spi_oled_swStatusTx = 0u;

        spi_oled_EnableTxInt();

    #else

        tmpStatus = spi_oled_TX_STATUS_REG;

    #endif /* (spi_oled_TX_SOFTWARE_BUF_ENABLED) */

    return(tmpStatus);
}


/*******************************************************************************
* Function Name: spi_oled_ReadRxStatus
********************************************************************************
*
* Summary:
*  Read the Rx status register for the component.
*
* Parameters:
*  None.
*
* Return:
*  Contents of the Rx status register.
*
* Global variables:
*  spi_oled_swStatusRx - used to store in software Rx status register,
*  modified every function call - resets to zero.
*
* Theory:
*  Allows the user and the API to read the Rx status register for error
*  detection and flow control.
*
* Side Effects:
*  Clear Rx status register of the component.
*
* Reentrant:
*  No.
*
*******************************************************************************/
uint8 spi_oled_ReadRxStatus(void) 
{
    uint8 tmpStatus;

    #if(spi_oled_RX_SOFTWARE_BUF_ENABLED)
        /* Disable RX interrupt to protect global veriables */
        spi_oled_DisableRxInt();

        tmpStatus = spi_oled_GET_STATUS_RX(spi_oled_swStatusRx);
        spi_oled_swStatusRx = 0u;

        spi_oled_EnableRxInt();

    #else

        tmpStatus = spi_oled_RX_STATUS_REG;

    #endif /* (spi_oled_RX_SOFTWARE_BUF_ENABLED) */

    return(tmpStatus);
}


/*******************************************************************************
* Function Name: spi_oled_WriteTxData
********************************************************************************
*
* Summary:
*  Write a byte of data to be sent across the SPI.
*
* Parameters:
*  txDataByte: The data value to send across the SPI.
*
* Return:
*  None.
*
* Global variables:
*  spi_oled_txBufferWrite - used for the account of the bytes which
*  have been written down in the TX software buffer, modified every function
*  call if TX Software Buffer is used.
*  spi_oled_txBufferRead - used for the account of the bytes which
*  have been read from the TX software buffer.
*  spi_oled_txBuffer[spi_oled_TX_BUFFER_SIZE] - used to store
*  data to sending, modified every function call if TX Software Buffer is used.
*
* Theory:
*  Allows the user to transmit any byte of data in a single transfer.
*
* Side Effects:
*  If this function is called again before the previous byte is finished then
*  the next byte will be appended to the transfer with no time between
*  the byte transfers. Clear Tx status register of the component.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void spi_oled_WriteTxData(uint8 txData) 
{
    #if(spi_oled_TX_SOFTWARE_BUF_ENABLED)

        uint8 tempStatus;
        uint8 tmpTxBufferRead;

        /* Block if TX buffer is FULL: don't overwrite */
        do
        {
            tmpTxBufferRead = spi_oled_txBufferRead;
            if(0u == tmpTxBufferRead)
            {
                tmpTxBufferRead = (spi_oled_TX_BUFFER_SIZE - 1u);
            }
            else
            {
                tmpTxBufferRead--;
            }

        }while(tmpTxBufferRead == spi_oled_txBufferWrite);

        /* Disable TX interrupt to protect global veriables */
        spi_oled_DisableTxInt();

        tempStatus = spi_oled_GET_STATUS_TX(spi_oled_swStatusTx);
        spi_oled_swStatusTx = tempStatus;


        if((spi_oled_txBufferRead == spi_oled_txBufferWrite) &&
           (0u != (spi_oled_swStatusTx & spi_oled_STS_TX_FIFO_NOT_FULL)))
        {
            /* Put data element into the TX FIFO */
            CY_SET_REG8(spi_oled_TXDATA_PTR, txData);
        }
        else
        {
            /* Add to the TX software buffer */
            spi_oled_txBufferWrite++;
            if(spi_oled_txBufferWrite >= spi_oled_TX_BUFFER_SIZE)
            {
                spi_oled_txBufferWrite = 0u;
            }

            if(spi_oled_txBufferWrite == spi_oled_txBufferRead)
            {
                spi_oled_txBufferRead++;
                if(spi_oled_txBufferRead >= spi_oled_TX_BUFFER_SIZE)
                {
                    spi_oled_txBufferRead = 0u;
                }
                spi_oled_txBufferFull = 1u;
            }

            spi_oled_txBuffer[spi_oled_txBufferWrite] = txData;

            spi_oled_TX_STATUS_MASK_REG |= spi_oled_STS_TX_FIFO_NOT_FULL;
        }

        spi_oled_EnableTxInt();

    #else
        /* Wait until TX FIFO has a place */
        while(0u == (spi_oled_TX_STATUS_REG & spi_oled_STS_TX_FIFO_NOT_FULL))
        {
        }

        /* Put data element into the TX FIFO */
        CY_SET_REG8(spi_oled_TXDATA_PTR, txData);

    #endif /* (spi_oled_TX_SOFTWARE_BUF_ENABLED) */
}


/*******************************************************************************
* Function Name: spi_oled_ReadRxData
********************************************************************************
*
* Summary:
*  Read the next byte of data received across the SPI.
*
* Parameters:
*  None.
*
* Return:
*  The next byte of data read from the FIFO.
*
* Global variables:
*  spi_oled_rxBufferWrite - used for the account of the bytes which
*  have been written down in the RX software buffer.
*  spi_oled_rxBufferRead - used for the account of the bytes which
*  have been read from the RX software buffer, modified every function
*  call if RX Software Buffer is used.
*  spi_oled_rxBuffer[spi_oled_RX_BUFFER_SIZE] - used to store
*  received data.
*
* Theory:
*  Allows the user to read a byte of data received.
*
* Side Effects:
*  Will return invalid data if the FIFO is empty. The user should Call
*  GetRxBufferSize() and if it returns a non-zero value then it is safe to call
*  ReadByte() function.
*
* Reentrant:
*  No.
*
*******************************************************************************/
uint8 spi_oled_ReadRxData(void) 
{
    uint8 rxData;

    #if(spi_oled_RX_SOFTWARE_BUF_ENABLED)

        /* Disable RX interrupt to protect global veriables */
        spi_oled_DisableRxInt();

        if(spi_oled_rxBufferRead != spi_oled_rxBufferWrite)
        {
            if(0u == spi_oled_rxBufferFull)
            {
                spi_oled_rxBufferRead++;
                if(spi_oled_rxBufferRead >= spi_oled_RX_BUFFER_SIZE)
                {
                    spi_oled_rxBufferRead = 0u;
                }
            }
            else
            {
                spi_oled_rxBufferFull = 0u;
            }
        }

        rxData = spi_oled_rxBuffer[spi_oled_rxBufferRead];

        spi_oled_EnableRxInt();

    #else

        rxData = CY_GET_REG8(spi_oled_RXDATA_PTR);

    #endif /* (spi_oled_RX_SOFTWARE_BUF_ENABLED) */

    return(rxData);
}


/*******************************************************************************
* Function Name: spi_oled_GetRxBufferSize
********************************************************************************
*
* Summary:
*  Returns the number of bytes/words of data currently held in the RX buffer.
*  If RX Software Buffer not used then function return 0 if FIFO empty or 1 if
*  FIFO not empty. In another case function return size of RX Software Buffer.
*
* Parameters:
*  None.
*
* Return:
*  Integer count of the number of bytes/words in the RX buffer.
*
* Global variables:
*  spi_oled_rxBufferWrite - used for the account of the bytes which
*  have been written down in the RX software buffer.
*  spi_oled_rxBufferRead - used for the account of the bytes which
*  have been read from the RX software buffer.
*
* Side Effects:
*  Clear status register of the component.
*
*******************************************************************************/
uint8 spi_oled_GetRxBufferSize(void) 
{
    uint8 size;

    #if(spi_oled_RX_SOFTWARE_BUF_ENABLED)

        /* Disable RX interrupt to protect global veriables */
        spi_oled_DisableRxInt();

        if(spi_oled_rxBufferRead == spi_oled_rxBufferWrite)
        {
            size = 0u;
        }
        else if(spi_oled_rxBufferRead < spi_oled_rxBufferWrite)
        {
            size = (spi_oled_rxBufferWrite - spi_oled_rxBufferRead);
        }
        else
        {
            size = (spi_oled_RX_BUFFER_SIZE - spi_oled_rxBufferRead) + spi_oled_rxBufferWrite;
        }

        spi_oled_EnableRxInt();

    #else

        /* We can only know if there is data in the RX FIFO */
        size = (0u != (spi_oled_RX_STATUS_REG & spi_oled_STS_RX_FIFO_NOT_EMPTY)) ? 1u : 0u;

    #endif /* (spi_oled_TX_SOFTWARE_BUF_ENABLED) */

    return(size);
}


/*******************************************************************************
* Function Name: spi_oled_GetTxBufferSize
********************************************************************************
*
* Summary:
*  Returns the number of bytes/words of data currently held in the TX buffer.
*  If TX Software Buffer not used then function return 0 - if FIFO empty, 1 - if
*  FIFO not full, 4 - if FIFO full. In another case function return size of TX
*  Software Buffer.
*
* Parameters:
*  None.
*
* Return:
*  Integer count of the number of bytes/words in the TX buffer.
*
* Global variables:
*  spi_oled_txBufferWrite - used for the account of the bytes which
*  have been written down in the TX software buffer.
*  spi_oled_txBufferRead - used for the account of the bytes which
*  have been read from the TX software buffer.
*
* Side Effects:
*  Clear status register of the component.
*
*******************************************************************************/
uint8  spi_oled_GetTxBufferSize(void) 
{
    uint8 size;

    #if(spi_oled_TX_SOFTWARE_BUF_ENABLED)
        /* Disable TX interrupt to protect global veriables */
        spi_oled_DisableTxInt();

        if(spi_oled_txBufferRead == spi_oled_txBufferWrite)
        {
            size = 0u;
        }
        else if(spi_oled_txBufferRead < spi_oled_txBufferWrite)
        {
            size = (spi_oled_txBufferWrite - spi_oled_txBufferRead);
        }
        else
        {
            size = (spi_oled_TX_BUFFER_SIZE - spi_oled_txBufferRead) + spi_oled_txBufferWrite;
        }

        spi_oled_EnableTxInt();

    #else

        size = spi_oled_TX_STATUS_REG;

        if(0u != (size & spi_oled_STS_TX_FIFO_EMPTY))
        {
            size = 0u;
        }
        else if(0u != (size & spi_oled_STS_TX_FIFO_NOT_FULL))
        {
            size = 1u;
        }
        else
        {
            size = spi_oled_FIFO_SIZE;
        }

    #endif /* (spi_oled_TX_SOFTWARE_BUF_ENABLED) */

    return(size);
}


/*******************************************************************************
* Function Name: spi_oled_ClearRxBuffer
********************************************************************************
*
* Summary:
*  Clear the RX RAM buffer by setting the read and write pointers both to zero.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Global variables:
*  spi_oled_rxBufferWrite - used for the account of the bytes which
*  have been written down in the RX software buffer, modified every function
*  call - resets to zero.
*  spi_oled_rxBufferRead - used for the account of the bytes which
*  have been read from the RX software buffer, modified every function call -
*  resets to zero.
*
* Theory:
*  Setting the pointers to zero makes the system believe there is no data to
*  read and writing will resume at address 0 overwriting any data that may have
*  remained in the RAM.
*
* Side Effects:
*  Any received data not read from the RAM buffer will be lost when overwritten.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void spi_oled_ClearRxBuffer(void) 
{
    /* Clear Hardware RX FIFO */
    while(0u !=(spi_oled_RX_STATUS_REG & spi_oled_STS_RX_FIFO_NOT_EMPTY))
    {
        (void) CY_GET_REG8(spi_oled_RXDATA_PTR);
    }

    #if(spi_oled_RX_SOFTWARE_BUF_ENABLED)
        /* Disable RX interrupt to protect global veriables */
        spi_oled_DisableRxInt();

        spi_oled_rxBufferFull  = 0u;
        spi_oled_rxBufferRead  = 0u;
        spi_oled_rxBufferWrite = 0u;

        spi_oled_EnableRxInt();
    #endif /* (spi_oled_RX_SOFTWARE_BUF_ENABLED) */
}


/*******************************************************************************
* Function Name: spi_oled_ClearTxBuffer
********************************************************************************
*
* Summary:
*  Clear the TX RAM buffer by setting the read and write pointers both to zero.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Global variables:
*  spi_oled_txBufferWrite - used for the account of the bytes which
*  have been written down in the TX software buffer, modified every function
*  call - resets to zero.
*  spi_oled_txBufferRead - used for the account of the bytes which
*  have been read from the TX software buffer, modified every function call -
*  resets to zero.
*
* Theory:
*  Setting the pointers to zero makes the system believe there is no data to
*  read and writing will resume at address 0 overwriting any data that may have
*  remained in the RAM.
*
* Side Effects:
*  Any data not yet transmitted from the RAM buffer will be lost when
*  overwritten.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void spi_oled_ClearTxBuffer(void) 
{
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();
    /* Clear TX FIFO */
    spi_oled_AUX_CONTROL_DP0_REG |= ((uint8)  spi_oled_TX_FIFO_CLR);
    spi_oled_AUX_CONTROL_DP0_REG &= ((uint8) ~spi_oled_TX_FIFO_CLR);

    #if(spi_oled_USE_SECOND_DATAPATH)
        /* Clear TX FIFO for 2nd Datapath */
        spi_oled_AUX_CONTROL_DP1_REG |= ((uint8)  spi_oled_TX_FIFO_CLR);
        spi_oled_AUX_CONTROL_DP1_REG &= ((uint8) ~spi_oled_TX_FIFO_CLR);
    #endif /* (spi_oled_USE_SECOND_DATAPATH) */
    CyExitCriticalSection(enableInterrupts);

    #if(spi_oled_TX_SOFTWARE_BUF_ENABLED)
        /* Disable TX interrupt to protect global veriables */
        spi_oled_DisableTxInt();

        spi_oled_txBufferFull  = 0u;
        spi_oled_txBufferRead  = 0u;
        spi_oled_txBufferWrite = 0u;

        /* Buffer is EMPTY: disable TX FIFO NOT FULL interrupt */
        spi_oled_TX_STATUS_MASK_REG &= ((uint8) ~spi_oled_STS_TX_FIFO_NOT_FULL);

        spi_oled_EnableTxInt();
    #endif /* (spi_oled_TX_SOFTWARE_BUF_ENABLED) */
}


#if(0u != spi_oled_BIDIRECTIONAL_MODE)
    /*******************************************************************************
    * Function Name: spi_oled_TxEnable
    ********************************************************************************
    *
    * Summary:
    *  If the SPI master is configured to use a single bi-directional pin then this
    *  will set the bi-directional pin to transmit.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  None.
    *
    *******************************************************************************/
    void spi_oled_TxEnable(void) 
    {
        spi_oled_CONTROL_REG |= spi_oled_CTRL_TX_SIGNAL_EN;
    }


    /*******************************************************************************
    * Function Name: spi_oled_TxDisable
    ********************************************************************************
    *
    * Summary:
    *  If the SPI master is configured to use a single bi-directional pin then this
    *  will set the bi-directional pin to receive.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  None.
    *
    *******************************************************************************/
    void spi_oled_TxDisable(void) 
    {
        spi_oled_CONTROL_REG &= ((uint8) ~spi_oled_CTRL_TX_SIGNAL_EN);
    }

#endif /* (0u != spi_oled_BIDIRECTIONAL_MODE) */


/*******************************************************************************
* Function Name: spi_oled_PutArray
********************************************************************************
*
* Summary:
*  Write available data from ROM/RAM to the TX buffer while space is available
*  in the TX buffer. Keep trying until all data is passed to the TX buffer.
*
* Parameters:
*  *buffer: Pointer to the location in RAM containing the data to send
*  byteCount: The number of bytes to move to the transmit buffer.
*
* Return:
*  None.
*
* Side Effects:
*  Will stay in this routine until all data has been sent.  May get locked in
*  this loop if data is not being initiated by the master if there is not
*  enough room in the TX FIFO.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void spi_oled_PutArray(const uint8 buffer[], uint8 byteCount)
                                                                          
{
    uint8 bufIndex;

    bufIndex = 0u;

    while(byteCount > 0u)
    {
        spi_oled_WriteTxData(buffer[bufIndex]);
        bufIndex++;
        byteCount--;
    }
}


/*******************************************************************************
* Function Name: spi_oled_ClearFIFO
********************************************************************************
*
* Summary:
*  Clear the RX and TX FIFO's of all data for a fresh start.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Side Effects:
*  Clear status register of the component.
*
*******************************************************************************/
void spi_oled_ClearFIFO(void) 
{
    uint8 enableInterrupts;

    /* Clear Hardware RX FIFO */
    while(0u !=(spi_oled_RX_STATUS_REG & spi_oled_STS_RX_FIFO_NOT_EMPTY))
    {
        (void) CY_GET_REG8(spi_oled_RXDATA_PTR);
    }

    enableInterrupts = CyEnterCriticalSection();
    /* Clear TX FIFO */
    spi_oled_AUX_CONTROL_DP0_REG |= ((uint8)  spi_oled_TX_FIFO_CLR);
    spi_oled_AUX_CONTROL_DP0_REG &= ((uint8) ~spi_oled_TX_FIFO_CLR);

    #if(spi_oled_USE_SECOND_DATAPATH)
        /* Clear TX FIFO for 2nd Datapath */
        spi_oled_AUX_CONTROL_DP1_REG |= ((uint8)  spi_oled_TX_FIFO_CLR);
        spi_oled_AUX_CONTROL_DP1_REG &= ((uint8) ~spi_oled_TX_FIFO_CLR);
    #endif /* (spi_oled_USE_SECOND_DATAPATH) */
    CyExitCriticalSection(enableInterrupts);
}


/* Following functions are for version Compatibility, they are obsolete.
*  Please do not use it in new projects.
*/


/*******************************************************************************
* Function Name: spi_oled_EnableInt
********************************************************************************
*
* Summary:
*  Enable internal interrupt generation.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Theory:
*  Enable the internal interrupt output -or- the interrupt component itself.
*
*******************************************************************************/
void spi_oled_EnableInt(void) 
{
    spi_oled_EnableRxInt();
    spi_oled_EnableTxInt();
}


/*******************************************************************************
* Function Name: spi_oled_DisableInt
********************************************************************************
*
* Summary:
*  Disable internal interrupt generation.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Theory:
*  Disable the internal interrupt output -or- the interrupt component itself.
*
*******************************************************************************/
void spi_oled_DisableInt(void) 
{
    spi_oled_DisableTxInt();
    spi_oled_DisableRxInt();
}


/*******************************************************************************
* Function Name: spi_oled_SetInterruptMode
********************************************************************************
*
* Summary:
*  Configure which status bits trigger an interrupt event.
*
* Parameters:
*  intSrc: An or'd combination of the desired status bit masks (defined in the
*  header file).
*
* Return:
*  None.
*
* Theory:
*  Enables the output of specific status bits to the interrupt controller.
*
*******************************************************************************/
void spi_oled_SetInterruptMode(uint8 intSrc) 
{
    spi_oled_TX_STATUS_MASK_REG  = (intSrc & ((uint8) ~spi_oled_STS_SPI_IDLE));
    spi_oled_RX_STATUS_MASK_REG  =  intSrc;
}


/*******************************************************************************
* Function Name: spi_oled_ReadStatus
********************************************************************************
*
* Summary:
*  Read the status register for the component.
*
* Parameters:
*  None.
*
* Return:
*  Contents of the status register.
*
* Global variables:
*  spi_oled_swStatus - used to store in software status register,
*  modified every function call - resets to zero.
*
* Theory:
*  Allows the user and the API to read the status register for error detection
*  and flow control.
*
* Side Effects:
*  Clear status register of the component.
*
* Reentrant:
*  No.
*
*******************************************************************************/
uint8 spi_oled_ReadStatus(void) 
{
    uint8 tmpStatus;

    #if(spi_oled_TX_SOFTWARE_BUF_ENABLED || spi_oled_RX_SOFTWARE_BUF_ENABLED)

        spi_oled_DisableInt();

        tmpStatus  = spi_oled_GET_STATUS_RX(spi_oled_swStatusRx);
        tmpStatus |= spi_oled_GET_STATUS_TX(spi_oled_swStatusTx);
        tmpStatus &= ((uint8) ~spi_oled_STS_SPI_IDLE);

        spi_oled_swStatusTx = 0u;
        spi_oled_swStatusRx = 0u;

        spi_oled_EnableInt();

    #else

        tmpStatus  = spi_oled_RX_STATUS_REG;
        tmpStatus |= spi_oled_TX_STATUS_REG;
        tmpStatus &= ((uint8) ~spi_oled_STS_SPI_IDLE);

    #endif /* (spi_oled_TX_SOFTWARE_BUF_ENABLED || spi_oled_RX_SOFTWARE_BUF_ENABLED) */

    return(tmpStatus);
}


/* [] END OF FILE */
