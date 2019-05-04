/***************************************************************************//**
* \file usb_uart_drv.c
* \version 3.20
*
* \brief
*  This file contains the Endpoint 0 Driver for the USBFS Component.  
*
********************************************************************************
* \copyright
* Copyright 2008-2016, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "usb_uart_pvt.h"
#include "cyapicallbacks.h"


/***************************************
* Global data allocation
***************************************/

volatile T_usb_uart_EP_CTL_BLOCK usb_uart_EP[usb_uart_MAX_EP];

/** Contains the current configuration number, which is set by the host using a 
 * SET_CONFIGURATION request. This variable is initialized to zero in 
 * USBFS_InitComponent() API and can be read by the USBFS_GetConfiguration() 
 * API.*/
volatile uint8 usb_uart_configuration;

/** Contains the current interface number.*/
volatile uint8 usb_uart_interfaceNumber;

/** This variable is set to one after SET_CONFIGURATION and SET_INTERFACE 
 *requests. It can be read by the USBFS_IsConfigurationChanged() API */
volatile uint8 usb_uart_configurationChanged;

/** Contains the current device address.*/
volatile uint8 usb_uart_deviceAddress;

/** This is a two-bit variable that contains power status in the bit 0 
 * (DEVICE_STATUS_BUS_POWERED or DEVICE_STATUS_SELF_POWERED) and remote wakeup 
 * status (DEVICE_STATUS_REMOTE_WAKEUP) in the bit 1. This variable is 
 * initialized to zero in USBFS_InitComponent() API, configured by the 
 * USBFS_SetPowerStatus() API. The remote wakeup status cannot be set using the 
 * API SetPowerStatus(). */
volatile uint8 usb_uart_deviceStatus;

volatile uint8 usb_uart_interfaceSetting[usb_uart_MAX_INTERFACES_NUMBER];
volatile uint8 usb_uart_interfaceSetting_last[usb_uart_MAX_INTERFACES_NUMBER];
volatile uint8 usb_uart_interfaceStatus[usb_uart_MAX_INTERFACES_NUMBER];

/** Contains the started device number. This variable is set by the 
 * USBFS_Start() or USBFS_InitComponent() APIs.*/
volatile uint8 usb_uart_device;

/** Initialized class array for each interface. It is used for handling Class 
 * specific requests depend on interface class. Different classes in multiple 
 * alternate settings are not supported.*/
const uint8 CYCODE *usb_uart_interfaceClass;


/***************************************
* Local data allocation
***************************************/

volatile uint8  usb_uart_ep0Toggle;
volatile uint8  usb_uart_lastPacketSize;

/** This variable is used by the communication functions to handle the current 
* transfer state.
* Initialized to TRANS_STATE_IDLE in the USBFS_InitComponent() API and after a 
* complete transfer in the status stage.
* Changed to the TRANS_STATE_CONTROL_READ or TRANS_STATE_CONTROL_WRITE in setup 
* transaction depending on the request type.
*/
volatile uint8  usb_uart_transferState;
volatile T_usb_uart_TD usb_uart_currentTD;
volatile uint8  usb_uart_ep0Mode;
volatile uint8  usb_uart_ep0Count;
volatile uint16 usb_uart_transferByteCount;


