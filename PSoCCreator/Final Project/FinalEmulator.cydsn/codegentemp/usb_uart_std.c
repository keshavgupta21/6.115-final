/***************************************************************************//**
* \file usb_uart_std.c
* \version 3.20
*
* \brief
*  This file contains the USB Standard request handler.
*
********************************************************************************
* \copyright
* Copyright 2008-2016, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "usb_uart_pvt.h"

/***************************************
*   Static data allocation
***************************************/

#if defined(usb_uart_ENABLE_FWSN_STRING)
    static volatile uint8* usb_uart_fwSerialNumberStringDescriptor;
    static volatile uint8  usb_uart_snStringConfirm = usb_uart_FALSE;
#endif  /* (usb_uart_ENABLE_FWSN_STRING) */

#if defined(usb_uart_ENABLE_FWSN_STRING)
    /***************************************************************************
    * Function Name: usb_uart_SerialNumString
    ************************************************************************//**
    *
    *  This function is available only when the User Call Back option in the 
    *  Serial Number String descriptor properties is selected. Application 
    *  firmware can provide the source of the USB device serial number string 
    *  descriptor during run time. The default string is used if the application 
    *  firmware does not use this function or sets the wrong string descriptor.
    *
    *  \param snString:  Pointer to the user-defined string descriptor. The 
    *  string descriptor should meet the Universal Serial Bus Specification 
    *  revision 2.0 chapter 9.6.7
    *  Offset|Size|Value|Description
    *  ------|----|------|---------------------------------
    *  0     |1   |N     |Size of this descriptor in bytes
    *  1     |1   |0x03  |STRING Descriptor Type
    *  2     |N-2 |Number|UNICODE encoded string
    *  
    * *For example:* uint8 snString[16]={0x0E,0x03,'F',0,'W',0,'S',0,'N',0,'0',0,'1',0};
    *
    * \reentrant
    *  No.
    *
    ***************************************************************************/
    void  usb_uart_SerialNumString(uint8 snString[]) 
    {
        usb_uart_snStringConfirm = usb_uart_FALSE;
        
        if (snString != NULL)
        {
            /* Check descriptor validation */
            if ((snString[0u] > 1u) && (snString[1u] == usb_uart_DESCR_STRING))
            {
                usb_uart_fwSerialNumberStringDescriptor = snString;
                usb_uart_snStringConfirm = usb_uart_TRUE;
            }
        }
    }
#endif  /* usb_uart_ENABLE_FWSN_STRING */


