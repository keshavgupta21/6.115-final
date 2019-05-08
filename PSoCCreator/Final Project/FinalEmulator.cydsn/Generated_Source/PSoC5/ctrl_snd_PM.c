/*******************************************************************************
* File Name: ctrl_snd_PM.c
* Version 1.80
*
* Description:
*  This file contains the setup, control, and status commands to support 
*  the component operation in the low power mode. 
*
* Note:
*
********************************************************************************
* Copyright 2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/

#include "ctrl_snd.h"

/* Check for removal by optimization */
#if !defined(ctrl_snd_Sync_ctrl_reg__REMOVED)

static ctrl_snd_BACKUP_STRUCT  ctrl_snd_backup = {0u};

    
/*******************************************************************************
* Function Name: ctrl_snd_SaveConfig
********************************************************************************
*
* Summary:
*  Saves the control register value.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void ctrl_snd_SaveConfig(void) 
{
    ctrl_snd_backup.controlState = ctrl_snd_Control;
}


/*******************************************************************************
* Function Name: ctrl_snd_RestoreConfig
********************************************************************************
*
* Summary:
*  Restores the control register value.
*
* Parameters:
*  None
*
* Return:
*  None
*
*
*******************************************************************************/
void ctrl_snd_RestoreConfig(void) 
{
     ctrl_snd_Control = ctrl_snd_backup.controlState;
}


/*******************************************************************************
* Function Name: ctrl_snd_Sleep
********************************************************************************
*
* Summary:
*  Prepares the component for entering the low power mode.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void ctrl_snd_Sleep(void) 
{
    ctrl_snd_SaveConfig();
}


/*******************************************************************************
* Function Name: ctrl_snd_Wakeup
********************************************************************************
*
* Summary:
*  Restores the component after waking up from the low power mode.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void ctrl_snd_Wakeup(void)  
{
    ctrl_snd_RestoreConfig();
}

#endif /* End check for removal by optimization */


/* [] END OF FILE */