/*******************************************************************************
* Function Name: usb_uart_ep_0_Interrupt
****************************************************************************//**
*
*  This Interrupt Service Routine handles Endpoint 0 (Control Pipe) traffic.
*  It dispatches setup requests and handles the data and status stages.
*
*
*******************************************************************************/
CY_ISR(usb_uart_EP_0_ISR)
{
    uint8 tempReg;
    uint8 modifyReg;

#ifdef usb_uart_EP_0_ISR_ENTRY_CALLBACK
    usb_uart_EP_0_ISR_EntryCallback();
#endif /* (usb_uart_EP_0_ISR_ENTRY_CALLBACK) */
    
    tempReg = usb_uart_EP0_CR_REG;
    if ((tempReg & usb_uart_MODE_ACKD) != 0u)
    {
        modifyReg = 1u;
        if ((tempReg & usb_uart_MODE_SETUP_RCVD) != 0u)
        {
            if ((tempReg & usb_uart_MODE_MASK) != usb_uart_MODE_NAK_IN_OUT)
            {
                /* Mode not equal to NAK_IN_OUT: invalid setup */
                modifyReg = 0u;
            }
            else
            {
                usb_uart_HandleSetup();
                
                if ((usb_uart_ep0Mode & usb_uart_MODE_SETUP_RCVD) != 0u)
                {
                    /* SETUP bit set: exit without mode modificaiton */
                    modifyReg = 0u;
                }
            }
        }
        else if ((tempReg & usb_uart_MODE_IN_RCVD) != 0u)
        {
            usb_uart_HandleIN();
        }
        else if ((tempReg & usb_uart_MODE_OUT_RCVD) != 0u)
        {
            usb_uart_HandleOUT();
        }
        else
        {
            modifyReg = 0u;
        }
        
        /* Modify the EP0_CR register */
        if (modifyReg != 0u)
        {
            
            tempReg = usb_uart_EP0_CR_REG;
            
            /* Make sure that SETUP bit is cleared before modification */
            if ((tempReg & usb_uart_MODE_SETUP_RCVD) == 0u)
            {
                /* Update count register */
                tempReg = (uint8) usb_uart_ep0Toggle | usb_uart_ep0Count;
                usb_uart_EP0_CNT_REG = tempReg;
               
                /* Make sure that previous write operaiton was successful */
                if (tempReg == usb_uart_EP0_CNT_REG)
                {
                    /* Repeat until next successful write operation */
                    do
                    {
                        /* Init temporary variable */
                        modifyReg = usb_uart_ep0Mode;
                        
                        /* Unlock register */
                        tempReg = (uint8) (usb_uart_EP0_CR_REG & usb_uart_MODE_SETUP_RCVD);
                        
                        /* Check if SETUP bit is not set */
                        if (0u == tempReg)
                        {
                            /* Set the Mode Register  */
                            usb_uart_EP0_CR_REG = usb_uart_ep0Mode;
                            
                            /* Writing check */
                            modifyReg = usb_uart_EP0_CR_REG & usb_uart_MODE_MASK;
                        }
                    }
                    while (modifyReg != usb_uart_ep0Mode);
                }
            }
        }
    }

    usb_uart_ClearSieInterruptSource(usb_uart_INTR_SIE_EP0_INTR);
	
#ifdef usb_uart_EP_0_ISR_EXIT_CALLBACK
    usb_uart_EP_0_ISR_ExitCallback();
#endif /* (usb_uart_EP_0_ISR_EXIT_CALLBACK) */
}


/*******************************************************************************
* Function Name: usb_uart_HandleSetup
****************************************************************************//**
*
*  This Routine dispatches requests for the four USB request types
*
*
* \reentrant
*  No.
*
*******************************************************************************/
void usb_uart_HandleSetup(void) 
{
    uint8 requestHandled;
    
    /* Clear register lock by SIE (read register) and clear setup bit 
    * (write any value in register).
    */
    requestHandled = (uint8) usb_uart_EP0_CR_REG;
    usb_uart_EP0_CR_REG = (uint8) requestHandled;
    requestHandled = (uint8) usb_uart_EP0_CR_REG;

    if ((requestHandled & usb_uart_MODE_SETUP_RCVD) != 0u)
    {
        /* SETUP bit is set: exit without mode modification. */
        usb_uart_ep0Mode = requestHandled;
    }
    else
    {
        /* In case the previous transfer did not complete, close it out */
        usb_uart_UpdateStatusBlock(usb_uart_XFER_PREMATURE);

        /* Check request type. */
        switch (usb_uart_bmRequestTypeReg & usb_uart_RQST_TYPE_MASK)
        {
            case usb_uart_RQST_TYPE_STD:
                requestHandled = usb_uart_HandleStandardRqst();
                break;
                
            case usb_uart_RQST_TYPE_CLS:
                requestHandled = usb_uart_DispatchClassRqst();
                break;
                
            case usb_uart_RQST_TYPE_VND:
                requestHandled = usb_uart_HandleVendorRqst();
                break;
                
            default:
                requestHandled = usb_uart_FALSE;
                break;
        }
        
        /* If request is not recognized. Stall endpoint 0 IN and OUT. */
        if (requestHandled == usb_uart_FALSE)
        {
            usb_uart_ep0Mode = usb_uart_MODE_STALL_IN_OUT;
        }
    }
}


