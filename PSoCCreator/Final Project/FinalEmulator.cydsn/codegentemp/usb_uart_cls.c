/***************************************************************************//**
* \file usb_uart_cls.c
* \version 3.20
*
* \brief
*  This file contains the USB Class request handler.
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

#if(usb_uart_EXTERN_CLS == usb_uart_FALSE)

/***************************************
* User Implemented Class Driver Declarations.
***************************************/
/* `#START USER_DEFINED_CLASS_DECLARATIONS` Place your declaration here */

/* `#END` */


/*******************************************************************************
* Function Name: usb_uart_DispatchClassRqst
****************************************************************************//**
*  This routine dispatches class specific requests depend on interface class.
*
* \return
*  requestHandled.
*
* \reentrant
*  No.
*
*******************************************************************************/
uint8 usb_uart_DispatchClassRqst(void) 
{
    uint8 interfaceNumber;
    uint8 requestHandled = usb_uart_FALSE;

    /* Get interface to which request is intended. */
    switch (usb_uart_bmRequestTypeReg & usb_uart_RQST_RCPT_MASK)
    {
        case usb_uart_RQST_RCPT_IFC:
            /* Class-specific request directed to interface: wIndexLoReg 
            * contains interface number.
            */
            interfaceNumber = (uint8) usb_uart_wIndexLoReg;
            break;
        
        case usb_uart_RQST_RCPT_EP:
            /* Class-specific request directed to endpoint: wIndexLoReg contains 
            * endpoint number. Find interface related to endpoint. 
            */
            interfaceNumber = usb_uart_EP[usb_uart_wIndexLoReg & usb_uart_DIR_UNUSED].interface;
            break;
            
        default:
            /* Default interface is zero. */
            interfaceNumber = 0u;
            break;
    }
    
    /* Check that interface is within acceptable range */
    if (interfaceNumber <= usb_uart_MAX_INTERFACES_NUMBER)
    {
    #if (defined(usb_uart_ENABLE_HID_CLASS)   || \
         defined(usb_uart_ENABLE_AUDIO_CLASS) || \
         defined(usb_uart_ENABLE_CDC_CLASS)   || \
         usb_uart_ENABLE_MSC_CLASS)

        /* Handle class request depends on interface type. */
        switch (usb_uart_interfaceClass[interfaceNumber])
        {
        #if defined(usb_uart_ENABLE_HID_CLASS)
            case usb_uart_CLASS_HID:
                requestHandled = usb_uart_DispatchHIDClassRqst();
                break;
        #endif /* (usb_uart_ENABLE_HID_CLASS) */
                
        #if defined(usb_uart_ENABLE_AUDIO_CLASS)
            case usb_uart_CLASS_AUDIO:
                requestHandled = usb_uart_DispatchAUDIOClassRqst();
                break;
        #endif /* (usb_uart_CLASS_AUDIO) */
                
        #if defined(usb_uart_ENABLE_CDC_CLASS)
            case usb_uart_CLASS_CDC:
                requestHandled = usb_uart_DispatchCDCClassRqst();
                break;
        #endif /* (usb_uart_ENABLE_CDC_CLASS) */
            
        #if (usb_uart_ENABLE_MSC_CLASS)
            case usb_uart_CLASS_MSD:
            #if (usb_uart_HANDLE_MSC_REQUESTS)
                /* MSC requests are handled by the component. */
                requestHandled = usb_uart_DispatchMSCClassRqst();
            #elif defined(usb_uart_DISPATCH_MSC_CLASS_RQST_CALLBACK)
                /* MSC requests are handled by user defined callbcak. */
                requestHandled = usb_uart_DispatchMSCClassRqst_Callback();
            #else
                /* MSC requests are not handled. */
                requestHandled = usb_uart_FALSE;
            #endif /* (usb_uart_HANDLE_MSC_REQUESTS) */
                break;
        #endif /* (usb_uart_ENABLE_MSC_CLASS) */
            
            default:
                /* Request is not handled: unknown class request type. */
                requestHandled = usb_uart_FALSE;
                break;
        }
    #endif /* Class support is enabled */
    }
    
    /* `#START USER_DEFINED_CLASS_CODE` Place your Class request here */

    /* `#END` */

#ifdef usb_uart_DISPATCH_CLASS_RQST_CALLBACK
    if (usb_uart_FALSE == requestHandled)
    {
        requestHandled = usb_uart_DispatchClassRqst_Callback(interfaceNumber);
    }
#endif /* (usb_uart_DISPATCH_CLASS_RQST_CALLBACK) */

    return (requestHandled);
}


/*******************************************************************************
* Additional user functions supporting Class Specific Requests
********************************************************************************/

/* `#START CLASS_SPECIFIC_FUNCTIONS` Place any additional functions here */

/* `#END` */

#endif /* usb_uart_EXTERN_CLS */


/* [] END OF FILE */
