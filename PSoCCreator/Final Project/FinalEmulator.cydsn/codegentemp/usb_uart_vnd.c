/***************************************************************************//**
* \file usb_uart_vnd.c
* \version 3.20
*
* \brief
*  This file contains the  USB vendor request handler.
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

#if(usb_uart_EXTERN_VND == usb_uart_FALSE)

/***************************************
* Vendor Specific Declarations
***************************************/

/* `#START VENDOR_SPECIFIC_DECLARATIONS` Place your declaration here */

/* `#END` */


/*******************************************************************************
* Function Name: usb_uart_HandleVendorRqst
****************************************************************************//**
*
*  This routine provide users with a method to implement vendor specific
*  requests.
*
*  To implement vendor specific requests, add your code in this function to
*  decode and disposition the request.  If the request is handled, your code
*  must set the variable "requestHandled" to TRUE, indicating that the
*  request has been handled.
*
* \return
*  requestHandled.
*
* \reentrant
*  No.
*
*******************************************************************************/
uint8 usb_uart_HandleVendorRqst(void) 
{
    uint8 requestHandled = usb_uart_FALSE;

    /* Check request direction: D2H or H2D. */
    if (0u != (usb_uart_bmRequestTypeReg & usb_uart_RQST_DIR_D2H))
    {
        /* Handle direction from device to host. */
        
        switch (usb_uart_bRequestReg)
        {
            case usb_uart_GET_EXTENDED_CONFIG_DESCRIPTOR:
            #if defined(usb_uart_ENABLE_MSOS_STRING)
                usb_uart_currentTD.pData = (volatile uint8 *) &usb_uart_MSOS_CONFIGURATION_DESCR[0u];
                usb_uart_currentTD.count = usb_uart_MSOS_CONFIGURATION_DESCR[0u];
                requestHandled  = usb_uart_InitControlRead();
            #endif /* (usb_uart_ENABLE_MSOS_STRING) */
                break;
            
            default:
                break;
        }
    }

    /* `#START VENDOR_SPECIFIC_CODE` Place your vendor specific request here */

    /* `#END` */

#ifdef usb_uart_HANDLE_VENDOR_RQST_CALLBACK
    if (usb_uart_FALSE == requestHandled)
    {
        requestHandled = usb_uart_HandleVendorRqst_Callback();
    }
#endif /* (usb_uart_HANDLE_VENDOR_RQST_CALLBACK) */

    return (requestHandled);
}


/*******************************************************************************
* Additional user functions supporting Vendor Specific Requests
********************************************************************************/

/* `#START VENDOR_SPECIFIC_FUNCTIONS` Place any additional functions here */

/* `#END` */


#endif /* usb_uart_EXTERN_VND */


/* [] END OF FILE */