/*******************************************************************************
* Function Name: usb_uart_HandleStandardRqst
****************************************************************************//**
*
*  This Routine dispatches standard requests
*
*
* \return
*  TRUE if request handled.
*
* \reentrant
*  No.
*
*******************************************************************************/
uint8 usb_uart_HandleStandardRqst(void) 
{
    uint8 requestHandled = usb_uart_FALSE;
    uint8 interfaceNumber;
    uint8 configurationN;
    uint8 bmRequestType = usb_uart_bmRequestTypeReg;

#if defined(usb_uart_ENABLE_STRINGS)
    volatile uint8 *pStr = 0u;
    #if defined(usb_uart_ENABLE_DESCRIPTOR_STRINGS)
        uint8 nStr;
        uint8 descrLength;
    #endif /* (usb_uart_ENABLE_DESCRIPTOR_STRINGS) */
#endif /* (usb_uart_ENABLE_STRINGS) */
    
    static volatile uint8 usb_uart_tBuffer[usb_uart_STATUS_LENGTH_MAX];
    const T_usb_uart_LUT CYCODE *pTmp;

    usb_uart_currentTD.count = 0u;

    if (usb_uart_RQST_DIR_D2H == (bmRequestType & usb_uart_RQST_DIR_MASK))
    {
        /* Control Read */
        switch (usb_uart_bRequestReg)
        {
            case usb_uart_GET_DESCRIPTOR:
                if (usb_uart_DESCR_DEVICE ==usb_uart_wValueHiReg)
                {
                    pTmp = usb_uart_GetDeviceTablePtr();
                    usb_uart_currentTD.pData = (volatile uint8 *)pTmp->p_list;
                    usb_uart_currentTD.count = usb_uart_DEVICE_DESCR_LENGTH;
                    
                    requestHandled  = usb_uart_InitControlRead();
                }
                else if (usb_uart_DESCR_CONFIG == usb_uart_wValueHiReg)
                {
                    pTmp = usb_uart_GetConfigTablePtr((uint8) usb_uart_wValueLoReg);
                    
                    /* Verify that requested descriptor exists */
                    if (pTmp != NULL)
                    {
                        usb_uart_currentTD.pData = (volatile uint8 *)pTmp->p_list;
                        usb_uart_currentTD.count = (uint16)((uint16)(usb_uart_currentTD.pData)[usb_uart_CONFIG_DESCR_TOTAL_LENGTH_HI] << 8u) | \
                                                                            (usb_uart_currentTD.pData)[usb_uart_CONFIG_DESCR_TOTAL_LENGTH_LOW];
                        requestHandled  = usb_uart_InitControlRead();
                    }
                }
                
            #if(usb_uart_BOS_ENABLE)
                else if (usb_uart_DESCR_BOS == usb_uart_wValueHiReg)
                {
                    pTmp = usb_uart_GetBOSPtr();
                    
                    /* Verify that requested descriptor exists */
                    if (pTmp != NULL)
                    {
                        usb_uart_currentTD.pData = (volatile uint8 *)pTmp;
                        usb_uart_currentTD.count = ((uint16)((uint16)(usb_uart_currentTD.pData)[usb_uart_BOS_DESCR_TOTAL_LENGTH_HI] << 8u)) | \
                                                                             (usb_uart_currentTD.pData)[usb_uart_BOS_DESCR_TOTAL_LENGTH_LOW];
                        requestHandled  = usb_uart_InitControlRead();
                    }
                }
            #endif /*(usb_uart_BOS_ENABLE)*/
            
            #if defined(usb_uart_ENABLE_STRINGS)
                else if (usb_uart_DESCR_STRING == usb_uart_wValueHiReg)
                {
                /* Descriptor Strings */
                #if defined(usb_uart_ENABLE_DESCRIPTOR_STRINGS)
                    nStr = 0u;
                    pStr = (volatile uint8 *) &usb_uart_STRING_DESCRIPTORS[0u];
                    
                    while ((usb_uart_wValueLoReg > nStr) && (*pStr != 0u))
                    {
                        /* Read descriptor length from 1st byte */
                        descrLength = *pStr;
                        /* Move to next string descriptor */
                        pStr = &pStr[descrLength];
                        nStr++;
                    }
                #endif /* (usb_uart_ENABLE_DESCRIPTOR_STRINGS) */
                
                /* Microsoft OS String */
                #if defined(usb_uart_ENABLE_MSOS_STRING)
                    if (usb_uart_STRING_MSOS == usb_uart_wValueLoReg)
                    {
                        pStr = (volatile uint8 *)& usb_uart_MSOS_DESCRIPTOR[0u];
                    }
                #endif /* (usb_uart_ENABLE_MSOS_STRING) */
                
                /* SN string */
                #if defined(usb_uart_ENABLE_SN_STRING)
                    if ((usb_uart_wValueLoReg != 0u) && 
                        (usb_uart_wValueLoReg == usb_uart_DEVICE0_DESCR[usb_uart_DEVICE_DESCR_SN_SHIFT]))
                    {
                    #if defined(usb_uart_ENABLE_IDSN_STRING)
                        /* Read DIE ID and generate string descriptor in RAM */
                        usb_uart_ReadDieID(usb_uart_idSerialNumberStringDescriptor);
                        pStr = usb_uart_idSerialNumberStringDescriptor;
                    #elif defined(usb_uart_ENABLE_FWSN_STRING)
                        
                        if(usb_uart_snStringConfirm != usb_uart_FALSE)
                        {
                            pStr = usb_uart_fwSerialNumberStringDescriptor;
                        }
                        else
                        {
                            pStr = (volatile uint8 *)&usb_uart_SN_STRING_DESCRIPTOR[0u];
                        }
                    #else
                        pStr = (volatile uint8 *)&usb_uart_SN_STRING_DESCRIPTOR[0u];
                    #endif  /* (usb_uart_ENABLE_IDSN_STRING) */
                    }
                #endif /* (usb_uart_ENABLE_SN_STRING) */
                
                    if (*pStr != 0u)
                    {
                        usb_uart_currentTD.count = *pStr;
                        usb_uart_currentTD.pData = pStr;
                        requestHandled  = usb_uart_InitControlRead();
                    }
                }
            #endif /*  usb_uart_ENABLE_STRINGS */
                else
                {
                    requestHandled = usb_uart_DispatchClassRqst();
                }
                break;
                
            case usb_uart_GET_STATUS:
                switch (bmRequestType & usb_uart_RQST_RCPT_MASK)
                {
                    case usb_uart_RQST_RCPT_EP:
                        usb_uart_currentTD.count = usb_uart_EP_STATUS_LENGTH;
                        usb_uart_tBuffer[0u]     = usb_uart_EP[usb_uart_wIndexLoReg & usb_uart_DIR_UNUSED].hwEpState;
                        usb_uart_tBuffer[1u]     = 0u;
                        usb_uart_currentTD.pData = &usb_uart_tBuffer[0u];
                        
                        requestHandled  = usb_uart_InitControlRead();
                        break;
                    case usb_uart_RQST_RCPT_DEV:
                        usb_uart_currentTD.count = usb_uart_DEVICE_STATUS_LENGTH;
                        usb_uart_tBuffer[0u]     = usb_uart_deviceStatus;
                        usb_uart_tBuffer[1u]     = 0u;
                        usb_uart_currentTD.pData = &usb_uart_tBuffer[0u];
                        
                        requestHandled  = usb_uart_InitControlRead();
                        break;
                    default:    /* requestHandled is initialized as FALSE by default */
                        break;
                }
                break;
                
            case usb_uart_GET_CONFIGURATION:
                usb_uart_currentTD.count = 1u;
                usb_uart_currentTD.pData = (volatile uint8 *) &usb_uart_configuration;
                requestHandled  = usb_uart_InitControlRead();
                break;
                
            case usb_uart_GET_INTERFACE:
                usb_uart_currentTD.count = 1u;
                usb_uart_currentTD.pData = (volatile uint8 *) &usb_uart_interfaceSetting[usb_uart_wIndexLoReg];
                requestHandled  = usb_uart_InitControlRead();
                break;
                
            default: /* requestHandled is initialized as FALSE by default */
                break;
        }
    }
    else
    {
        /* Control Write */
        switch (usb_uart_bRequestReg)
        {
            case usb_uart_SET_ADDRESS:
                /* Store address to be set in usb_uart_NoDataControlStatusStage(). */
                usb_uart_deviceAddress = (uint8) usb_uart_wValueLoReg;
                requestHandled = usb_uart_InitNoDataControlTransfer();
                break;
                
            case usb_uart_SET_CONFIGURATION:
                configurationN = usb_uart_wValueLoReg;
                
                /* Verify that configuration descriptor exists */
                if(configurationN > 0u)
                {
                    pTmp = usb_uart_GetConfigTablePtr((uint8) configurationN - 1u);
                }
                
                /* Responds with a Request Error when configuration number is invalid */
                if (((configurationN > 0u) && (pTmp != NULL)) || (configurationN == 0u))
                {
                    /* Set new configuration if it has been changed */
                    if(configurationN != usb_uart_configuration)
                    {
                        usb_uart_configuration = (uint8) configurationN;
                        usb_uart_configurationChanged = usb_uart_TRUE;
                        usb_uart_Config(usb_uart_TRUE);
                    }
                    requestHandled = usb_uart_InitNoDataControlTransfer();
                }
                break;
                
            case usb_uart_SET_INTERFACE:
                if (0u != usb_uart_ValidateAlternateSetting())
                {
                    /* Get interface number from the request. */
                    interfaceNumber = usb_uart_wIndexLoReg;
                    usb_uart_interfaceNumber = (uint8) usb_uart_wIndexLoReg;
                     
                    /* Check if alternate settings is changed for interface. */
                    if (usb_uart_interfaceSettingLast[interfaceNumber] != usb_uart_interfaceSetting[interfaceNumber])
                    {
                        usb_uart_configurationChanged = usb_uart_TRUE;
                    
                        /* Change alternate setting for the endpoints. */
                    #if (usb_uart_EP_MANAGEMENT_MANUAL && usb_uart_EP_ALLOC_DYNAMIC)
                        usb_uart_Config(usb_uart_FALSE);
                    #else
                        usb_uart_ConfigAltChanged();
                    #endif /* (usb_uart_EP_MANAGEMENT_MANUAL && usb_uart_EP_ALLOC_DYNAMIC) */
                    }
                    
                    requestHandled = usb_uart_InitNoDataControlTransfer();
                }
                break;
                
            case usb_uart_CLEAR_FEATURE:
                switch (bmRequestType & usb_uart_RQST_RCPT_MASK)
                {
                    case usb_uart_RQST_RCPT_EP:
                        if (usb_uart_wValueLoReg == usb_uart_ENDPOINT_HALT)
                        {
                            requestHandled = usb_uart_ClearEndpointHalt();
                        }
                        break;
                    case usb_uart_RQST_RCPT_DEV:
                        /* Clear device REMOTE_WAKEUP */
                        if (usb_uart_wValueLoReg == usb_uart_DEVICE_REMOTE_WAKEUP)
                        {
                            usb_uart_deviceStatus &= (uint8)~usb_uart_DEVICE_STATUS_REMOTE_WAKEUP;
                            requestHandled = usb_uart_InitNoDataControlTransfer();
                        }
                        break;
                    case usb_uart_RQST_RCPT_IFC:
                        /* Validate interfaceNumber */
                        if (usb_uart_wIndexLoReg < usb_uart_MAX_INTERFACES_NUMBER)
                        {
                            usb_uart_interfaceStatus[usb_uart_wIndexLoReg] &= (uint8) ~usb_uart_wValueLoReg;
                            requestHandled = usb_uart_InitNoDataControlTransfer();
                        }
                        break;
                    default:    /* requestHandled is initialized as FALSE by default */
                        break;
                }
                break;
                
            case usb_uart_SET_FEATURE:
                switch (bmRequestType & usb_uart_RQST_RCPT_MASK)
                {
                    case usb_uart_RQST_RCPT_EP:
                        if (usb_uart_wValueLoReg == usb_uart_ENDPOINT_HALT)
                        {
                            requestHandled = usb_uart_SetEndpointHalt();
                        }
                        break;
                        
                    case usb_uart_RQST_RCPT_DEV:
                        /* Set device REMOTE_WAKEUP */
                        if (usb_uart_wValueLoReg == usb_uart_DEVICE_REMOTE_WAKEUP)
                        {
                            usb_uart_deviceStatus |= usb_uart_DEVICE_STATUS_REMOTE_WAKEUP;
                            requestHandled = usb_uart_InitNoDataControlTransfer();
                        }
                        break;
                        
                    case usb_uart_RQST_RCPT_IFC:
                        /* Validate interfaceNumber */
                        if (usb_uart_wIndexLoReg < usb_uart_MAX_INTERFACES_NUMBER)
                        {
                            usb_uart_interfaceStatus[usb_uart_wIndexLoReg] &= (uint8) ~usb_uart_wValueLoReg;
                            requestHandled = usb_uart_InitNoDataControlTransfer();
                        }
                        break;
                    
                    default:    /* requestHandled is initialized as FALSE by default */
                        break;
                }
                break;
                
            default:    /* requestHandled is initialized as FALSE by default */
                break;
        }
    }
    
    return (requestHandled);
}


