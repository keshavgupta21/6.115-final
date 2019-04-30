/***************************************************************************
* File Name: dma_vga_dma.c  
* Version 1.70
*
*  Description:
*   Provides an API for the DMAC component. The API includes functions
*   for the DMA controller, DMA channels and Transfer Descriptors.
*
*
*   Note:
*     This module requires the developer to finish or fill in the auto
*     generated funcions and setup the dma channel and TD's.
*
********************************************************************************
* Copyright 2008-2010, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
********************************************************************************/
#include <CYLIB.H>
#include <CYDMAC.H>
#include <dma_vga_dma.H>



/****************************************************************************
* 
* The following defines are available in Cyfitter.h
* 
* 
* 
* dma_vga__DRQ_CTL_REG
* 
* 
* dma_vga__DRQ_NUMBER
* 
* Number of TD's used by this channel.
* dma_vga__NUMBEROF_TDS
* 
* Priority of this channel.
* dma_vga__PRIORITY
* 
* True if dma_vga_TERMIN_SEL is used.
* dma_vga__TERMIN_EN
* 
* TERMIN interrupt line to signal terminate.
* dma_vga__TERMIN_SEL
* 
* 
* True if dma_vga_TERMOUT0_SEL is used.
* dma_vga__TERMOUT0_EN
* 
* 
* TERMOUT0 interrupt line to signal completion.
* dma_vga__TERMOUT0_SEL
* 
* 
* True if dma_vga_TERMOUT1_SEL is used.
* dma_vga__TERMOUT1_EN
* 
* 
* TERMOUT1 interrupt line to signal completion.
* dma_vga__TERMOUT1_SEL
* 
****************************************************************************/


/* Zero based index of dma_vga dma channel */
uint8 dma_vga_DmaHandle = DMA_INVALID_CHANNEL;

/*********************************************************************
* Function Name: uint8 dma_vga_DmaInitalize
**********************************************************************
* Summary:
*   Allocates and initialises a channel of the DMAC to be used by the
*   caller.
*
* Parameters:
*   BurstCount.
*       
*       
*   ReqestPerBurst.
*       
*       
*   UpperSrcAddress.
*       
*       
*   UpperDestAddress.
*       
*
* Return:
*   The channel that can be used by the caller for DMA activity.
*   DMA_INVALID_CHANNEL (0xFF) if there are no channels left. 
*
*
*******************************************************************/
uint8 dma_vga_DmaInitialize(uint8 BurstCount, uint8 ReqestPerBurst, uint16 UpperSrcAddress, uint16 UpperDestAddress) 
{

    /* Allocate a DMA channel. */
    dma_vga_DmaHandle = (uint8)dma_vga__DRQ_NUMBER;

    /* Configure the channel. */
    (void)CyDmaChSetConfiguration(dma_vga_DmaHandle,
                                  BurstCount,
                                  ReqestPerBurst,
                                  (uint8)dma_vga__TERMOUT0_SEL,
                                  (uint8)dma_vga__TERMOUT1_SEL,
                                  (uint8)dma_vga__TERMIN_SEL);

    /* Set the extended address for the transfers */
    (void)CyDmaChSetExtendedAddress(dma_vga_DmaHandle, UpperSrcAddress, UpperDestAddress);

    /* Set the priority for this channel */
    (void)CyDmaChPriority(dma_vga_DmaHandle, (uint8)dma_vga__PRIORITY);
    
    return dma_vga_DmaHandle;
}

/*********************************************************************
* Function Name: void dma_vga_DmaRelease
**********************************************************************
* Summary:
*   Frees the channel associated with dma_vga.
*
*
* Parameters:
*   void.
*
*
*
* Return:
*   void.
*
*******************************************************************/
void dma_vga_DmaRelease(void) 
{
    /* Disable the channel */
    (void)CyDmaChDisable(dma_vga_DmaHandle);
}

