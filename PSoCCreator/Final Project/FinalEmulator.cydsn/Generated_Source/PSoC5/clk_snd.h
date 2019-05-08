/*******************************************************************************
* File Name: clk_snd.h
* Version 2.20
*
*  Description:
*   Provides the function and constant definitions for the clock component.
*
*  Note:
*
********************************************************************************
* Copyright 2008-2012, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_CLOCK_clk_snd_H)
#define CY_CLOCK_clk_snd_H

#include <cytypes.h>
#include <cyfitter.h>


/***************************************
* Conditional Compilation Parameters
***************************************/

/* Check to see if required defines such as CY_PSOC5LP are available */
/* They are defined starting with cy_boot v3.0 */
#if !defined (CY_PSOC5LP)
    #error Component cy_clock_v2_20 requires cy_boot v3.0 or later
#endif /* (CY_PSOC5LP) */


/***************************************
*        Function Prototypes
***************************************/

void clk_snd_Start(void) ;
void clk_snd_Stop(void) ;

#if(CY_PSOC3 || CY_PSOC5LP)
void clk_snd_StopBlock(void) ;
#endif /* (CY_PSOC3 || CY_PSOC5LP) */

void clk_snd_StandbyPower(uint8 state) ;
void clk_snd_SetDividerRegister(uint16 clkDivider, uint8 restart) 
                                ;
uint16 clk_snd_GetDividerRegister(void) ;
void clk_snd_SetModeRegister(uint8 modeBitMask) ;
void clk_snd_ClearModeRegister(uint8 modeBitMask) ;
uint8 clk_snd_GetModeRegister(void) ;
void clk_snd_SetSourceRegister(uint8 clkSource) ;
uint8 clk_snd_GetSourceRegister(void) ;
#if defined(clk_snd__CFG3)
void clk_snd_SetPhaseRegister(uint8 clkPhase) ;
uint8 clk_snd_GetPhaseRegister(void) ;
#endif /* defined(clk_snd__CFG3) */

#define clk_snd_Enable()                       clk_snd_Start()
#define clk_snd_Disable()                      clk_snd_Stop()
#define clk_snd_SetDivider(clkDivider)         clk_snd_SetDividerRegister(clkDivider, 1u)
#define clk_snd_SetDividerValue(clkDivider)    clk_snd_SetDividerRegister((clkDivider) - 1u, 1u)
#define clk_snd_SetMode(clkMode)               clk_snd_SetModeRegister(clkMode)
#define clk_snd_SetSource(clkSource)           clk_snd_SetSourceRegister(clkSource)
#if defined(clk_snd__CFG3)
#define clk_snd_SetPhase(clkPhase)             clk_snd_SetPhaseRegister(clkPhase)
#define clk_snd_SetPhaseValue(clkPhase)        clk_snd_SetPhaseRegister((clkPhase) + 1u)
#endif /* defined(clk_snd__CFG3) */


/***************************************
*             Registers
***************************************/

/* Register to enable or disable the clock */
#define clk_snd_CLKEN              (* (reg8 *) clk_snd__PM_ACT_CFG)
#define clk_snd_CLKEN_PTR          ((reg8 *) clk_snd__PM_ACT_CFG)

/* Register to enable or disable the clock */
#define clk_snd_CLKSTBY            (* (reg8 *) clk_snd__PM_STBY_CFG)
#define clk_snd_CLKSTBY_PTR        ((reg8 *) clk_snd__PM_STBY_CFG)

/* Clock LSB divider configuration register. */
#define clk_snd_DIV_LSB            (* (reg8 *) clk_snd__CFG0)
#define clk_snd_DIV_LSB_PTR        ((reg8 *) clk_snd__CFG0)
#define clk_snd_DIV_PTR            ((reg16 *) clk_snd__CFG0)

/* Clock MSB divider configuration register. */
#define clk_snd_DIV_MSB            (* (reg8 *) clk_snd__CFG1)
#define clk_snd_DIV_MSB_PTR        ((reg8 *) clk_snd__CFG1)

/* Mode and source configuration register */
#define clk_snd_MOD_SRC            (* (reg8 *) clk_snd__CFG2)
#define clk_snd_MOD_SRC_PTR        ((reg8 *) clk_snd__CFG2)

#if defined(clk_snd__CFG3)
/* Analog clock phase configuration register */
#define clk_snd_PHASE              (* (reg8 *) clk_snd__CFG3)
#define clk_snd_PHASE_PTR          ((reg8 *) clk_snd__CFG3)
#endif /* defined(clk_snd__CFG3) */


/**************************************
*       Register Constants
**************************************/

/* Power manager register masks */
#define clk_snd_CLKEN_MASK         clk_snd__PM_ACT_MSK
#define clk_snd_CLKSTBY_MASK       clk_snd__PM_STBY_MSK

/* CFG2 field masks */
#define clk_snd_SRC_SEL_MSK        clk_snd__CFG2_SRC_SEL_MASK
#define clk_snd_MODE_MASK          (~(clk_snd_SRC_SEL_MSK))

#if defined(clk_snd__CFG3)
/* CFG3 phase mask */
#define clk_snd_PHASE_MASK         clk_snd__CFG3_PHASE_DLY_MASK
#endif /* defined(clk_snd__CFG3) */

#endif /* CY_CLOCK_clk_snd_H */


/* [] END OF FILE */