/*******************************************************************************
* Function Name: usb_uart_HandleIN
****************************************************************************//**
*
*  This routine handles EP0 IN transfers.
*
*
* \reentrant
*  No.
*
*******************************************************************************/
void usb_uart_HandleIN(void) 
{
    switch (usb_uart_transferState)
    {
        case usb_uart_TRANS_STATE_IDLE:
            break;
        
        case usb_uart_TRANS_STATE_CONTROL_READ:
            usb_uart_ControlReadDataStage();
            break;
            
        case usb_uart_TRANS_STATE_CONTROL_WRITE:
            usb_uart_ControlWriteStatusStage();
            break;
            
        case usb_uart_TRANS_STATE_NO_DATA_CONTROL:
            usb_uart_NoDataControlStatusStage();
            break;
            
        default:    /* there are no more states */
            break;
    }
}


/*******************************************************************************
* Function Name: usb_uart_HandleOUT
****************************************************************************//**
*
*  This routine handles EP0 OUT transfers.
*
*
* \reentrant
*  No.
*
*******************************************************************************/
void usb_uart_HandleOUT(void) 
{
    switch (usb_uart_transferState)
    {
        case usb_uart_TRANS_STATE_IDLE:
            break;
        
        case usb_uart_TRANS_STATE_CONTROL_READ:
            usb_uart_ControlReadStatusStage();
            break;
            
        case usb_uart_TRANS_STATE_CONTROL_WRITE:
            usb_uart_ControlWriteDataStage();
            break;
            
        case usb_uart_TRANS_STATE_NO_DATA_CONTROL:
            /* Update the completion block */
            usb_uart_UpdateStatusBlock(usb_uart_XFER_ERROR);
            
            /* We expect no more data, so stall INs and OUTs */
            usb_uart_ep0Mode = usb_uart_MODE_STALL_IN_OUT;
            break;
            
        default:    
            /* There are no more states */
            break;
    }
}


/*******************************************************************************
* Function Name: usb_uart_LoadEP0
****************************************************************************//**
*
*  This routine loads the EP0 data registers for OUT transfers. It uses the
*  currentTD (previously initialized by the _InitControlWrite function and
*  updated for each OUT transfer, and the bLastPacketSize) to determine how
*  many uint8s to transfer on the current OUT.
*
*  If the number of uint8s remaining is zero and the last transfer was full,
*  we need to send a zero length packet.  Otherwise we send the minimum
*  of the control endpoint size (8) or remaining number of uint8s for the
*  transaction.
*
*
* \globalvars
*  usb_uart_transferByteCount - Update the transfer byte count from the
*     last transaction.
*  usb_uart_ep0Count - counts the data loaded to the SIE memory in
*     current packet.
*  usb_uart_lastPacketSize - remembers the USBFS_ep0Count value for the
*     next packet.
*  usb_uart_transferByteCount - sum of the previous bytes transferred
*     on previous packets(sum of USBFS_lastPacketSize)
*  usb_uart_ep0Toggle - inverted
*  usb_uart_ep0Mode  - prepare for mode register content.
*  usb_uart_transferState - set to TRANS_STATE_CONTROL_READ
*
* \reentrant
*  No.
*
*******************************************************************************/
void usb_uart_LoadEP0(void) 
{
    uint8 ep0Count = 0u;

    /* Update the transfer byte count from the last transaction */
    usb_uart_transferByteCount += usb_uart_lastPacketSize;

    /* Now load the next transaction */
    while ((usb_uart_currentTD.count > 0u) && (ep0Count < 8u))
    {
        usb_uart_EP0_DR_BASE.epData[ep0Count] = (uint8) *usb_uart_currentTD.pData;
        usb_uart_currentTD.pData = &usb_uart_currentTD.pData[1u];
        ep0Count++;
        usb_uart_currentTD.count--;
    }

    /* Support zero-length packet */
    if ((usb_uart_lastPacketSize == 8u) || (ep0Count > 0u))
    {
        /* Update the data toggle */
        usb_uart_ep0Toggle ^= usb_uart_EP0_CNT_DATA_TOGGLE;
        /* Set the Mode Register  */
        usb_uart_ep0Mode = usb_uart_MODE_ACK_IN_STATUS_OUT;
        /* Update the state (or stay the same) */
        usb_uart_transferState = usb_uart_TRANS_STATE_CONTROL_READ;
    }
    else
    {
        /* Expect Status Stage Out */
        usb_uart_ep0Mode = usb_uart_MODE_STATUS_OUT_ONLY;
        /* Update the state (or stay the same) */
        usb_uart_transferState = usb_uart_TRANS_STATE_CONTROL_READ;
    }

    /* Save the packet size for next time */
    usb_uart_ep0Count =       (uint8) ep0Count;
    usb_uart_lastPacketSize = (uint8) ep0Count;
}