#if defined(usb_uart_ENABLE_IDSN_STRING)
    /***************************************************************************
    * Function Name: usb_uart_ReadDieID
    ************************************************************************//**
    *
    *  This routine read Die ID and generate Serial Number string descriptor.
    *
    *  \param descr:  pointer on string descriptor. This string size has to be equal or
    *          greater than usb_uart_IDSN_DESCR_LENGTH.
    *
    *
    * \reentrant
    *  No.
    *
    ***************************************************************************/
    void usb_uart_ReadDieID(uint8 descr[]) 
    {
        const char8 CYCODE hex[] = "0123456789ABCDEF";
        uint8 i;
        uint8 j = 0u;
        uint8 uniqueId[8u];

        if (NULL != descr)
        {
            /* Initialize descriptor header. */
            descr[0u] = usb_uart_IDSN_DESCR_LENGTH;
            descr[1u] = usb_uart_DESCR_STRING;
            
            /* Unique ID size is 8 bytes. */
            CyGetUniqueId((uint32 *) uniqueId);

            /* Fill descriptor with unique device ID. */
            for (i = 2u; i < usb_uart_IDSN_DESCR_LENGTH; i += 4u)
            {
                descr[i]      = (uint8) hex[(uniqueId[j] >> 4u)];
                descr[i + 1u] = 0u;
                descr[i + 2u] = (uint8) hex[(uniqueId[j] & 0x0Fu)];
                descr[i + 3u] = 0u;
                ++j;
            }
        }
    }
#endif /* (usb_uart_ENABLE_IDSN_STRING) */


