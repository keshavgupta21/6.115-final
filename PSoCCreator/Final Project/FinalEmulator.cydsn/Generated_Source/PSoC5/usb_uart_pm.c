/***************************************************************************//**
* \file usb_uart_pm.c
* \version 3.20
*
* \brief
*  This file provides Suspend/Resume APIs implementation.
*
********************************************************************************
* \copyright
* Copyright 2008-2016, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "usb_uart_pvt.h"
#include "usb_uart_Dp.h"
#include "cyapicallbacks.h"

/***************************************
* Custom Declarations
***************************************/
/* `#START PM_CUSTOM_DECLARATIONS` Place your declaration here */

/* `#END` */


/***************************************
* Local data allocation
***************************************/

static usb_uart_BACKUP_STRUCT  usb_uart_backup;

#if (usb_uart_DP_ISR_ACTIVE)
    /*******************************************************************************
    * Function Name: usb_uart_DP_ISR
    ****************************************************************************//**
    *
    *  This Interrupt Service Routine handles DP pin changes for wake-up from
    *  the sleep mode.
    *
    *******************************************************************************/
    CY_ISR(usb_uart_DP_ISR)
    {
    #ifdef usb_uart_DP_ISR_ENTRY_CALLBACK
        usb_uart_DP_ISR_EntryCallback();
    #endif /* (usb_uart_DP_ISR_ENTRY_CALLBACK) */

        /* `#START DP_USER_CODE` Place your code here */

        /* `#END` */

        (void) usb_uart_Dp_ClearInterrupt();
		
	#ifdef usb_uart_DP_ISR_EXIT_CALLBACK
        usb_uart_DP_ISR_ExitCallback();
    #endif /* (usb_uart_DP_ISR_EXIT_CALLBACK) */
    }
#endif /* (usb_uart_DP_ISR_ACTIVE) */


/*******************************************************************************
* Function Name: usb_uart_SaveConfig
****************************************************************************//**
*
*  Saves the current user configuration.
*
* \reentrant
*  No.
*
*******************************************************************************/
void usb_uart_SaveConfig(void) 
{
    /* Empty function added for the compatibility purpose. */
}


/*******************************************************************************
* Function Name: usb_uart_RestoreConfig
****************************************************************************//**
*
*  Restores the current user configuration.
*
* \reentrant
*  No.
*
*******************************************************************************/
void usb_uart_RestoreConfig(void) 
{
    if (usb_uart_configuration != 0u)
    {
        usb_uart_ConfigReg();
        usb_uart_EpStateInit();
    }
}