/*******************************************************************************
* Function Name: usb_uart_InitControlRead
****************************************************************************//**
*
*  Initialize a control read transaction. It is used to send data to the host.
*  The following global variables should be initialized before this function
*  called. To send zero length packet use InitZeroLengthControlTransfer
*  function.
*
*
* \return
*  requestHandled state.
*
* \globalvars
*  usb_uart_currentTD.count - counts of data to be sent.
*  usb_uart_currentTD.pData - data pointer.
*
* \reentrant
*  No.
*
*******************************************************************************/
uint8 usb_uart_InitControlRead(void) 
{
    uint16 xferCount;

    if (usb_uart_currentTD.count == 0u)
    {
        (void) usb_uart_InitZeroLengthControlTransfer();
    }
    else
    {
        /* Set up the state machine */
        usb_uart_transferState = usb_uart_TRANS_STATE_CONTROL_READ;
        
        /* Set the toggle, it gets updated in LoadEP */
        usb_uart_ep0Toggle = 0u;
        
        /* Initialize the Status Block */
        usb_uart_InitializeStatusBlock();
        
        xferCount = ((uint16)((uint16) usb_uart_lengthHiReg << 8u) | ((uint16) usb_uart_lengthLoReg));

        if (usb_uart_currentTD.count > xferCount)
        {
            usb_uart_currentTD.count = xferCount;
        }
        
        usb_uart_LoadEP0();
    }

    return (usb_uart_TRUE);
}


/*******************************************************************************
* Function Name: usb_uart_InitZeroLengthControlTransfer
****************************************************************************//**
*
*  Initialize a zero length data IN transfer.
*
* \return
*  requestHandled state.
*
* \globalvars
*  usb_uart_ep0Toggle - set to EP0_CNT_DATA_TOGGLE
*  usb_uart_ep0Mode  - prepare for mode register content.
*  usb_uart_transferState - set to TRANS_STATE_CONTROL_READ
*  usb_uart_ep0Count - cleared, means the zero-length packet.
*  usb_uart_lastPacketSize - cleared.
*
* \reentrant
*  No.
*
*******************************************************************************/
uint8 usb_uart_InitZeroLengthControlTransfer(void)
                                                
{
    /* Update the state */
    usb_uart_transferState = usb_uart_TRANS_STATE_CONTROL_READ;
    
    /* Set the data toggle */
    usb_uart_ep0Toggle = usb_uart_EP0_CNT_DATA_TOGGLE;
    
    /* Set the Mode Register  */
    usb_uart_ep0Mode = usb_uart_MODE_ACK_IN_STATUS_OUT;
    
    /* Save the packet size for next time */
    usb_uart_lastPacketSize = 0u;
    
    usb_uart_ep0Count = 0u;

    return (usb_uart_TRUE);
}


/*******************************************************************************
* Function Name: usb_uart_ControlReadDataStage
****************************************************************************//**
*
*  Handle the Data Stage of a control read transfer.
*
*
* \reentrant
*  No.
*
*******************************************************************************/
void usb_uart_ControlReadDataStage(void) 

{
    usb_uart_LoadEP0();
}


/*******************************************************************************
* Function Name: usb_uart_ControlReadStatusStage
****************************************************************************//**
*
*  Handle the Status Stage of a control read transfer.
*
*
* \globalvars
*  usb_uart_USBFS_transferByteCount - updated with last packet size.
*  usb_uart_transferState - set to TRANS_STATE_IDLE.
*  usb_uart_ep0Mode  - set to MODE_STALL_IN_OUT.
*
* \reentrant
*  No.
*
*******************************************************************************/
void usb_uart_ControlReadStatusStage(void) 
{
    /* Update the transfer byte count */
    usb_uart_transferByteCount += usb_uart_lastPacketSize;
    
    /* Go Idle */
    usb_uart_transferState = usb_uart_TRANS_STATE_IDLE;
    
    /* Update the completion block */
    usb_uart_UpdateStatusBlock(usb_uart_XFER_STATUS_ACK);
    
    /* We expect no more data, so stall INs and OUTs */
    usb_uart_ep0Mode = usb_uart_MODE_STALL_IN_OUT;
}