/*******************************************************************************
* Function Name: usb_uart_ConfigReg
****************************************************************************//**
*
*  This routine configures hardware registers from the variables.
*  It is called from usb_uart_Config() function and from RestoreConfig
*  after Wakeup.
*
*******************************************************************************/
void usb_uart_ConfigReg(void) 
{
    uint8 ep;

#if (usb_uart_EP_MANAGEMENT_DMA_AUTO)
    uint8 epType = 0u;
#endif /* (usb_uart_EP_MANAGEMENT_DMA_AUTO) */

    /* Go thought all endpoints and set hardware configuration */
    for (ep = usb_uart_EP1; ep < usb_uart_MAX_EP; ++ep)
    {
        usb_uart_ARB_EP_BASE.arbEp[ep].epCfg = usb_uart_ARB_EPX_CFG_DEFAULT;
        
    #if (usb_uart_EP_MANAGEMENT_DMA)
        /* Enable arbiter endpoint interrupt sources */
        usb_uart_ARB_EP_BASE.arbEp[ep].epIntEn = usb_uart_ARB_EPX_INT_MASK;
    #endif /* (usb_uart_EP_MANAGEMENT_DMA) */
    
        if (usb_uart_EP[ep].epMode != usb_uart_MODE_DISABLE)
        {
            if (0u != (usb_uart_EP[ep].addr & usb_uart_DIR_IN))
            {
                usb_uart_SIE_EP_BASE.sieEp[ep].epCr0 = usb_uart_MODE_NAK_IN;
                
            #if (usb_uart_EP_MANAGEMENT_DMA_AUTO && CY_PSOC4)
                /* Clear DMA_TERMIN for IN endpoint. */
                usb_uart_ARB_EP_BASE.arbEp[ep].epIntEn &= (uint32) ~usb_uart_ARB_EPX_INT_DMA_TERMIN;
            #endif /* (usb_uart_EP_MANAGEMENT_DMA_AUTO && CY_PSOC4) */
            }
            else
            {
                usb_uart_SIE_EP_BASE.sieEp[ep].epCr0 = usb_uart_MODE_NAK_OUT;

            #if (usb_uart_EP_MANAGEMENT_DMA_AUTO)
                /* (CY_PSOC4): DMA_TERMIN for OUT endpoint is set above. */
                
                /* Prepare endpoint type mask. */
                epType |= (uint8) (0x01u << (ep - usb_uart_EP1));
            #endif /* (usb_uart_EP_MANAGEMENT_DMA_AUTO) */
            }
        }
        else
        {
            usb_uart_SIE_EP_BASE.sieEp[ep].epCr0 = usb_uart_MODE_STALL_DATA_EP;
        }
        
    #if (!usb_uart_EP_MANAGEMENT_DMA_AUTO)
        #if (CY_PSOC4)
            usb_uart_ARB_EP16_BASE.arbEp[ep].rwRa16  = (uint32) usb_uart_EP[ep].buffOffset;
            usb_uart_ARB_EP16_BASE.arbEp[ep].rwWa16  = (uint32) usb_uart_EP[ep].buffOffset;
        #else
            usb_uart_ARB_EP_BASE.arbEp[ep].rwRa    = LO8(usb_uart_EP[ep].buffOffset);
            usb_uart_ARB_EP_BASE.arbEp[ep].rwRaMsb = HI8(usb_uart_EP[ep].buffOffset);
            usb_uart_ARB_EP_BASE.arbEp[ep].rwWa    = LO8(usb_uart_EP[ep].buffOffset);
            usb_uart_ARB_EP_BASE.arbEp[ep].rwWaMsb = HI8(usb_uart_EP[ep].buffOffset);
        #endif /* (CY_PSOC4) */
    #endif /* (!usb_uart_EP_MANAGEMENT_DMA_AUTO) */
    }

#if (usb_uart_EP_MANAGEMENT_DMA_AUTO)
     /* BUF_SIZE depend on DMA_THRESS value:0x55-32 bytes  0x44-16 bytes 0x33-8 bytes 0x22-4 bytes 0x11-2 bytes */
    usb_uart_BUF_SIZE_REG = usb_uart_DMA_BUF_SIZE;

    /* Configure DMA burst threshold */
#if (CY_PSOC4)
    usb_uart_DMA_THRES16_REG   = usb_uart_DMA_BYTES_PER_BURST;
#else
    usb_uart_DMA_THRES_REG     = usb_uart_DMA_BYTES_PER_BURST;
    usb_uart_DMA_THRES_MSB_REG = 0u;
#endif /* (CY_PSOC4) */
    usb_uart_EP_ACTIVE_REG = usb_uart_DEFAULT_ARB_INT_EN;
    usb_uart_EP_TYPE_REG   = epType;
    
    /* Cfg_cmp bit set to 1 once configuration is complete. */
    /* Lock arbiter configtuation */
    usb_uart_ARB_CFG_REG |= (uint8)  usb_uart_ARB_CFG_CFG_CMP;
    /* Cfg_cmp bit set to 0 during configuration of PFSUSB Registers. */
    usb_uart_ARB_CFG_REG &= (uint8) ~usb_uart_ARB_CFG_CFG_CMP;

#endif /* (usb_uart_EP_MANAGEMENT_DMA_AUTO) */

    /* Enable interrupt SIE interurpt source from EP0-EP1 */
    usb_uart_SIE_EP_INT_EN_REG = (uint8) usb_uart_DEFAULT_SIE_EP_INT_EN;
}


/*******************************************************************************
* Function Name: usb_uart_EpStateInit
****************************************************************************//**
*
*  This routine initialize state of Data end points based of its type: 
*   IN  - usb_uart_IN_BUFFER_EMPTY (usb_uart_EVENT_PENDING)
*   OUT - usb_uart_OUT_BUFFER_EMPTY (usb_uart_NO_EVENT_PENDING)
*
*******************************************************************************/
void usb_uart_EpStateInit(void) 
{
    uint8 i;

    for (i = usb_uart_EP1; i < usb_uart_MAX_EP; i++)
    { 
        if (0u != (usb_uart_EP[i].addr & usb_uart_DIR_IN))
        {
            /* IN Endpoint */
            usb_uart_EP[i].apiEpState = usb_uart_EVENT_PENDING;
        }
        else
        {
            /* OUT Endpoint */
            usb_uart_EP[i].apiEpState = usb_uart_NO_EVENT_PENDING;
        }
    }
                    
}