/*******************************************************************************
* Function Name: usb_uart_Suspend
****************************************************************************//**
*
*  This function prepares the USBFS component to enter low power mode. The 
*  interrupt on falling edge on Dp pin is configured to wakeup device when the 
*  host drives resume condition. The pull-up is enabled on the Dp line while 
*  device is in low power mode. The supported low power modes are Deep Sleep 
*  (PSoC 4200L) and Sleep (PSoC 3/ PSoC 5LP).
*  
*  *Note* For PSoC 4200L devices, this function should not be called before 
*  entering Sleep.
*  
*  *Note* After enter low power mode, the data which is left in the IN or OUT 
*  endpoint buffers is not restored after wakeup and lost. Therefore it should 
*  be stored in the SRAM for OUT endpoint or read by the host for IN endpoint 
*  before enter low power mode.
*
* \globalvars
*  usb_uart_backup.enable:  modified.
*
* \reentrant
*  No.
*
*******************************************************************************/
void usb_uart_Suspend(void) 
{
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();

    if (0u != (usb_uart_CR0_REG & usb_uart_CR0_ENABLE))
    {
        /* USB block is enabled. */
        usb_uart_backup.enableState = 1u;

    #if (usb_uart_EP_MANAGEMENT_DMA)
        usb_uart_Stop_DMA(usb_uart_MAX_EP);
    #endif /* (usb_uart_EP_MANAGEMENT_DMA) */

    #if (CY_PSOC4)
        /* Suspend enter sequence. */
        usb_uart_POWER_CTRL_REG |= (usb_uart_POWER_CTRL_SUSPEND |
                                            usb_uart_POWER_CTRL_SUSPEND_DEL);

        /* Store state of USB regulator and disable it. */
        usb_uart_backup.mode = (uint8)  (usb_uart_CR1_REG & usb_uart_CR1_REG_ENABLE);
        usb_uart_CR1_REG    &= (uint32) ~usb_uart_CR1_REG_ENABLE;
        
        /* Store SIE interrupt sources. Valid bits are 0 - 4. */
        usb_uart_backup.intrSeiMask = (uint8) usb_uart_INTR_SIE_MASK_REG;

    #else
        /* Ensure USB transmit enable is low (USB_USBIO_CR0.ten). - Manual Transmission - Disabled. */
        usb_uart_USBIO_CR0_REG &= (uint8) ~usb_uart_USBIO_CR0_TEN;
        CyDelayUs(usb_uart_WAIT_REG_STABILITY_50NS);  /*~50ns delay. */

        /* Disable the USBIO by asserting PM.USB_CR0.fsusbio_pd_n(Inverted) and pd_pullup_hv(Inverted) high. */
        usb_uart_PM_USB_CR0_REG &= (uint8) ~(usb_uart_PM_USB_CR0_PD_N |
                                                     usb_uart_PM_USB_CR0_PD_PULLUP_N);

        /* Disable the SIE. */
        usb_uart_CR0_REG &= (uint8) ~usb_uart_CR0_ENABLE;

        CyDelayUs(usb_uart_WAIT_REG_STABILITY_50NS);  /* ~50ns delay. */
        /* Store mode and disable VRegulator. */
        usb_uart_backup.mode = (uint8) (usb_uart_CR1_REG & usb_uart_CR1_REG_ENABLE);
        usb_uart_CR1_REG    &= (uint8) ~usb_uart_CR1_REG_ENABLE;

        CyDelayUs(usb_uart_WAIT_REG_STABILITY_1US); /* min 0.5us delay required. */

        /* Disable the USBIO reference by setting PM.USB_CR0.fsusbio_ref_en.*/
        usb_uart_PM_USB_CR0_REG &= (uint8) ~usb_uart_PM_USB_CR0_REF_EN;

        /* Switch DP and DM terminals to GPIO mode and disconnect 1.5k pull-up. */
        usb_uart_USBIO_CR1_REG |= usb_uart_USBIO_CR1_IOMODE;

        /* Disable USBFS block. */
        /* Clear power active and standby mode templates: disable USB block. */
        usb_uart_PM_ACT_CFG_REG  &= (uint8) ~usb_uart_PM_ACT_EN_FSUSB;
        usb_uart_PM_STBY_CFG_REG &= (uint8) ~usb_uart_PM_STBY_EN_FSUSB;

        CyDelayUs(usb_uart_WAIT_REG_STABILITY_1US); /* min 0.5us delay required. */
    #endif /* (CY_PSOC4) */
    }
    else
    {
        usb_uart_backup.enableState = 0u;
    }

    CyExitCriticalSection(enableInterrupts);

#if (usb_uart_DP_ISR_ACTIVE)
    /* Clear active mode Dp interrupt source history. */
    (void) usb_uart_Dp_ClearInterrupt();
    CyIntClearPending(usb_uart_DP_INTC_VECT_NUM);

    CyIntEnable      (usb_uart_DP_INTC_VECT_NUM);
#endif /* (usb_uart_DP_ISR_ACTIVE). */
}


