/*******************************************************************************
* File Name: spi_oled_PM.c
* Version 2.50
*
* Description:
*  This file contains the setup, control and status commands to support
*  component operations in low power mode.
*
* Note:
*
********************************************************************************
* Copyright 2008-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "spi_oled_PVT.h"

static spi_oled_BACKUP_STRUCT spi_oled_backup =
{
    spi_oled_DISABLED,
    spi_oled_BITCTR_INIT,
};


/*******************************************************************************
* Function Name: spi_oled_SaveConfig
********************************************************************************
*
* Summary:
*  Empty function. Included for consistency with other components.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
*******************************************************************************/
void spi_oled_SaveConfig(void) 
{

}


/*******************************************************************************
* Function Name: spi_oled_RestoreConfig
********************************************************************************
*
* Summary:
*  Empty function. Included for consistency with other components.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
*******************************************************************************/
void spi_oled_RestoreConfig(void) 
{

}


/*******************************************************************************
* Function Name: spi_oled_Sleep
********************************************************************************
*
* Summary:
*  Prepare SPIM Component goes to sleep.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Global Variables:
*  spi_oled_backup - modified when non-retention registers are saved.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void spi_oled_Sleep(void) 
{
    /* Save components enable state */
    spi_oled_backup.enableState = ((uint8) spi_oled_IS_ENABLED);

    spi_oled_Stop();
}


/*******************************************************************************
* Function Name: spi_oled_Wakeup
********************************************************************************
*
* Summary:
*  Prepare SPIM Component to wake up.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Global Variables:
*  spi_oled_backup - used when non-retention registers are restored.
*  spi_oled_txBufferWrite - modified every function call - resets to
*  zero.
*  spi_oled_txBufferRead - modified every function call - resets to
*  zero.
*  spi_oled_rxBufferWrite - modified every function call - resets to
*  zero.
*  spi_oled_rxBufferRead - modified every function call - resets to
*  zero.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void spi_oled_Wakeup(void) 
{
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

    /* Clear any data from the RX and TX FIFO */
    spi_oled_ClearFIFO();

    /* Restore components block enable state */
    if(0u != spi_oled_backup.enableState)
    {
        spi_oled_Enable();
    }
}


/* [] END OF FILE */