/*******************************************************************************
* Function Name: usb_uart_Config
****************************************************************************//**
*
*  This routine configures endpoints for the entire configuration by scanning
*  the configuration descriptor.
*
*  \param clearAltSetting: It configures the bAlternateSetting 0 for each interface.
*
* usb_uart_interfaceClass - Initialized class array for each interface.
*   It is used for handling Class specific requests depend on interface class.
*   Different classes in multiple Alternate settings does not supported.
*
* \reentrant
*  No.
*
*******************************************************************************/
void usb_uart_Config(uint8 clearAltSetting) 
{
    uint8 ep;
    uint8 curEp;
    uint8 i;
    uint8 epType;
    const uint8 *pDescr;
    
    #if (!usb_uart_EP_MANAGEMENT_DMA_AUTO)
        uint16 buffCount = 0u;
    #endif /* (!usb_uart_EP_MANAGEMENT_DMA_AUTO) */

    const T_usb_uart_LUT CYCODE *pTmp;
    const T_usb_uart_EP_SETTINGS_BLOCK CYCODE *pEP;

    /* Clear endpoints settings */
    for (ep = 0u; ep < usb_uart_MAX_EP; ++ep)
    {
        usb_uart_EP[ep].attrib     = 0u;
        usb_uart_EP[ep].hwEpState  = 0u;
        usb_uart_EP[ep].epToggle   = 0u;
        usb_uart_EP[ep].bufferSize = 0u;
        usb_uart_EP[ep].interface  = 0u;
        usb_uart_EP[ep].apiEpState = usb_uart_NO_EVENT_PENDING;
        usb_uart_EP[ep].epMode     = usb_uart_MODE_DISABLE;   
    }

    /* Clear Alternate settings for all interfaces. */
    if (0u != clearAltSetting)
    {
        for (i = 0u; i < usb_uart_MAX_INTERFACES_NUMBER; ++i)
        {
            usb_uart_interfaceSetting[i]     = 0u;
            usb_uart_interfaceSettingLast[i] = 0u;
        }
    }

    /* Init Endpoints and Device Status if configured */
    if (usb_uart_configuration > 0u)
    {
        #if defined(usb_uart_ENABLE_CDC_CLASS)
            uint8 cdcComNums = 0u;
        #endif  /* (usb_uart_ENABLE_CDC_CLASS) */  

        pTmp = usb_uart_GetConfigTablePtr(usb_uart_configuration - 1u);
        
        /* Set Power status for current configuration */
        pDescr = (const uint8 *)pTmp->p_list;
        if ((pDescr[usb_uart_CONFIG_DESCR_ATTRIB] & usb_uart_CONFIG_DESCR_ATTRIB_SELF_POWERED) != 0u)
        {
            usb_uart_deviceStatus |= (uint8)  usb_uart_DEVICE_STATUS_SELF_POWERED;
        }
        else
        {
            usb_uart_deviceStatus &= (uint8) ~usb_uart_DEVICE_STATUS_SELF_POWERED;
        }
        
        /* Move to next element */
        pTmp = &pTmp[1u];
        ep = pTmp->c;  /* For this table, c is the number of endpoints configurations  */

        #if (usb_uart_EP_MANAGEMENT_MANUAL && usb_uart_EP_ALLOC_DYNAMIC)
            /* Configure for dynamic EP memory allocation */
            /* p_list points the endpoint setting table. */
            pEP = (T_usb_uart_EP_SETTINGS_BLOCK *) pTmp->p_list;
            
            for (i = 0u; i < ep; i++)
            {     
                /* Compare current Alternate setting with EP Alt */
                if (usb_uart_interfaceSetting[pEP->interface] == pEP->altSetting)
                {                                                          
                    curEp  = pEP->addr & usb_uart_DIR_UNUSED;
                    epType = pEP->attributes & usb_uart_EP_TYPE_MASK;
                    
                    usb_uart_EP[curEp].addr       = pEP->addr;
                    usb_uart_EP[curEp].attrib     = pEP->attributes;
                    usb_uart_EP[curEp].bufferSize = pEP->bufferSize;

                    if (0u != (pEP->addr & usb_uart_DIR_IN))
                    {
                        /* IN Endpoint */
                        usb_uart_EP[curEp].epMode     = usb_uart_GET_ACTIVE_IN_EP_CR0_MODE(epType);
                        usb_uart_EP[curEp].apiEpState = usb_uart_EVENT_PENDING;
                    
                    #if (defined(usb_uart_ENABLE_MIDI_STREAMING) && (usb_uart_MIDI_IN_BUFF_SIZE > 0))
                        if ((pEP->bMisc == usb_uart_CLASS_AUDIO) && (epType == usb_uart_EP_TYPE_BULK))
                        {
                            usb_uart_midi_in_ep = curEp;
                        }
                    #endif  /* (usb_uart_ENABLE_MIDI_STREAMING) */
                    }
                    else
                    {
                        /* OUT Endpoint */
                        usb_uart_EP[curEp].epMode     = usb_uart_GET_ACTIVE_OUT_EP_CR0_MODE(epType);
                        usb_uart_EP[curEp].apiEpState = usb_uart_NO_EVENT_PENDING;
                        
                    #if (defined(usb_uart_ENABLE_MIDI_STREAMING) && (usb_uart_MIDI_OUT_BUFF_SIZE > 0))
                        if ((pEP->bMisc == usb_uart_CLASS_AUDIO) && (epType == usb_uart_EP_TYPE_BULK))
                        {
                            usb_uart_midi_out_ep = curEp;
                        }
                    #endif  /* (usb_uart_ENABLE_MIDI_STREAMING) */
                    }

                #if(defined (usb_uart_ENABLE_CDC_CLASS))
                    if((pEP->bMisc == usb_uart_CLASS_CDC_DATA) ||(pEP->bMisc == usb_uart_CLASS_CDC))
                    {
                        cdcComNums = usb_uart_Cdc_EpInit(pEP, curEp, cdcComNums);
                    }
                #endif  /* (usb_uart_ENABLE_CDC_CLASS) */
                }
                
                pEP = &pEP[1u];
            }
            
        #else
            for (i = usb_uart_EP1; i < usb_uart_MAX_EP; ++i)
            {
                /* p_list points the endpoint setting table. */
                pEP = (const T_usb_uart_EP_SETTINGS_BLOCK CYCODE *) pTmp->p_list;
                /* Find max length for each EP and select it (length could be different in different Alt settings) */
                /* but other settings should be correct with regards to Interface alt Setting */
                
                for (curEp = 0u; curEp < ep; ++curEp)
                {
                    if (i == (pEP->addr & usb_uart_DIR_UNUSED))
                    {
                        /* Compare endpoint buffers size with current size to find greater. */
                        if (usb_uart_EP[i].bufferSize < pEP->bufferSize)
                        {
                            usb_uart_EP[i].bufferSize = pEP->bufferSize;
                        }
                        
                        /* Compare current Alternate setting with EP Alt */
                        if (usb_uart_interfaceSetting[pEP->interface] == pEP->altSetting)
                        {                            
                            usb_uart_EP[i].addr = pEP->addr;
                            usb_uart_EP[i].attrib = pEP->attributes;
                            
                            epType = pEP->attributes & usb_uart_EP_TYPE_MASK;
                            
                            if (0u != (pEP->addr & usb_uart_DIR_IN))
                            {
                                /* IN Endpoint */
                                usb_uart_EP[i].epMode     = usb_uart_GET_ACTIVE_IN_EP_CR0_MODE(epType);
                                usb_uart_EP[i].apiEpState = usb_uart_EVENT_PENDING;
                                
                            #if (defined(usb_uart_ENABLE_MIDI_STREAMING) && (usb_uart_MIDI_IN_BUFF_SIZE > 0))
                                if ((pEP->bMisc == usb_uart_CLASS_AUDIO) && (epType == usb_uart_EP_TYPE_BULK))
                                {
                                    usb_uart_midi_in_ep = i;
                                }
                            #endif  /* (usb_uart_ENABLE_MIDI_STREAMING) */
                            }
                            else
                            {
                                /* OUT Endpoint */
                                usb_uart_EP[i].epMode     = usb_uart_GET_ACTIVE_OUT_EP_CR0_MODE(epType);
                                usb_uart_EP[i].apiEpState = usb_uart_NO_EVENT_PENDING;
                                
                            #if (defined(usb_uart_ENABLE_MIDI_STREAMING) && (usb_uart_MIDI_OUT_BUFF_SIZE > 0))
                                if ((pEP->bMisc == usb_uart_CLASS_AUDIO) && (epType == usb_uart_EP_TYPE_BULK))
                                {
                                    usb_uart_midi_out_ep = i;
                                }
                            #endif  /* (usb_uart_ENABLE_MIDI_STREAMING) */
                            }

                        #if (defined(usb_uart_ENABLE_CDC_CLASS))
                            if((pEP->bMisc == usb_uart_CLASS_CDC_DATA) ||(pEP->bMisc == usb_uart_CLASS_CDC))
                            {
                                cdcComNums = usb_uart_Cdc_EpInit(pEP, i, cdcComNums);
                            }
                        #endif  /* (usb_uart_ENABLE_CDC_CLASS) */

                            #if (usb_uart_EP_MANAGEMENT_DMA_AUTO)
                                break;  /* Use first EP setting in Auto memory management */
                            #endif /* (usb_uart_EP_MANAGEMENT_DMA_AUTO) */
                        }
                    }
                    
                    pEP = &pEP[1u];
                }
            }
        #endif /*  (usb_uart_EP_MANAGEMENT_MANUAL && usb_uart_EP_ALLOC_DYNAMIC) */

        /* Init class array for each interface and interface number for each EP.
        *  It is used for handling Class specific requests directed to either an
        *  interface or the endpoint.
        */
        /* p_list points the endpoint setting table. */
        pEP = (const T_usb_uart_EP_SETTINGS_BLOCK CYCODE *) pTmp->p_list;
        for (i = 0u; i < ep; i++)
        {
            /* Configure interface number for each EP */
            usb_uart_EP[pEP->addr & usb_uart_DIR_UNUSED].interface = pEP->interface;
            pEP = &pEP[1u];
        }
        
        /* Init pointer on interface class table */
        usb_uart_interfaceClass = usb_uart_GetInterfaceClassTablePtr();
        
    /* Set the endpoint buffer addresses */
    #if (!usb_uart_EP_MANAGEMENT_DMA_AUTO)
        buffCount = 0u;
        for (ep = usb_uart_EP1; ep < usb_uart_MAX_EP; ++ep)
        {
            usb_uart_EP[ep].buffOffset = buffCount;        
            buffCount += usb_uart_EP[ep].bufferSize;
            
        #if (usb_uart_GEN_16BITS_EP_ACCESS)
            /* Align EP buffers to be event size to access 16-bits DR register. */
            buffCount += (0u != (buffCount & 0x01u)) ? 1u : 0u;
        #endif /* (usb_uart_GEN_16BITS_EP_ACCESS) */            
        }
    #endif /* (!usb_uart_EP_MANAGEMENT_DMA_AUTO) */

        /* Configure hardware registers */
        usb_uart_ConfigReg();
    }
}