/*******************************************************************************
* Function Name: usb_uart_InitControlWrite
****************************************************************************//**
*
*  Initialize a control write transaction
*
* \return
*  requestHandled state.
*
* \globalvars
*  usb_uart_USBFS_transferState - set to TRANS_STATE_CONTROL_WRITE
*  usb_uart_ep0Toggle - set to EP0_CNT_DATA_TOGGLE
*  usb_uart_ep0Mode  - set to MODE_ACK_OUT_STATUS_IN
*
* \reentrant
*  No.
*
*******************************************************************************/
uint8 usb_uart_InitControlWrite(void) 
{
    uint16 xferCount;

    /* Set up the state machine */
    usb_uart_transferState = usb_uart_TRANS_STATE_CONTROL_WRITE;
    
    /* This might not be necessary */
    usb_uart_ep0Toggle = usb_uart_EP0_CNT_DATA_TOGGLE;
    
    /* Initialize the Status Block */
    usb_uart_InitializeStatusBlock();

    xferCount = ((uint16)((uint16) usb_uart_lengthHiReg << 8u) | ((uint16) usb_uart_lengthLoReg));

    if (usb_uart_currentTD.count > xferCount)
    {
        usb_uart_currentTD.count = xferCount;
    }

    /* Expect Data or Status Stage */
    usb_uart_ep0Mode = usb_uart_MODE_ACK_OUT_STATUS_IN;

    return(usb_uart_TRUE);
}


/*******************************************************************************
* Function Name: usb_uart_ControlWriteDataStage
****************************************************************************//**
*
*  Handle the Data Stage of a control write transfer
*       1. Get the data (We assume the destination was validated previously)
*       2. Update the count and data toggle
*       3. Update the mode register for the next transaction
*
*
* \globalvars
*  usb_uart_transferByteCount - Update the transfer byte count from the
*    last transaction.
*  usb_uart_ep0Count - counts the data loaded from the SIE memory
*    in current packet.
*  usb_uart_transferByteCount - sum of the previous bytes transferred
*    on previous packets(sum of USBFS_lastPacketSize)
*  usb_uart_ep0Toggle - inverted
*  usb_uart_ep0Mode  - set to MODE_ACK_OUT_STATUS_IN.
*
* \reentrant
*  No.
*
*******************************************************************************/
void usb_uart_ControlWriteDataStage(void) 
{
    uint8 ep0Count;
    uint8 regIndex = 0u;

    ep0Count = (usb_uart_EP0_CNT_REG & usb_uart_EPX_CNT0_MASK) - usb_uart_EPX_CNTX_CRC_COUNT;

    usb_uart_transferByteCount += (uint8)ep0Count;

    while ((usb_uart_currentTD.count > 0u) && (ep0Count > 0u))
    {
        *usb_uart_currentTD.pData = (uint8) usb_uart_EP0_DR_BASE.epData[regIndex];
        usb_uart_currentTD.pData = &usb_uart_currentTD.pData[1u];
        regIndex++;
        ep0Count--;
        usb_uart_currentTD.count--;
    }
    
    usb_uart_ep0Count = (uint8)ep0Count;
    
    /* Update the data toggle */
    usb_uart_ep0Toggle ^= usb_uart_EP0_CNT_DATA_TOGGLE;
    
    /* Expect Data or Status Stage */
    usb_uart_ep0Mode = usb_uart_MODE_ACK_OUT_STATUS_IN;
}


/*******************************************************************************
* Function Name: usb_uart_ControlWriteStatusStage
****************************************************************************//**
*
*  Handle the Status Stage of a control write transfer
*
* \globalvars
*  usb_uart_transferState - set to TRANS_STATE_IDLE.
*  usb_uart_USBFS_ep0Mode  - set to MODE_STALL_IN_OUT.
*
* \reentrant
*  No.
*
*******************************************************************************/
void usb_uart_ControlWriteStatusStage(void) 
{
    /* Go Idle */
    usb_uart_transferState = usb_uart_TRANS_STATE_IDLE;
    
    /* Update the completion block */    
    usb_uart_UpdateStatusBlock(usb_uart_XFER_STATUS_ACK);
    
    /* We expect no more data, so stall INs and OUTs */
    usb_uart_ep0Mode = usb_uart_MODE_STALL_IN_OUT;
}