/*******************************************************************************
* Function Name: usb_uart_Resume
****************************************************************************//**
*
*  This function prepares the USBFS component for active mode operation after 
*  exit low power mode. It restores the component active mode configuration such 
*  as device address assigned previously by the host, endpoints buffer and disables 
*  interrupt on Dp pin.
*  The supported low power modes are Deep Sleep (PSoC 4200L) and Sleep 
*  (PSoC 3/ PSoC 5LP).
*  
*  *Note* For PSoC 4200L devices, this function should not be called after 
*  exiting Sleep.
*  
*  *Note* To resume communication with the host, the data endpoints must be 
*  managed: the OUT endpoints must be enabled and IN endpoints must be loaded 
*  with data. For DMA with Automatic Buffer Management, all endpoints buffers 
*  must be initialized again before making them available to the host.
*
*
* \globalvars
*  usb_uart_backup - checked.
*
* \reentrant
*  No.
*
*******************************************************************************/
void usb_uart_Resume(void) 
{
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();

    if (0u != usb_uart_backup.enableState)
    {
    #if (usb_uart_DP_ISR_ACTIVE)
        CyIntDisable(usb_uart_DP_INTC_VECT_NUM);
    #endif /* (usb_uart_DP_ISR_ACTIVE) */

    #if (CY_PSOC4)
        /* Enable clock to USB IP. */
        usb_uart_USB_CLK_EN_REG |= usb_uart_USB_CLK_CSR_CLK_EN;

        /* Restore arbiter configuration for DMA transfers. */
        #if (usb_uart_EP_MANAGEMENT_DMA)
            #if (usb_uart_ARB_ISR_ACTIVE)
                /* Enable ARB EP interrupt sources. */
                usb_uart_ARB_INT_EN_REG = usb_uart_DEFAULT_ARB_INT_EN;
            #endif /* (usb_uart_EP_MANAGEMENT_DMA) */

            /* Configure arbiter for Manual or Auto DMA operation and clear 
            * configuration completion. 
            */
            usb_uart_ARB_CFG_REG = usb_uart_DEFAULT_ARB_CFG;
        #endif /* (usb_uart_EP_MANAGEMENT_DMA) */

        /* Restore level (hi, lo, med) for each interrupt source. */
        usb_uart_INTR_LVL_SEL_REG = usb_uart_DEFAULT_INTR_LVL_SEL;
        
        /* Store SIE interrupt sources. */
        usb_uart_INTR_SIE_MASK_REG = (uint32) usb_uart_backup.intrSeiMask;

        /* Set EP0.CR: ACK Setup, NAK IN/OUT. */
        usb_uart_EP0_CR_REG = usb_uart_MODE_NAK_IN_OUT;

        /* Restore data EP1-8 configuration. */
        usb_uart_RestoreConfig();

        /* Restore state of USB regulator and wait until it supples stable power. */
        usb_uart_CR1_REG |= usb_uart_backup.mode;
        CyDelayUs(usb_uart_WAIT_VREF_STABILITY);

        /* Suspend exit sequence. */
        usb_uart_POWER_CTRL_REG &= (uint32) ~usb_uart_POWER_CTRL_SUSPEND;
        CyDelayUs(usb_uart_WAIT_SUSPEND_DEL_DISABLE);
        usb_uart_POWER_CTRL_REG &= (uint32) ~usb_uart_POWER_CTRL_SUSPEND_DEL;

    #else
        /* Set power active and standby mode templates: enable USB block. */
        usb_uart_PM_ACT_CFG_REG  |= usb_uart_PM_ACT_EN_FSUSB;
        usb_uart_PM_STBY_CFG_REG |= usb_uart_PM_STBY_EN_FSUSB;

        /* Enable core clock. */
        usb_uart_USB_CLK_EN_REG |= usb_uart_USB_CLK_ENABLE;

        /* Enable the USBIO reference by setting PM.USB_CR0.fsusbio_ref_en.*/
        usb_uart_PM_USB_CR0_REG |= usb_uart_PM_USB_CR0_REF_EN;

        /* The reference is available ~40us after power restored. */
        CyDelayUs(usb_uart_WAIT_VREF_RESTORE);
        /* Restore state of USB regulator and wait until it supples stable power. */
        usb_uart_CR1_REG |= usb_uart_backup.mode;
        CyDelayUs(usb_uart_WAIT_VREF_STABILITY);   /*~50ns delay. */

        /* Enable USBIO. */
        usb_uart_PM_USB_CR0_REG |= usb_uart_PM_USB_CR0_PD_N;
        CyDelayUs(usb_uart_WAIT_PD_PULLUP_N_ENABLE);
        /* Set the USBIO pull-up enable. */
        usb_uart_PM_USB_CR0_REG |= usb_uart_PM_USB_CR0_PD_PULLUP_N;

        /* Restore arbiter configuration for DMA transfers. */
        #if (usb_uart_EP_MANAGEMENT_DMA)
            #if (usb_uart_ARB_ISR_ACTIVE)
                /* Enable ARB EP interrupt sources. */
                usb_uart_ARB_INT_EN_REG = usb_uart_DEFAULT_ARB_INT_EN;
            #endif /* (usb_uart_EP_MANAGEMENT_DMA) */

            /* Configure arbiter for Manual or Auto DMA operation and clear 
            * configuration completion. 
            */
            usb_uart_ARB_CFG_REG = usb_uart_DEFAULT_ARB_CFG;
        #endif /* (usb_uart_EP_MANAGEMENT_DMA) */

        /* Set EP0.CR: ACK Setup, STALL IN/OUT. */
        usb_uart_EP0_CR_REG = usb_uart_MODE_STALL_IN_OUT;

        /* Enable the USB IP to respond to USB traffic with the last address. */
        usb_uart_CR0_REG |= usb_uart_CR0_ENABLE;
        CyDelayCycles(usb_uart_WAIT_CR0_REG_STABILITY);

        /* Enable D+ pull-up and keep USB control on IO. */
        usb_uart_USBIO_CR1_REG = usb_uart_USBIO_CR1_USBPUEN;

        /* Restore data EP1-8 configuration. */
        usb_uart_RestoreConfig();
    #endif /* (CY_PSOC4) */
    }

    CyExitCriticalSection(enableInterrupts);
}


/* [] END OF FILE */