/*******************************************************************************
* Function Name: usb_uart_ConfigAltChanged
****************************************************************************//**
*
*  This routine update configuration for the required endpoints only.
*  It is called after SET_INTERFACE request when Static memory allocation used.
*
* \reentrant
*  No.
*
*******************************************************************************/
void usb_uart_ConfigAltChanged(void) 
{
    uint8 ep;
    uint8 curEp;
    uint8 epType;
    uint8 i;
    uint8 interfaceNum;

    const T_usb_uart_LUT CYCODE *pTmp;
    const T_usb_uart_EP_SETTINGS_BLOCK CYCODE *pEP;

    /* Init Endpoints and Device Status if configured */
    if (usb_uart_configuration > 0u)
    {
        /* Get number of endpoints configurations (ep). */
        pTmp = usb_uart_GetConfigTablePtr(usb_uart_configuration - 1u);
        pTmp = &pTmp[1u];
        ep = pTmp->c;

        /* Get pointer to endpoints setting table (pEP). */
        pEP = (const T_usb_uart_EP_SETTINGS_BLOCK CYCODE *) pTmp->p_list;
        
        /* Look through all possible endpoint configurations. Find endpoints 
        * which belong to current interface and alternate settings for 
        * re-configuration.
        */
        interfaceNum = usb_uart_interfaceNumber;
        for (i = 0u; i < ep; i++)
        {
            /* Find endpoints which belong to current interface and alternate settings. */
            if ((interfaceNum == pEP->interface) && 
                (usb_uart_interfaceSetting[interfaceNum] == pEP->altSetting))
            {
                curEp  = ((uint8) pEP->addr & usb_uart_DIR_UNUSED);
                epType = ((uint8) pEP->attributes & usb_uart_EP_TYPE_MASK);
                
                /* Change the SIE mode for the selected EP to NAK ALL */
                usb_uart_EP[curEp].epToggle   = 0u;
                usb_uart_EP[curEp].addr       = pEP->addr;
                usb_uart_EP[curEp].attrib     = pEP->attributes;
                usb_uart_EP[curEp].bufferSize = pEP->bufferSize;

                if (0u != (pEP->addr & usb_uart_DIR_IN))
                {
                    /* IN Endpoint */
                    usb_uart_EP[curEp].epMode     = usb_uart_GET_ACTIVE_IN_EP_CR0_MODE(epType);
                    usb_uart_EP[curEp].apiEpState = usb_uart_EVENT_PENDING;
                }
                else
                {
                    /* OUT Endpoint */
                    usb_uart_EP[curEp].epMode     = usb_uart_GET_ACTIVE_OUT_EP_CR0_MODE(epType);
                    usb_uart_EP[curEp].apiEpState = usb_uart_NO_EVENT_PENDING;
                }
                
                /* Make SIE to NAK any endpoint requests */
                usb_uart_SIE_EP_BASE.sieEp[curEp].epCr0 = usb_uart_MODE_NAK_IN_OUT;

            #if (usb_uart_EP_MANAGEMENT_DMA_AUTO)
                /* Clear IN data ready. */
                usb_uart_ARB_EP_BASE.arbEp[curEp].epCfg &= (uint8) ~usb_uart_ARB_EPX_CFG_IN_DATA_RDY;

                /* Select endpoint number of reconfiguration */
                usb_uart_DYN_RECONFIG_REG = (uint8) ((curEp - 1u) << usb_uart_DYN_RECONFIG_EP_SHIFT);
                
                /* Request for dynamic re-configuration of endpoint. */
                usb_uart_DYN_RECONFIG_REG |= usb_uart_DYN_RECONFIG_ENABLE;
                
                /* Wait until block is ready for re-configuration */
                while (0u == (usb_uart_DYN_RECONFIG_REG & usb_uart_DYN_RECONFIG_RDY_STS))
                {
                }
                
                /* Once DYN_RECONFIG_RDY_STS bit is set, FW can change the EP configuration. */
                /* Change EP Type with new direction */
                if (0u != (pEP->addr & usb_uart_DIR_IN))
                {
                    /* Set endpoint type: 0 - IN and 1 - OUT. */
                    usb_uart_EP_TYPE_REG &= (uint8) ~(uint8)((uint8) 0x01u << (curEp - 1u));
                    
                #if (CY_PSOC4)
                    /* Clear DMA_TERMIN for IN endpoint */
                    usb_uart_ARB_EP_BASE.arbEp[curEp].epIntEn &= (uint32) ~usb_uart_ARB_EPX_INT_DMA_TERMIN;
                #endif /* (CY_PSOC4) */
                }
                else
                {
                    /* Set endpoint type: 0 - IN and 1- OUT. */
                    usb_uart_EP_TYPE_REG |= (uint8) ((uint8) 0x01u << (curEp - 1u));
                    
                #if (CY_PSOC4)
                    /* Set DMA_TERMIN for OUT endpoint */
                    usb_uart_ARB_EP_BASE.arbEp[curEp].epIntEn |= (uint32) usb_uart_ARB_EPX_INT_DMA_TERMIN;
                #endif /* (CY_PSOC4) */
                }
                
                /* Complete dynamic re-configuration: all endpoint related status and signals 
                * are set into the default state.
                */
                usb_uart_DYN_RECONFIG_REG &= (uint8) ~usb_uart_DYN_RECONFIG_ENABLE;

            #else
                usb_uart_SIE_EP_BASE.sieEp[curEp].epCnt0 = HI8(usb_uart_EP[curEp].bufferSize);
                usb_uart_SIE_EP_BASE.sieEp[curEp].epCnt1 = LO8(usb_uart_EP[curEp].bufferSize);
                
                #if (CY_PSOC4)
                    usb_uart_ARB_EP16_BASE.arbEp[curEp].rwRa16  = (uint32) usb_uart_EP[curEp].buffOffset;
                    usb_uart_ARB_EP16_BASE.arbEp[curEp].rwWa16  = (uint32) usb_uart_EP[curEp].buffOffset;
                #else
                    usb_uart_ARB_EP_BASE.arbEp[curEp].rwRa    = LO8(usb_uart_EP[curEp].buffOffset);
                    usb_uart_ARB_EP_BASE.arbEp[curEp].rwRaMsb = HI8(usb_uart_EP[curEp].buffOffset);
                    usb_uart_ARB_EP_BASE.arbEp[curEp].rwWa    = LO8(usb_uart_EP[curEp].buffOffset);
                    usb_uart_ARB_EP_BASE.arbEp[curEp].rwWaMsb = HI8(usb_uart_EP[curEp].buffOffset);
                #endif /* (CY_PSOC4) */                
            #endif /* (usb_uart_EP_MANAGEMENT_DMA_AUTO) */
            }
            
            pEP = &pEP[1u]; /* Get next EP element */
        }
        
        /* The main loop has to re-enable DMA and OUT endpoint */
    }
}