/*******************************************************************************
* Function Name: usb_uart_InitNoDataControlTransfer
****************************************************************************//**
*
*  Initialize a no data control transfer
*
* \return
*  requestHandled state.
*
* \globalvars
*  usb_uart_transferState - set to TRANS_STATE_NO_DATA_CONTROL.
*  usb_uart_ep0Mode  - set to MODE_STATUS_IN_ONLY.
*  usb_uart_ep0Count - cleared.
*  usb_uart_ep0Toggle - set to EP0_CNT_DATA_TOGGLE
*
* \reentrant
*  No.
*
*******************************************************************************/
uint8 usb_uart_InitNoDataControlTransfer(void) 
{
    usb_uart_transferState = usb_uart_TRANS_STATE_NO_DATA_CONTROL;
    usb_uart_ep0Mode       = usb_uart_MODE_STATUS_IN_ONLY;
    usb_uart_ep0Toggle     = usb_uart_EP0_CNT_DATA_TOGGLE;
    usb_uart_ep0Count      = 0u;

    return (usb_uart_TRUE);
}


/*******************************************************************************
* Function Name: usb_uart_NoDataControlStatusStage
****************************************************************************//**
*  Handle the Status Stage of a no data control transfer.
*
*  SET_ADDRESS is special, since we need to receive the status stage with
*  the old address.
*
* \globalvars
*  usb_uart_transferState - set to TRANS_STATE_IDLE.
*  usb_uart_ep0Mode  - set to MODE_STALL_IN_OUT.
*  usb_uart_ep0Toggle - set to EP0_CNT_DATA_TOGGLE
*  usb_uart_deviceAddress - used to set new address and cleared
*
* \reentrant
*  No.
*
*******************************************************************************/
void usb_uart_NoDataControlStatusStage(void) 
{
    if (0u != usb_uart_deviceAddress)
    {
        /* Update device address if we got new address. */
        usb_uart_CR0_REG = (uint8) usb_uart_deviceAddress | usb_uart_CR0_ENABLE;
        usb_uart_deviceAddress = 0u;
    }

    usb_uart_transferState = usb_uart_TRANS_STATE_IDLE;
    
    /* Update the completion block. */
    usb_uart_UpdateStatusBlock(usb_uart_XFER_STATUS_ACK);
    
    /* Stall IN and OUT, no more data is expected. */
    usb_uart_ep0Mode = usb_uart_MODE_STALL_IN_OUT;
}


/*******************************************************************************
* Function Name: usb_uart_UpdateStatusBlock
****************************************************************************//**
*
*  Update the Completion Status Block for a Request.  The block is updated
*  with the completion code the usb_uart_transferByteCount.  The
*  StatusBlock Pointer is set to NULL.
*
*  completionCode - status.
*
*
* \globalvars
*  usb_uart_currentTD.pStatusBlock->status - updated by the
*    completionCode parameter.
*  usb_uart_currentTD.pStatusBlock->length - updated.
*  usb_uart_currentTD.pStatusBlock - cleared.
*
* \reentrant
*  No.
*
*******************************************************************************/
void usb_uart_UpdateStatusBlock(uint8 completionCode) 
{
    if (usb_uart_currentTD.pStatusBlock != NULL)
    {
        usb_uart_currentTD.pStatusBlock->status = completionCode;
        usb_uart_currentTD.pStatusBlock->length = usb_uart_transferByteCount;
        usb_uart_currentTD.pStatusBlock = NULL;
    }
}


/*******************************************************************************
* Function Name: usb_uart_InitializeStatusBlock
****************************************************************************//**
*
*  Initialize the Completion Status Block for a Request.  The completion
*  code is set to USB_XFER_IDLE.
*
*  Also, initializes usb_uart_transferByteCount.  Save some space,
*  this is the only consumer.
*
* \globalvars
*  usb_uart_currentTD.pStatusBlock->status - set to XFER_IDLE.
*  usb_uart_currentTD.pStatusBlock->length - cleared.
*  usb_uart_transferByteCount - cleared.
*
* \reentrant
*  No.
*
*******************************************************************************/
void usb_uart_InitializeStatusBlock(void) 
{
    usb_uart_transferByteCount = 0u;
    
    if (usb_uart_currentTD.pStatusBlock != NULL)
    {
        usb_uart_currentTD.pStatusBlock->status = usb_uart_XFER_IDLE;
        usb_uart_currentTD.pStatusBlock->length = 0u;
    }
}


/* [] END OF FILE */
