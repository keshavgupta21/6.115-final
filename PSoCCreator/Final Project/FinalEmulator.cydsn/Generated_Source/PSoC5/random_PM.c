/*******************************************************************************
* File Name: random_PM.c
* Version 2.40
*
* Description:
*  This file provides Sleep APIs for PRS component.
*
* Note:
*  None
*
********************************************************************************
* Copyright 2008-2013, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/

#include "random.h"

#if ((random_TIME_MULTIPLEXING_ENABLE) && (random_WAKEUP_BEHAVIOUR == random__RESUMEWORK))
    uint8 random_sleepState = random_NORMAL_SEQUENCE;    
#endif  /* End ((random_TIME_MULTIPLEXING_ENABLE) && 
          (random_WAKEUP_BEHAVIOUR == random__RESUMEWORK)) */


/*******************************************************************************
* Function Name: random_SaveConfig
********************************************************************************
*
* Summary:
*  Saves seed and polynomial registers.
*
* Parameters:
*  void
*
* Return:
*  void
*
* Global Variables:
*  random_backup - modified when non-retention registers are saved.
*
*******************************************************************************/
void random_SaveConfig(void) 
{    
    #if (random_WAKEUP_BEHAVIOUR == random__RESUMEWORK)
        /* Save dff register for time mult */
        #if (random_TIME_MULTIPLEXING_ENABLE)
            random_backup.dffStatus = random_STATUS;
            /* Clear normal dff sequence set */
            random_sleepState &= ((uint8)~random_NORMAL_SEQUENCE);
        #endif  /* End random_TIME_MULTIPLEXING_ENABLE */
        
        /* Save A0 and A1 registers */
        #if (random_PRS_SIZE <= 32u)
            random_backup.seed = random_Read();
            
        #else
            random_backup.seedUpper = random_ReadUpper();
            random_backup.seedLower = random_ReadLower();
            
        #endif     /* End (random_PRS_SIZE <= 32u) */
        
    #endif  /* End (random_WAKEUP_BEHAVIOUR == random__RESUMEWORK) */
}


/*******************************************************************************
* Function Name: random_Sleep
********************************************************************************
*
* Summary:
*  Stops PRS computation and saves PRS configuration.
*
* Parameters:
*  void
*
* Return:
*  void
*
* Global Variables:
*  random_backup - modified when non-retention registers are saved.
*
*******************************************************************************/
void random_Sleep(void) 
{
    /* Store PRS enable state */
    if(random_IS_PRS_ENABLE(random_CONTROL_REG))
    {
        random_backup.enableState = 1u;
        random_Stop();
    }
    else
    {
        random_backup.enableState = 0u;
    }
    
    random_SaveConfig();
}


/*******************************************************************************
* Function Name: random_RestoreConfig
********************************************************************************
*
* Summary:
*  Restores seed and polynomial registers.
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
#if (random_WAKEUP_BEHAVIOUR == random__RESUMEWORK)
    #if (random_TIME_MULTIPLEXING_ENABLE)
        void random_RestoreConfig(void) 
        {   
            /* Restore A0 and A1 registers */
            #if (random_PRS_SIZE <= 32u)
                random_WriteSeed(random_backup.seed);
            #else
                random_WriteSeedUpper(random_backup.seedUpper);
                random_WriteSeedLower(random_backup.seedLower);
            #endif  /* End (random_PRS_SIZE <= 32u) */
            
            #if (random_RUN_MODE == random__CLOCKED)
                #if (random_PRS_SIZE <= 32u)
                    random_ResetSeedInit(random_DEFAULT_SEED);                        
                #else
                    random_ResetSeedInitUpper(random_DEFAULT_SEED_UPPER);
                    random_ResetSeedInitLower(random_DEFAULT_SEED_LOWER); 
                #endif  /* End (random_PRS_SIZE <= 32u) */ 
            #endif  /* End (random_RUN_MODE == random__CLOCKED) */
            
            /* Restore dff state for time mult: use async set/reest */
            /* Set CI, SI, SO, STATE0, STATE1 */
            random_CONTROL_REG = random_backup.dffStatus;
            
            /* Make pulse, to set trigger to defined state */
            random_EXECUTE_DFF_SET;
            
            /* Set normal dff sequence set */
            random_sleepState |= random_NORMAL_SEQUENCE;
        }
        
    #else
        void random_RestoreConfig(void) 
        {   
            /* Restore A0 and A1 registers */
            #if (random_PRS_SIZE <= 32u)
                random_WriteSeed(random_backup.seed);
            #else
                random_WriteSeedUpper(random_backup.seedUpper);
                random_WriteSeedLower(random_backup.seedLower);
            #endif  /* End (random_PRS_SIZE <= 32u) */
            
            #if (random_RUN_MODE == random__CLOCKED)
                #if (random_PRS_SIZE <= 32u)
                    random_ResetSeedInit(random_DEFAULT_SEED);                        
                #else
                    random_ResetSeedInitUpper(random_DEFAULT_SEED_UPPER);
                    random_ResetSeedInitLower(random_DEFAULT_SEED_LOWER); 
                #endif  /* End (random_PRS_SIZE <= 32u) */ 
            #endif  /* End (random_RUN_MODE == random__CLOCKED) */
        }
        
    #endif  /* End (random_TIME_MULTIPLEXING_ENABLE) */
    
#else
    void random_RestoreConfig(void) 
    {
        random_Init();
    }
    
#endif  /* End (random_WAKEUP_BEHAVIOUR == random__RESUMEWORK) */


/*******************************************************************************
* Function Name: random_Wakeup
********************************************************************************
*
* Summary:
*  Restores PRS configuration and starts PRS computation. 
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
#if ((random_WAKEUP_BEHAVIOUR == random__RESUMEWORK) && (random_TIME_MULTIPLEXING_ENABLE))
    void random_Wakeup(void) 
#else
    void random_Wakeup(void) 
#endif  /* End ((random_WAKEUP_BEHAVIOUR == random__RESUMEWORK) && 
                (random_TIME_MULTIPLEXING_ENABLE)) */
{
    random_RestoreConfig();
    
    /* Restore PRS enable state */
    if (random_backup.enableState != 0u)
    {
        random_Enable();
    }
}

/* [] END OF FILE */
