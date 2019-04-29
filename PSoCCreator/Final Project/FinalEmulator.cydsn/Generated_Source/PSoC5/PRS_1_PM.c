/*******************************************************************************
* File Name: prs_1_PM.c
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

#include "prs_1.h"

#if ((prs_1_TIME_MULTIPLEXING_ENABLE) && (prs_1_WAKEUP_BEHAVIOUR == prs_1__RESUMEWORK))
    uint8 prs_1_sleepState = prs_1_NORMAL_SEQUENCE;    
#endif  /* End ((prs_1_TIME_MULTIPLEXING_ENABLE) && 
          (prs_1_WAKEUP_BEHAVIOUR == prs_1__RESUMEWORK)) */


/*******************************************************************************
* Function Name: prs_1_SaveConfig
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
*  prs_1_backup - modified when non-retention registers are saved.
*
*******************************************************************************/
void prs_1_SaveConfig(void) 
{    
    #if (prs_1_WAKEUP_BEHAVIOUR == prs_1__RESUMEWORK)
        /* Save dff register for time mult */
        #if (prs_1_TIME_MULTIPLEXING_ENABLE)
            prs_1_backup.dffStatus = prs_1_STATUS;
            /* Clear normal dff sequence set */
            prs_1_sleepState &= ((uint8)~prs_1_NORMAL_SEQUENCE);
        #endif  /* End prs_1_TIME_MULTIPLEXING_ENABLE */
        
        /* Save A0 and A1 registers */
        #if (prs_1_PRS_SIZE <= 32u)
            prs_1_backup.seed = prs_1_Read();
            
        #else
            prs_1_backup.seedUpper = prs_1_ReadUpper();
            prs_1_backup.seedLower = prs_1_ReadLower();
            
        #endif     /* End (prs_1_PRS_SIZE <= 32u) */
        
    #endif  /* End (prs_1_WAKEUP_BEHAVIOUR == prs_1__RESUMEWORK) */
}


/*******************************************************************************
* Function Name: prs_1_Sleep
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
*  prs_1_backup - modified when non-retention registers are saved.
*
*******************************************************************************/
void prs_1_Sleep(void) 
{
    /* Store PRS enable state */
    if(prs_1_IS_PRS_ENABLE(prs_1_CONTROL_REG))
    {
        prs_1_backup.enableState = 1u;
        prs_1_Stop();
    }
    else
    {
        prs_1_backup.enableState = 0u;
    }
    
    prs_1_SaveConfig();
}


/*******************************************************************************
* Function Name: prs_1_RestoreConfig
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
#if (prs_1_WAKEUP_BEHAVIOUR == prs_1__RESUMEWORK)
    #if (prs_1_TIME_MULTIPLEXING_ENABLE)
        void prs_1_RestoreConfig(void) 
        {   
            /* Restore A0 and A1 registers */
            #if (prs_1_PRS_SIZE <= 32u)
                prs_1_WriteSeed(prs_1_backup.seed);
            #else
                prs_1_WriteSeedUpper(prs_1_backup.seedUpper);
                prs_1_WriteSeedLower(prs_1_backup.seedLower);
            #endif  /* End (prs_1_PRS_SIZE <= 32u) */
            
            #if (prs_1_RUN_MODE == prs_1__CLOCKED)
                #if (prs_1_PRS_SIZE <= 32u)
                    prs_1_ResetSeedInit(prs_1_DEFAULT_SEED);                        
                #else
                    prs_1_ResetSeedInitUpper(prs_1_DEFAULT_SEED_UPPER);
                    prs_1_ResetSeedInitLower(prs_1_DEFAULT_SEED_LOWER); 
                #endif  /* End (prs_1_PRS_SIZE <= 32u) */ 
            #endif  /* End (prs_1_RUN_MODE == prs_1__CLOCKED) */
            
            /* Restore dff state for time mult: use async set/reest */
            /* Set CI, SI, SO, STATE0, STATE1 */
            prs_1_CONTROL_REG = prs_1_backup.dffStatus;
            
            /* Make pulse, to set trigger to defined state */
            prs_1_EXECUTE_DFF_SET;
            
            /* Set normal dff sequence set */
            prs_1_sleepState |= prs_1_NORMAL_SEQUENCE;
        }
        
    #else
        void prs_1_RestoreConfig(void) 
        {   
            /* Restore A0 and A1 registers */
            #if (prs_1_PRS_SIZE <= 32u)
                prs_1_WriteSeed(prs_1_backup.seed);
            #else
                prs_1_WriteSeedUpper(prs_1_backup.seedUpper);
                prs_1_WriteSeedLower(prs_1_backup.seedLower);
            #endif  /* End (prs_1_PRS_SIZE <= 32u) */
            
            #if (prs_1_RUN_MODE == prs_1__CLOCKED)
                #if (prs_1_PRS_SIZE <= 32u)
                    prs_1_ResetSeedInit(prs_1_DEFAULT_SEED);                        
                #else
                    prs_1_ResetSeedInitUpper(prs_1_DEFAULT_SEED_UPPER);
                    prs_1_ResetSeedInitLower(prs_1_DEFAULT_SEED_LOWER); 
                #endif  /* End (prs_1_PRS_SIZE <= 32u) */ 
            #endif  /* End (prs_1_RUN_MODE == prs_1__CLOCKED) */
        }
        
    #endif  /* End (prs_1_TIME_MULTIPLEXING_ENABLE) */
    
#else
    void prs_1_RestoreConfig(void) 
    {
        prs_1_Init();
    }
    
#endif  /* End (prs_1_WAKEUP_BEHAVIOUR == prs_1__RESUMEWORK) */


/*******************************************************************************
* Function Name: prs_1_Wakeup
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
#if ((prs_1_WAKEUP_BEHAVIOUR == prs_1__RESUMEWORK) && (prs_1_TIME_MULTIPLEXING_ENABLE))
    void prs_1_Wakeup(void) 
#else
    void prs_1_Wakeup(void) 
#endif  /* End ((prs_1_WAKEUP_BEHAVIOUR == prs_1__RESUMEWORK) && 
                (prs_1_TIME_MULTIPLEXING_ENABLE)) */
{
    prs_1_RestoreConfig();
    
    /* Restore PRS enable state */
    if (prs_1_backup.enableState != 0u)
    {
        prs_1_Enable();
    }
}

/* [] END OF FILE */
