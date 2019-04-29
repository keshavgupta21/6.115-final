/*******************************************************************************
* File Name: clk_timer.h
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

#if !defined(CY_CLOCK_clk_timer_H)
#define CY_CLOCK_clk_timer_H

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

void clk_timer_Start(void) ;
void clk_timer_Stop(void) ;

#if(CY_PSOC3 || CY_PSOC5LP)
void clk_timer_StopBlock(void) ;
#endif /* (CY_PSOC3 || CY_PSOC5LP) */

void clk_timer_StandbyPower(uint8 state) ;
void clk_timer_SetDividerRegister(uint16 clkDivider, uint8 restart) 
                                ;
uint16 clk_timer_GetDividerRegister(void) ;
void clk_timer_SetModeRegister(uint8 modeBitMask) ;
void clk_timer_ClearModeRegister(uint8 modeBitMask) ;
uint8 clk_timer_GetModeRegister(void) ;
void clk_timer_SetSourceRegister(uint8 clkSource) ;
uint8 clk_timer_GetSourceRegister(void) ;
#if defined(clk_timer__CFG3)
void clk_timer_SetPhaseRegister(uint8 clkPhase) ;
uint8 clk_timer_GetPhaseRegister(void) ;
#endif /* defined(clk_timer__CFG3) */

#define clk_timer_Enable()                       clk_timer_Start()
#define clk_timer_Disable()                      clk_timer_Stop()
#define clk_timer_SetDivider(clkDivider)         clk_timer_SetDividerRegister(clkDivider, 1u)
#define clk_timer_SetDividerValue(clkDivider)    clk_timer_SetDividerRegister((clkDivider) - 1u, 1u)
#define clk_timer_SetMode(clkMode)               clk_timer_SetModeRegister(clkMode)
#define clk_timer_SetSource(clkSource)           clk_timer_SetSourceRegister(clkSource)
#if defined(clk_timer__CFG3)
#define clk_timer_SetPhase(clkPhase)             clk_timer_SetPhaseRegister(clkPhase)
#define clk_timer_SetPhaseValue(clkPhase)        clk_timer_SetPhaseRegister((clkPhase) + 1u)
#endif /* defined(clk_timer__CFG3) */


/***************************************
*             Registers
***************************************/

/* Register to enable or disable the clock */
#define clk_timer_CLKEN              (* (reg8 *) clk_timer__PM_ACT_CFG)
#define clk_timer_CLKEN_PTR          ((reg8 *) clk_timer__PM_ACT_CFG)

/* Register to enable or disable the clock */
#define clk_timer_CLKSTBY            (* (reg8 *) clk_timer__PM_STBY_CFG)
#define clk_timer_CLKSTBY_PTR        ((reg8 *) clk_timer__PM_STBY_CFG)

/* Clock LSB divider configuration register. */
#define clk_timer_DIV_LSB            (* (reg8 *) clk_timer__CFG0)
#define clk_timer_DIV_LSB_PTR        ((reg8 *) clk_timer__CFG0)
#define clk_timer_DIV_PTR            ((reg16 *) clk_timer__CFG0)

/* Clock MSB divider configuration register. */
#define clk_timer_DIV_MSB            (* (reg8 *) clk_timer__CFG1)
#define clk_timer_DIV_MSB_PTR        ((reg8 *) clk_timer__CFG1)

/* Mode and source configuration register */
#define clk_timer_MOD_SRC            (* (reg8 *) clk_timer__CFG2)
#define clk_timer_MOD_SRC_PTR        ((reg8 *) clk_timer__CFG2)

#if defined(clk_timer__CFG3)
/* Analog clock phase configuration register */
#define clk_timer_PHASE              (* (reg8 *) clk_timer__CFG3)
#define clk_timer_PHASE_PTR          ((reg8 *) clk_timer__CFG3)
#endif /* defined(clk_timer__CFG3) */


/**************************************
*       Register Constants
**************************************/

/* Power manager register masks */
#define clk_timer_CLKEN_MASK         clk_timer__PM_ACT_MSK
#define clk_timer_CLKSTBY_MASK       clk_timer__PM_STBY_MSK

/* CFG2 field masks */
#define clk_timer_SRC_SEL_MSK        clk_timer__CFG2_SRC_SEL_MASK
#define clk_timer_MODE_MASK          (~(clk_timer_SRC_SEL_MSK))

#if defined(clk_timer__CFG3)
/* CFG3 phase mask */
#define clk_timer_PHASE_MASK         clk_timer__CFG3_PHASE_DLY_MASK
#endif /* defined(clk_timer__CFG3) */

#endif /* CY_CLOCK_clk_timer_H */


/* [] END OF FILE */