/*******************************************************************************
* Function Name: usb_uart_GetConfigTablePtr
****************************************************************************//**
*
*  This routine returns a pointer a configuration table entry
*
*  \param confIndex:  Configuration Index
*
* \return
*  Device Descriptor pointer or NULL when descriptor does not exist.
*
*******************************************************************************/
const T_usb_uart_LUT CYCODE *usb_uart_GetConfigTablePtr(uint8 confIndex)
                                                        
{
    /* Device Table */
    const T_usb_uart_LUT CYCODE *pTmp;

    pTmp = (const T_usb_uart_LUT CYCODE *) usb_uart_TABLE[usb_uart_device].p_list;

    /* The first entry points to the Device Descriptor,
    *  the second entry point to the BOS Descriptor
    *  the rest configuration entries.
    *  Set pointer to the first Configuration Descriptor
    */
    pTmp = &pTmp[2u];
    /* For this table, c is the number of configuration descriptors  */
    if(confIndex >= pTmp->c)   /* Verify that required configuration descriptor exists */
    {
        pTmp = (const T_usb_uart_LUT CYCODE *) NULL;
    }
    else
    {
        pTmp = (const T_usb_uart_LUT CYCODE *) pTmp[confIndex].p_list;
    }

    return (pTmp);
}


#if (usb_uart_BOS_ENABLE)
    /*******************************************************************************
    * Function Name: usb_uart_GetBOSPtr
    ****************************************************************************//**
    *
    *  This routine returns a pointer a BOS table entry
    *
    *  
    *
    * \return
    *  BOS Descriptor pointer or NULL when descriptor does not exist.
    *
    *******************************************************************************/
    const T_usb_uart_LUT CYCODE *usb_uart_GetBOSPtr(void)
                                                            
    {
        /* Device Table */
        const T_usb_uart_LUT CYCODE *pTmp;

        pTmp = (const T_usb_uart_LUT CYCODE *) usb_uart_TABLE[usb_uart_device].p_list;

        /* The first entry points to the Device Descriptor,
        *  the second entry points to the BOS Descriptor
        */
        pTmp = &pTmp[1u];
        pTmp = (const T_usb_uart_LUT CYCODE *) pTmp->p_list;
        return (pTmp);
    }
#endif /* (usb_uart_BOS_ENABLE) */


/*******************************************************************************
* Function Name: usb_uart_GetDeviceTablePtr
****************************************************************************//**
*
*  This routine returns a pointer to the Device table
*
* \return
*  Device Table pointer
*
*******************************************************************************/
const T_usb_uart_LUT CYCODE *usb_uart_GetDeviceTablePtr(void)
                                                            
{
    /* Device Table */
    return( (const T_usb_uart_LUT CYCODE *) usb_uart_TABLE[usb_uart_device].p_list );
}


/*******************************************************************************
* Function Name: USB_GetInterfaceClassTablePtr
****************************************************************************//**
*
*  This routine returns Interface Class table pointer, which contains
*  the relation between interface number and interface class.
*
* \return
*  Interface Class table pointer.
*
*******************************************************************************/
const uint8 CYCODE *usb_uart_GetInterfaceClassTablePtr(void)
                                                        
{
    const T_usb_uart_LUT CYCODE *pTmp;
    const uint8 CYCODE *pInterfaceClass;
    uint8 currentInterfacesNum;

    pTmp = usb_uart_GetConfigTablePtr(usb_uart_configuration - 1u);
    if (pTmp != NULL)
    {
        currentInterfacesNum  = ((const uint8 *) pTmp->p_list)[usb_uart_CONFIG_DESCR_NUM_INTERFACES];
        /* Third entry in the LUT starts the Interface Table pointers */
        /* The INTERFACE_CLASS table is located after all interfaces */
        pTmp = &pTmp[currentInterfacesNum + 2u];
        pInterfaceClass = (const uint8 CYCODE *) pTmp->p_list;
    }
    else
    {
        pInterfaceClass = (const uint8 CYCODE *) NULL;
    }

    return (pInterfaceClass);
}


