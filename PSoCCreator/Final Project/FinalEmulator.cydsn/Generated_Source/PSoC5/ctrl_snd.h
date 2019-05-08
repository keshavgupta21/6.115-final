/*******************************************************************************
* File Name: ctrl_snd.h  
* Version 1.80
*
* Description:
*  This file containts Control Register function prototypes and register defines
*
* Note:
*
********************************************************************************
* Copyright 2008-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_CONTROL_REG_ctrl_snd_H) /* CY_CONTROL_REG_ctrl_snd_H */
#define CY_CONTROL_REG_ctrl_snd_H

#include "cyfitter.h"

#if ((CYDEV_CHIP_FAMILY_USED == CYDEV_CHIP_FAMILY_PSOC3) || \
     (CYDEV_CHIP_FAMILY_USED == CYDEV_CHIP_FAMILY_PSOC4) || \
     (CYDEV_CHIP_FAMILY_USED == CYDEV_CHIP_FAMILY_PSOC5))
    #include "cytypes.h"
#else
    #include "syslib/cy_syslib.h"
#endif

    
/***************************************
*     Data Struct Definitions
***************************************/

/* Sleep Mode API Support */
typedef struct
{
    uint8 controlState;

} ctrl_snd_BACKUP_STRUCT;


/***************************************
*         Function Prototypes 
***************************************/

void    ctrl_snd_Write(uint8 control) ;
uint8   ctrl_snd_Read(void) ;

void ctrl_snd_SaveConfig(void) ;
void ctrl_snd_RestoreConfig(void) ;
void ctrl_snd_Sleep(void) ; 
void ctrl_snd_Wakeup(void) ;


/***************************************
*            Registers        
***************************************/

/* Control Register */
#define ctrl_snd_Control        (* (reg8 *) ctrl_snd_Sync_ctrl_reg__CONTROL_REG )
#define ctrl_snd_Control_PTR    (  (reg8 *) ctrl_snd_Sync_ctrl_reg__CONTROL_REG )

#endif /* End CY_CONTROL_REG_ctrl_snd_H */


/* [] END OF FILE */
