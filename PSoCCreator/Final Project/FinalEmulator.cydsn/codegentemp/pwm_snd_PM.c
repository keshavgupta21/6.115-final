/*******************************************************************************
* File Name: pwm_snd_PM.c
* Version 3.30
*
* Description:
*  This file provides the power management source code to API for the
*  PWM.
*
* Note:
*
********************************************************************************
* Copyright 2008-2014, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "pwm_snd.h"

static pwm_snd_backupStruct pwm_snd_backup;


/*******************************************************************************
* Function Name: pwm_snd_SaveConfig
********************************************************************************
*
* Summary:
*  Saves the current user configuration of the component.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Global variables:
*  pwm_snd_backup:  Variables of this global structure are modified to
*  store the values of non retention configuration registers when Sleep() API is
*  called.
*
*******************************************************************************/
void pwm_snd_SaveConfig(void) 
{

    #if(!pwm_snd_UsingFixedFunction)
        #if(!pwm_snd_PWMModeIsCenterAligned)
            pwm_snd_backup.PWMPeriod = pwm_snd_ReadPeriod();
        #endif /* (!pwm_snd_PWMModeIsCenterAligned) */
        pwm_snd_backup.PWMUdb = pwm_snd_ReadCounter();
        #if (pwm_snd_UseStatus)
            pwm_snd_backup.InterruptMaskValue = pwm_snd_STATUS_MASK;
        #endif /* (pwm_snd_UseStatus) */

        #if(pwm_snd_DeadBandMode == pwm_snd__B_PWM__DBM_256_CLOCKS || \
            pwm_snd_DeadBandMode == pwm_snd__B_PWM__DBM_2_4_CLOCKS)
            pwm_snd_backup.PWMdeadBandValue = pwm_snd_ReadDeadTime();
        #endif /*  deadband count is either 2-4 clocks or 256 clocks */

        #if(pwm_snd_KillModeMinTime)
             pwm_snd_backup.PWMKillCounterPeriod = pwm_snd_ReadKillTime();
        #endif /* (pwm_snd_KillModeMinTime) */

        #if(pwm_snd_UseControl)
            pwm_snd_backup.PWMControlRegister = pwm_snd_ReadControlRegister();
        #endif /* (pwm_snd_UseControl) */
    #endif  /* (!pwm_snd_UsingFixedFunction) */
}


/*******************************************************************************
* Function Name: pwm_snd_RestoreConfig
********************************************************************************
*
* Summary:
*  Restores the current user configuration of the component.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Global variables:
*  pwm_snd_backup:  Variables of this global structure are used to
*  restore the values of non retention registers on wakeup from sleep mode.
*
*******************************************************************************/
void pwm_snd_RestoreConfig(void) 
{
        #if(!pwm_snd_UsingFixedFunction)
            #if(!pwm_snd_PWMModeIsCenterAligned)
                pwm_snd_WritePeriod(pwm_snd_backup.PWMPeriod);
            #endif /* (!pwm_snd_PWMModeIsCenterAligned) */

            pwm_snd_WriteCounter(pwm_snd_backup.PWMUdb);

            #if (pwm_snd_UseStatus)
                pwm_snd_STATUS_MASK = pwm_snd_backup.InterruptMaskValue;
            #endif /* (pwm_snd_UseStatus) */

            #if(pwm_snd_DeadBandMode == pwm_snd__B_PWM__DBM_256_CLOCKS || \
                pwm_snd_DeadBandMode == pwm_snd__B_PWM__DBM_2_4_CLOCKS)
                pwm_snd_WriteDeadTime(pwm_snd_backup.PWMdeadBandValue);
            #endif /* deadband count is either 2-4 clocks or 256 clocks */

            #if(pwm_snd_KillModeMinTime)
                pwm_snd_WriteKillTime(pwm_snd_backup.PWMKillCounterPeriod);
            #endif /* (pwm_snd_KillModeMinTime) */

            #if(pwm_snd_UseControl)
                pwm_snd_WriteControlRegister(pwm_snd_backup.PWMControlRegister);
            #endif /* (pwm_snd_UseControl) */
        #endif  /* (!pwm_snd_UsingFixedFunction) */
    }


/*******************************************************************************
* Function Name: pwm_snd_Sleep
********************************************************************************
*
* Summary:
*  Disables block's operation and saves the user configuration. Should be called
*  just prior to entering sleep.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Global variables:
*  pwm_snd_backup.PWMEnableState:  Is modified depending on the enable
*  state of the block before entering sleep mode.
*
*******************************************************************************/
void pwm_snd_Sleep(void) 
{
    #if(pwm_snd_UseControl)
        if(pwm_snd_CTRL_ENABLE == (pwm_snd_CONTROL & pwm_snd_CTRL_ENABLE))
        {
            /*Component is enabled */
            pwm_snd_backup.PWMEnableState = 1u;
        }
        else
        {
            /* Component is disabled */
            pwm_snd_backup.PWMEnableState = 0u;
        }
    #endif /* (pwm_snd_UseControl) */

    /* Stop component */
    pwm_snd_Stop();

    /* Save registers configuration */
    pwm_snd_SaveConfig();
}


/*******************************************************************************
* Function Name: pwm_snd_Wakeup
********************************************************************************
*
* Summary:
*  Restores and enables the user configuration. Should be called just after
*  awaking from sleep.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Global variables:
*  pwm_snd_backup.pwmEnable:  Is used to restore the enable state of
*  block on wakeup from sleep mode.
*
*******************************************************************************/
void pwm_snd_Wakeup(void) 
{
     /* Restore registers values */
    pwm_snd_RestoreConfig();

    if(pwm_snd_backup.PWMEnableState != 0u)
    {
        /* Enable component's operation */
        pwm_snd_Enable();
    } /* Do nothing if component's block was disabled before */

}


/* [] END OF FILE */