/*******************************************************************************
* Function Name: usb_uart_TerminateEP
****************************************************************************//**
*
*  This function terminates the specified USBFS endpoint.
*  This function should be used before endpoint reconfiguration.
*
*  \param ep Contains the data endpoint number.
*
*  \reentrant
*  No.
*
* \sideeffect
* 
* The device responds with a NAK for any transactions on the selected endpoint.
*   
*******************************************************************************/
void usb_uart_TerminateEP(uint8 epNumber) 
{
    /* Get endpoint number */
    epNumber &= usb_uart_DIR_UNUSED;

    if ((epNumber > usb_uart_EP0) && (epNumber < usb_uart_MAX_EP))
    {
        /* Set the endpoint Halt */
        usb_uart_EP[epNumber].hwEpState |= usb_uart_ENDPOINT_STATUS_HALT;

        /* Clear the data toggle */
        usb_uart_EP[epNumber].epToggle = 0u;
        usb_uart_EP[epNumber].apiEpState = usb_uart_NO_EVENT_ALLOWED;

        if ((usb_uart_EP[epNumber].addr & usb_uart_DIR_IN) != 0u)
        {   
            /* IN Endpoint */
            usb_uart_SIE_EP_BASE.sieEp[epNumber].epCr0 = usb_uart_MODE_NAK_IN;
        }
        else
        {
            /* OUT Endpoint */
            usb_uart_SIE_EP_BASE.sieEp[epNumber].epCr0 = usb_uart_MODE_NAK_OUT;
        }
    }
}


/*******************************************************************************
* Function Name: usb_uart_SetEndpointHalt
****************************************************************************//**
*
*  This routine handles set endpoint halt.
*
* \return
*  requestHandled.
*
* \reentrant
*  No.
*
*******************************************************************************/
uint8 usb_uart_SetEndpointHalt(void) 
{
    uint8 requestHandled = usb_uart_FALSE;
    uint8 ep;
    
    /* Set endpoint halt */
    ep = usb_uart_wIndexLoReg & usb_uart_DIR_UNUSED;

    if ((ep > usb_uart_EP0) && (ep < usb_uart_MAX_EP))
    {
        /* Set the endpoint Halt */
        usb_uart_EP[ep].hwEpState |= (usb_uart_ENDPOINT_STATUS_HALT);

        /* Clear the data toggle */
        usb_uart_EP[ep].epToggle = 0u;
        usb_uart_EP[ep].apiEpState |= usb_uart_NO_EVENT_ALLOWED;

        if ((usb_uart_EP[ep].addr & usb_uart_DIR_IN) != 0u)
        {
            /* IN Endpoint */
            usb_uart_SIE_EP_BASE.sieEp[ep].epCr0 = (usb_uart_MODE_STALL_DATA_EP | 
                                                            usb_uart_MODE_ACK_IN);
        }
        else
        {
            /* OUT Endpoint */
            usb_uart_SIE_EP_BASE.sieEp[ep].epCr0 = (usb_uart_MODE_STALL_DATA_EP | 
                                                            usb_uart_MODE_ACK_OUT);
        }
        requestHandled = usb_uart_InitNoDataControlTransfer();
    }

    return (requestHandled);
}


/*******************************************************************************
* Function Name: usb_uart_ClearEndpointHalt
****************************************************************************//**
*
*  This routine handles clear endpoint halt.
*
* \return
*  requestHandled.
*
* \reentrant
*  No.
*
*******************************************************************************/
uint8 usb_uart_ClearEndpointHalt(void) 
{
    uint8 requestHandled = usb_uart_FALSE;
    uint8 ep;

    /* Clear endpoint halt */
    ep = usb_uart_wIndexLoReg & usb_uart_DIR_UNUSED;

    if ((ep > usb_uart_EP0) && (ep < usb_uart_MAX_EP))
    {
        /* Clear the endpoint Halt */
        usb_uart_EP[ep].hwEpState &= (uint8) ~usb_uart_ENDPOINT_STATUS_HALT;

        /* Clear the data toggle */
        usb_uart_EP[ep].epToggle = 0u;
        
        /* Clear toggle bit for already armed packet */
        usb_uart_SIE_EP_BASE.sieEp[ep].epCnt0 &= (uint8) ~(uint8)usb_uart_EPX_CNT_DATA_TOGGLE;
        
        /* Return API State as it was defined before */
        usb_uart_EP[ep].apiEpState &= (uint8) ~usb_uart_NO_EVENT_ALLOWED;

        if ((usb_uart_EP[ep].addr & usb_uart_DIR_IN) != 0u)
        {
            /* IN Endpoint */
            if(usb_uart_EP[ep].apiEpState == usb_uart_IN_BUFFER_EMPTY)
            {       
                /* Wait for next packet from application */
                usb_uart_SIE_EP_BASE.sieEp[ep].epCr0 = usb_uart_MODE_NAK_IN;
            }
            else    /* Continue armed transfer */
            {
                usb_uart_SIE_EP_BASE.sieEp[ep].epCr0 = usb_uart_MODE_ACK_IN;
            }
        }
        else
        {
            /* OUT Endpoint */
            if (usb_uart_EP[ep].apiEpState == usb_uart_OUT_BUFFER_FULL)
            {       
                /* Allow application to read full buffer */
                usb_uart_SIE_EP_BASE.sieEp[ep].epCr0 = usb_uart_MODE_NAK_OUT;
            }
            else    /* Mark endpoint as empty, so it will be reloaded */
            {
                usb_uart_SIE_EP_BASE.sieEp[ep].epCr0 = usb_uart_MODE_ACK_OUT;
            }
        }
        
        requestHandled = usb_uart_InitNoDataControlTransfer();
    }

    return(requestHandled);
}


/*******************************************************************************
* Function Name: usb_uart_ValidateAlternateSetting
****************************************************************************//**
*
*  Validates (and records) a SET INTERFACE request.
*
* \return
*  requestHandled.
*
* \reentrant
*  No.
*
*******************************************************************************/
uint8 usb_uart_ValidateAlternateSetting(void) 
{
    uint8 requestHandled = usb_uart_FALSE;
    
    uint8 interfaceNum;
    uint8 curInterfacesNum;
    const T_usb_uart_LUT CYCODE *pTmp;
    
    /* Get interface number from the request. */
    interfaceNum = (uint8) usb_uart_wIndexLoReg;
    
    /* Get number of interfaces for current configuration. */
    pTmp = usb_uart_GetConfigTablePtr(usb_uart_configuration - 1u);
    curInterfacesNum  = ((const uint8 *) pTmp->p_list)[usb_uart_CONFIG_DESCR_NUM_INTERFACES];

    /* Validate that interface number is within range. */
    if ((interfaceNum <= curInterfacesNum) || (interfaceNum <= usb_uart_MAX_INTERFACES_NUMBER))
    {
        /* Save current and new alternate settings (come with request) to make 
        * desicion about following endpoint re-configuration.
        */
        usb_uart_interfaceSettingLast[interfaceNum] = usb_uart_interfaceSetting[interfaceNum];
        usb_uart_interfaceSetting[interfaceNum]     = (uint8) usb_uart_wValueLoReg;
        
        requestHandled = usb_uart_TRUE;
    }

    return (requestHandled);
}


/* [] END OF FILE */
