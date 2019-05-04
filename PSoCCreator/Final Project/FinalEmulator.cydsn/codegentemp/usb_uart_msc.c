/***************************************************************************//**
* \file usb_uart_cdc.c
* \version 3.20
*
* \brief
*  This file contains the USB MSC Class request handler and global API for MSC 
*  class.
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

#include "usb_uart_msc.h"
#include "usb_uart_pvt.h"
#include "cyapicallbacks.h"

#if (usb_uart_HANDLE_MSC_REQUESTS)

/***************************************
*          Internal variables
***************************************/

static uint8 usb_uart_lunCount = usb_uart_MSC_LUN_NUMBER;


/*******************************************************************************
* Function Name: usb_uart_DispatchMSCClassRqst
****************************************************************************//**
*   
*  \internal 
*  This routine dispatches MSC class requests.
*
* \return
*  Status of request processing: handled or not handled.
*
* \globalvars
*  usb_uart_lunCount - stores number of LUN (logical units).
*
* \reentrant
*  No.
*
*******************************************************************************/
uint8 usb_uart_DispatchMSCClassRqst(void) 
{
    uint8 requestHandled = usb_uart_FALSE;
    
    /* Get request data. */
    uint16 value  = usb_uart_GET_UINT16(usb_uart_wValueHiReg,  usb_uart_wValueLoReg);
    uint16 dataLength = usb_uart_GET_UINT16(usb_uart_wLengthHiReg, usb_uart_wLengthLoReg);
       
    /* Check request direction: D2H or H2D. */
    if (0u != (usb_uart_bmRequestTypeReg & usb_uart_RQST_DIR_D2H))
    {
        /* Handle direction from device to host. */
        
        if (usb_uart_MSC_GET_MAX_LUN == usb_uart_bRequestReg)
        {
            /* Check request fields. */
            if ((value  == usb_uart_MSC_GET_MAX_LUN_WVALUE) &&
                (dataLength == usb_uart_MSC_GET_MAX_LUN_WLENGTH))
            {
                /* Reply to Get Max LUN request: setup control read. */
                usb_uart_currentTD.pData = &usb_uart_lunCount;
                usb_uart_currentTD.count =  usb_uart_MSC_GET_MAX_LUN_WLENGTH;
                
                requestHandled  = usb_uart_InitControlRead();
            }
        }
    }
    else
    {
        /* Handle direction from host to device. */
        
        if (usb_uart_MSC_RESET == usb_uart_bRequestReg)
        {
            /* Check request fields. */
            if ((value  == usb_uart_MSC_RESET_WVALUE) &&
                (dataLength == usb_uart_MSC_RESET_WLENGTH))
            {
                /* Handle to Bulk-Only Reset request: no data control transfer. */
                usb_uart_currentTD.count = usb_uart_MSC_RESET_WLENGTH;
                
            #ifdef usb_uart_DISPATCH_MSC_CLASS_MSC_RESET_RQST_CALLBACK
                usb_uart_DispatchMSCClass_MSC_RESET_RQST_Callback();
            #endif /* (usb_uart_DISPATCH_MSC_CLASS_MSC_RESET_RQST_CALLBACK) */
                
                requestHandled = usb_uart_InitNoDataControlTransfer();
            }
        }
    }
    
    return (requestHandled);
}


/*******************************************************************************
* Function Name: usb_uart_MSC_SetLunCount
****************************************************************************//**
*
*  This function sets the number of logical units supported in the application. 
*  The default number of logical units is set in the component customizer.
*
*  \param lunCount: Count of the logical units. Valid range is between 1 and 16.
*
*
* \globalvars
*  usb_uart_lunCount - stores number of LUN (logical units).
*
* \reentrant
*  No.
*
*******************************************************************************/
void usb_uart_MSC_SetLunCount(uint8 lunCount) 
{
    usb_uart_lunCount = (lunCount - 1u);
}


/*******************************************************************************
* Function Name: usb_uart_MSC_GetLunCount
****************************************************************************//**
*
*  This function returns the number of logical units.
*
* \return
*   Number of the logical units.
*
* \globalvars
*  usb_uart_lunCount - stores number of LUN (logical units).
*
* \reentrant
*  No.
*
*******************************************************************************/
uint8 usb_uart_MSC_GetLunCount(void) 
{
    return (usb_uart_lunCount + 1u);
}   

#endif /* (usb_uart_HANDLE_MSC_REQUESTS) */


/* [] END OF FILE */
