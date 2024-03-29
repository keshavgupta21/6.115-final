/*******************************************************************************
* File Name: pwm_snd.c
* Version 3.30
*
* Description:
*  The PWM User Module consist of an 8 or 16-bit counter with two 8 or 16-bit
*  comparitors. Each instance of this user module is capable of generating
*  two PWM outputs with the same period. The pulse width is selectable between
*  1 and 255/65535. The period is selectable between 2 and 255/65536 clocks.
*  The compare value output may be configured to be active when the present
*  counter is less than or less than/equal to the compare value.
*  A terminal count output is also provided. It generates a pulse one clock
*  width wide when the counter is equal to zero.
*
* Note:
*
*******************************************************************************
* Copyright 2008-2014, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
********************************************************************************/

#include "pwm_snd.h"

/* Error message for removed <resource> through optimization */
#ifdef pwm_snd_PWMUDB_genblk1_ctrlreg__REMOVED
    #error PWM_v3_30 detected with a constant 0 for the enable or \
         constant 1 for reset. This will prevent the component from operating.
#endif /* pwm_snd_PWMUDB_genblk1_ctrlreg__REMOVED */

uint8 pwm_snd_initVar = 0u;


/*******************************************************************************
* Function Name: pwm_snd_Start
********************************************************************************
*
* Summary:
*  The start function initializes the pwm with the default values, the
*  enables the counter to begin counting.  It does not enable interrupts,
*  the EnableInt command should be called if interrupt generation is required.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Global variables:
*  pwm_snd_initVar: Is modified when this function is called for the
*   first time. Is used to ensure that initialization happens only once.
*
*******************************************************************************/
void pwm_snd_Start(void) 
{
    /* If not Initialized then initialize all required hardware and software */
    if(pwm_snd_initVar == 0u)
    {
        pwm_snd_Init();
        pwm_snd_initVar = 1u;
    }
    pwm_snd_Enable();

}


/*******************************************************************************
* Function Name: pwm_snd_Init
********************************************************************************
*
* Summary:
*  Initialize component's parameters to the parameters set by user in the
*  customizer of the component placed onto schematic. Usually called in
*  pwm_snd_Start().
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void pwm_snd_Init(void) 
{
    #if (pwm_snd_UsingFixedFunction || pwm_snd_UseControl)
        uint8 ctrl;
    #endif /* (pwm_snd_UsingFixedFunction || pwm_snd_UseControl) */

    #if(!pwm_snd_UsingFixedFunction)
        #if(pwm_snd_UseStatus)
            /* Interrupt State Backup for Critical Region*/
            uint8 pwm_snd_interruptState;
        #endif /* (pwm_snd_UseStatus) */
    #endif /* (!pwm_snd_UsingFixedFunction) */

    #if (pwm_snd_UsingFixedFunction)
        /* You are allowed to write the compare value (FF only) */
        pwm_snd_CONTROL |= pwm_snd_CFG0_MODE;
        #if (pwm_snd_DeadBand2_4)
            pwm_snd_CONTROL |= pwm_snd_CFG0_DB;
        #endif /* (pwm_snd_DeadBand2_4) */

        ctrl = pwm_snd_CONTROL3 & ((uint8 )(~pwm_snd_CTRL_CMPMODE1_MASK));
        pwm_snd_CONTROL3 = ctrl | pwm_snd_DEFAULT_COMPARE1_MODE;

         /* Clear and Set SYNCTC and SYNCCMP bits of RT1 register */
        pwm_snd_RT1 &= ((uint8)(~pwm_snd_RT1_MASK));
        pwm_snd_RT1 |= pwm_snd_SYNC;

        /*Enable DSI Sync all all inputs of the PWM*/
        pwm_snd_RT1 &= ((uint8)(~pwm_snd_SYNCDSI_MASK));
        pwm_snd_RT1 |= pwm_snd_SYNCDSI_EN;

    #elif (pwm_snd_UseControl)
        /* Set the default compare mode defined in the parameter */
        ctrl = pwm_snd_CONTROL & ((uint8)(~pwm_snd_CTRL_CMPMODE2_MASK)) &
                ((uint8)(~pwm_snd_CTRL_CMPMODE1_MASK));
        pwm_snd_CONTROL = ctrl | pwm_snd_DEFAULT_COMPARE2_MODE |
                                   pwm_snd_DEFAULT_COMPARE1_MODE;
    #endif /* (pwm_snd_UsingFixedFunction) */

    #if (!pwm_snd_UsingFixedFunction)
        #if (pwm_snd_Resolution == 8)
            /* Set FIFO 0 to 1 byte register for period*/
            pwm_snd_AUX_CONTROLDP0 |= (pwm_snd_AUX_CTRL_FIFO0_CLR);
        #else /* (pwm_snd_Resolution == 16)*/
            /* Set FIFO 0 to 1 byte register for period */
            pwm_snd_AUX_CONTROLDP0 |= (pwm_snd_AUX_CTRL_FIFO0_CLR);
            pwm_snd_AUX_CONTROLDP1 |= (pwm_snd_AUX_CTRL_FIFO0_CLR);
        #endif /* (pwm_snd_Resolution == 8) */

        pwm_snd_WriteCounter(pwm_snd_INIT_PERIOD_VALUE);
    #endif /* (!pwm_snd_UsingFixedFunction) */

    pwm_snd_WritePeriod(pwm_snd_INIT_PERIOD_VALUE);

        #if (pwm_snd_UseOneCompareMode)
            pwm_snd_WriteCompare(pwm_snd_INIT_COMPARE_VALUE1);
        #else
            pwm_snd_WriteCompare1(pwm_snd_INIT_COMPARE_VALUE1);
            pwm_snd_WriteCompare2(pwm_snd_INIT_COMPARE_VALUE2);
        #endif /* (pwm_snd_UseOneCompareMode) */

        #if (pwm_snd_KillModeMinTime)
            pwm_snd_WriteKillTime(pwm_snd_MinimumKillTime);
        #endif /* (pwm_snd_KillModeMinTime) */

        #if (pwm_snd_DeadBandUsed)
            pwm_snd_WriteDeadTime(pwm_snd_INIT_DEAD_TIME);
        #endif /* (pwm_snd_DeadBandUsed) */

    #if (pwm_snd_UseStatus || pwm_snd_UsingFixedFunction)
        pwm_snd_SetInterruptMode(pwm_snd_INIT_INTERRUPTS_MODE);
    #endif /* (pwm_snd_UseStatus || pwm_snd_UsingFixedFunction) */

    #if (pwm_snd_UsingFixedFunction)
        /* Globally Enable the Fixed Function Block chosen */
        pwm_snd_GLOBAL_ENABLE |= pwm_snd_BLOCK_EN_MASK;
        /* Set the Interrupt source to come from the status register */
        pwm_snd_CONTROL2 |= pwm_snd_CTRL2_IRQ_SEL;
    #else
        #if(pwm_snd_UseStatus)

            /* CyEnterCriticalRegion and CyExitCriticalRegion are used to mark following region critical*/
            /* Enter Critical Region*/
            pwm_snd_interruptState = CyEnterCriticalSection();
            /* Use the interrupt output of the status register for IRQ output */
            pwm_snd_STATUS_AUX_CTRL |= pwm_snd_STATUS_ACTL_INT_EN_MASK;

             /* Exit Critical Region*/
            CyExitCriticalSection(pwm_snd_interruptState);

            /* Clear the FIFO to enable the pwm_snd_STATUS_FIFOFULL
                   bit to be set on FIFO full. */
            pwm_snd_ClearFIFO();
        #endif /* (pwm_snd_UseStatus) */
    #endif /* (pwm_snd_UsingFixedFunction) */
}


/*******************************************************************************
* Function Name: pwm_snd_Enable
********************************************************************************
*
* Summary:
*  Enables the PWM block operation
*
* Parameters:
*  None
*
* Return:
*  None
*
* Side Effects:
*  This works only if software enable mode is chosen
*
*******************************************************************************/
void pwm_snd_Enable(void) 
{
    /* Globally Enable the Fixed Function Block chosen */
    #if (pwm_snd_UsingFixedFunction)
        pwm_snd_GLOBAL_ENABLE |= pwm_snd_BLOCK_EN_MASK;
        pwm_snd_GLOBAL_STBY_ENABLE |= pwm_snd_BLOCK_STBY_EN_MASK;
    #endif /* (pwm_snd_UsingFixedFunction) */

    /* Enable the PWM from the control register  */
    #if (pwm_snd_UseControl || pwm_snd_UsingFixedFunction)
        pwm_snd_CONTROL |= pwm_snd_CTRL_ENABLE;
    #endif /* (pwm_snd_UseControl || pwm_snd_UsingFixedFunction) */
}


/*******************************************************************************
* Function Name: pwm_snd_Stop
********************************************************************************
*
* Summary:
*  The stop function halts the PWM, but does not change any modes or disable
*  interrupts.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Side Effects:
*  If the Enable mode is set to Hardware only then this function
*  has no effect on the operation of the PWM
*
*******************************************************************************/
void pwm_snd_Stop(void) 
{
    #if (pwm_snd_UseControl || pwm_snd_UsingFixedFunction)
        pwm_snd_CONTROL &= ((uint8)(~pwm_snd_CTRL_ENABLE));
    #endif /* (pwm_snd_UseControl || pwm_snd_UsingFixedFunction) */

    /* Globally disable the Fixed Function Block chosen */
    #if (pwm_snd_UsingFixedFunction)
        pwm_snd_GLOBAL_ENABLE &= ((uint8)(~pwm_snd_BLOCK_EN_MASK));
        pwm_snd_GLOBAL_STBY_ENABLE &= ((uint8)(~pwm_snd_BLOCK_STBY_EN_MASK));
    #endif /* (pwm_snd_UsingFixedFunction) */
}

#if (pwm_snd_UseOneCompareMode)
    #if (pwm_snd_CompareMode1SW)


        /*******************************************************************************
        * Function Name: pwm_snd_SetCompareMode
        ********************************************************************************
        *
        * Summary:
        *  This function writes the Compare Mode for the pwm output when in Dither mode,
        *  Center Align Mode or One Output Mode.
        *
        * Parameters:
        *  comparemode:  The new compare mode for the PWM output. Use the compare types
        *                defined in the H file as input arguments.
        *
        * Return:
        *  None
        *
        *******************************************************************************/
        void pwm_snd_SetCompareMode(uint8 comparemode) 
        {
            #if(pwm_snd_UsingFixedFunction)

                #if(0 != pwm_snd_CTRL_CMPMODE1_SHIFT)
                    uint8 comparemodemasked = ((uint8)((uint8)comparemode << pwm_snd_CTRL_CMPMODE1_SHIFT));
                #else
                    uint8 comparemodemasked = comparemode;
                #endif /* (0 != pwm_snd_CTRL_CMPMODE1_SHIFT) */

                pwm_snd_CONTROL3 &= ((uint8)(~pwm_snd_CTRL_CMPMODE1_MASK)); /*Clear Existing Data */
                pwm_snd_CONTROL3 |= comparemodemasked;

            #elif (pwm_snd_UseControl)

                #if(0 != pwm_snd_CTRL_CMPMODE1_SHIFT)
                    uint8 comparemode1masked = ((uint8)((uint8)comparemode << pwm_snd_CTRL_CMPMODE1_SHIFT)) &
                                                pwm_snd_CTRL_CMPMODE1_MASK;
                #else
                    uint8 comparemode1masked = comparemode & pwm_snd_CTRL_CMPMODE1_MASK;
                #endif /* (0 != pwm_snd_CTRL_CMPMODE1_SHIFT) */

                #if(0 != pwm_snd_CTRL_CMPMODE2_SHIFT)
                    uint8 comparemode2masked = ((uint8)((uint8)comparemode << pwm_snd_CTRL_CMPMODE2_SHIFT)) &
                                               pwm_snd_CTRL_CMPMODE2_MASK;
                #else
                    uint8 comparemode2masked = comparemode & pwm_snd_CTRL_CMPMODE2_MASK;
                #endif /* (0 != pwm_snd_CTRL_CMPMODE2_SHIFT) */

                /*Clear existing mode */
                pwm_snd_CONTROL &= ((uint8)(~(pwm_snd_CTRL_CMPMODE1_MASK |
                                            pwm_snd_CTRL_CMPMODE2_MASK)));
                pwm_snd_CONTROL |= (comparemode1masked | comparemode2masked);

            #else
                uint8 temp = comparemode;
            #endif /* (pwm_snd_UsingFixedFunction) */
        }
    #endif /* pwm_snd_CompareMode1SW */

#else /* UseOneCompareMode */

    #if (pwm_snd_CompareMode1SW)


        /*******************************************************************************
        * Function Name: pwm_snd_SetCompareMode1
        ********************************************************************************
        *
        * Summary:
        *  This function writes the Compare Mode for the pwm or pwm1 output
        *
        * Parameters:
        *  comparemode:  The new compare mode for the PWM output. Use the compare types
        *                defined in the H file as input arguments.
        *
        * Return:
        *  None
        *
        *******************************************************************************/
        void pwm_snd_SetCompareMode1(uint8 comparemode) 
        {
            #if(0 != pwm_snd_CTRL_CMPMODE1_SHIFT)
                uint8 comparemodemasked = ((uint8)((uint8)comparemode << pwm_snd_CTRL_CMPMODE1_SHIFT)) &
                                           pwm_snd_CTRL_CMPMODE1_MASK;
            #else
                uint8 comparemodemasked = comparemode & pwm_snd_CTRL_CMPMODE1_MASK;
            #endif /* (0 != pwm_snd_CTRL_CMPMODE1_SHIFT) */

            #if (pwm_snd_UseControl)
                pwm_snd_CONTROL &= ((uint8)(~pwm_snd_CTRL_CMPMODE1_MASK)); /*Clear existing mode */
                pwm_snd_CONTROL |= comparemodemasked;
            #endif /* (pwm_snd_UseControl) */
        }
    #endif /* pwm_snd_CompareMode1SW */

#if (pwm_snd_CompareMode2SW)


    /*******************************************************************************
    * Function Name: pwm_snd_SetCompareMode2
    ********************************************************************************
    *
    * Summary:
    *  This function writes the Compare Mode for the pwm or pwm2 output
    *
    * Parameters:
    *  comparemode:  The new compare mode for the PWM output. Use the compare types
    *                defined in the H file as input arguments.
    *
    * Return:
    *  None
    *
    *******************************************************************************/
    void pwm_snd_SetCompareMode2(uint8 comparemode) 
    {

        #if(0 != pwm_snd_CTRL_CMPMODE2_SHIFT)
            uint8 comparemodemasked = ((uint8)((uint8)comparemode << pwm_snd_CTRL_CMPMODE2_SHIFT)) &
                                                 pwm_snd_CTRL_CMPMODE2_MASK;
        #else
            uint8 comparemodemasked = comparemode & pwm_snd_CTRL_CMPMODE2_MASK;
        #endif /* (0 != pwm_snd_CTRL_CMPMODE2_SHIFT) */

        #if (pwm_snd_UseControl)
            pwm_snd_CONTROL &= ((uint8)(~pwm_snd_CTRL_CMPMODE2_MASK)); /*Clear existing mode */
            pwm_snd_CONTROL |= comparemodemasked;
        #endif /* (pwm_snd_UseControl) */
    }
    #endif /*pwm_snd_CompareMode2SW */

#endif /* UseOneCompareMode */


#if (!pwm_snd_UsingFixedFunction)


    /*******************************************************************************
    * Function Name: pwm_snd_WriteCounter
    ********************************************************************************
    *
    * Summary:
    *  Writes a new counter value directly to the counter register. This will be
    *  implemented for that currently running period and only that period. This API
    *  is valid only for UDB implementation and not available for fixed function
    *  PWM implementation.
    *
    * Parameters:
    *  counter:  The period new period counter value.
    *
    * Return:
    *  None
    *
    * Side Effects:
    *  The PWM Period will be reloaded when a counter value will be a zero
    *
    *******************************************************************************/
    void pwm_snd_WriteCounter(uint8 counter) \
                                       
    {
        CY_SET_REG8(pwm_snd_COUNTER_LSB_PTR, counter);
    }


    /*******************************************************************************
    * Function Name: pwm_snd_ReadCounter
    ********************************************************************************
    *
    * Summary:
    *  This function returns the current value of the counter.  It doesn't matter
    *  if the counter is enabled or running.
    *
    * Parameters:
    *  None
    *
    * Return:
    *  The current value of the counter.
    *
    *******************************************************************************/
    uint8 pwm_snd_ReadCounter(void) 
    {
        /* Force capture by reading Accumulator */
        /* Must first do a software capture to be able to read the counter */
        /* It is up to the user code to make sure there isn't already captured data in the FIFO */
          (void)CY_GET_REG8(pwm_snd_COUNTERCAP_LSB_PTR_8BIT);

        /* Read the data from the FIFO */
        return (CY_GET_REG8(pwm_snd_CAPTURE_LSB_PTR));
    }

    #if (pwm_snd_UseStatus)


        /*******************************************************************************
        * Function Name: pwm_snd_ClearFIFO
        ********************************************************************************
        *
        * Summary:
        *  This function clears all capture data from the capture FIFO
        *
        * Parameters:
        *  None
        *
        * Return:
        *  None
        *
        *******************************************************************************/
        void pwm_snd_ClearFIFO(void) 
        {
            while(0u != (pwm_snd_ReadStatusRegister() & pwm_snd_STATUS_FIFONEMPTY))
            {
                (void)pwm_snd_ReadCapture();
            }
        }

    #endif /* pwm_snd_UseStatus */

#endif /* !pwm_snd_UsingFixedFunction */


/*******************************************************************************
* Function Name: pwm_snd_WritePeriod
********************************************************************************
*
* Summary:
*  This function is used to change the period of the counter.  The new period
*  will be loaded the next time terminal count is detected.
*
* Parameters:
*  period:  Period value. May be between 1 and (2^Resolution)-1.  A value of 0
*           will result in the counter remaining at zero.
*
* Return:
*  None
*
*******************************************************************************/
void pwm_snd_WritePeriod(uint8 period) 
{
    #if(pwm_snd_UsingFixedFunction)
        CY_SET_REG16(pwm_snd_PERIOD_LSB_PTR, (uint16)period);
    #else
        CY_SET_REG8(pwm_snd_PERIOD_LSB_PTR, period);
    #endif /* (pwm_snd_UsingFixedFunction) */
}

#if (pwm_snd_UseOneCompareMode)


    /*******************************************************************************
    * Function Name: pwm_snd_WriteCompare
    ********************************************************************************
    *
    * Summary:
    *  This funtion is used to change the compare1 value when the PWM is in Dither
    *  mode. The compare output will reflect the new value on the next UDB clock.
    *  The compare output will be driven high when the present counter value is
    *  compared to the compare value based on the compare mode defined in
    *  Dither Mode.
    *
    * Parameters:
    *  compare:  New compare value.
    *
    * Return:
    *  None
    *
    * Side Effects:
    *  This function is only available if the PWM mode parameter is set to
    *  Dither Mode, Center Aligned Mode or One Output Mode
    *
    *******************************************************************************/
    void pwm_snd_WriteCompare(uint8 compare) \
                                       
    {
        #if(pwm_snd_UsingFixedFunction)
            CY_SET_REG16(pwm_snd_COMPARE1_LSB_PTR, (uint16)compare);
        #else
            CY_SET_REG8(pwm_snd_COMPARE1_LSB_PTR, compare);
        #endif /* (pwm_snd_UsingFixedFunction) */

        #if (pwm_snd_PWMMode == pwm_snd__B_PWM__DITHER)
            #if(pwm_snd_UsingFixedFunction)
                CY_SET_REG16(pwm_snd_COMPARE2_LSB_PTR, (uint16)(compare + 1u));
            #else
                CY_SET_REG8(pwm_snd_COMPARE2_LSB_PTR, (compare + 1u));
            #endif /* (pwm_snd_UsingFixedFunction) */
        #endif /* (pwm_snd_PWMMode == pwm_snd__B_PWM__DITHER) */
    }


#else


    /*******************************************************************************
    * Function Name: pwm_snd_WriteCompare1
    ********************************************************************************
    *
    * Summary:
    *  This funtion is used to change the compare1 value.  The compare output will
    *  reflect the new value on the next UDB clock.  The compare output will be
    *  driven high when the present counter value is less than or less than or
    *  equal to the compare register, depending on the mode.
    *
    * Parameters:
    *  compare:  New compare value.
    *
    * Return:
    *  None
    *
    *******************************************************************************/
    void pwm_snd_WriteCompare1(uint8 compare) \
                                        
    {
        #if(pwm_snd_UsingFixedFunction)
            CY_SET_REG16(pwm_snd_COMPARE1_LSB_PTR, (uint16)compare);
        #else
            CY_SET_REG8(pwm_snd_COMPARE1_LSB_PTR, compare);
        #endif /* (pwm_snd_UsingFixedFunction) */
    }


    /*******************************************************************************
    * Function Name: pwm_snd_WriteCompare2
    ********************************************************************************
    *
    * Summary:
    *  This funtion is used to change the compare value, for compare1 output.
    *  The compare output will reflect the new value on the next UDB clock.
    *  The compare output will be driven high when the present counter value is
    *  less than or less than or equal to the compare register, depending on the
    *  mode.
    *
    * Parameters:
    *  compare:  New compare value.
    *
    * Return:
    *  None
    *
    *******************************************************************************/
    void pwm_snd_WriteCompare2(uint8 compare) \
                                        
    {
        #if(pwm_snd_UsingFixedFunction)
            CY_SET_REG16(pwm_snd_COMPARE2_LSB_PTR, compare);
        #else
            CY_SET_REG8(pwm_snd_COMPARE2_LSB_PTR, compare);
        #endif /* (pwm_snd_UsingFixedFunction) */
    }
#endif /* UseOneCompareMode */

#if (pwm_snd_DeadBandUsed)


    /*******************************************************************************
    * Function Name: pwm_snd_WriteDeadTime
    ********************************************************************************
    *
    * Summary:
    *  This function writes the dead-band counts to the corresponding register
    *
    * Parameters:
    *  deadtime:  Number of counts for dead time
    *
    * Return:
    *  None
    *
    *******************************************************************************/
    void pwm_snd_WriteDeadTime(uint8 deadtime) 
    {
        /* If using the Dead Band 1-255 mode then just write the register */
        #if(!pwm_snd_DeadBand2_4)
            CY_SET_REG8(pwm_snd_DEADBAND_COUNT_PTR, deadtime);
        #else
            /* Otherwise the data has to be masked and offset */
            /* Clear existing data */
            pwm_snd_DEADBAND_COUNT &= ((uint8)(~pwm_snd_DEADBAND_COUNT_MASK));

            /* Set new dead time */
            #if(pwm_snd_DEADBAND_COUNT_SHIFT)
                pwm_snd_DEADBAND_COUNT |= ((uint8)((uint8)deadtime << pwm_snd_DEADBAND_COUNT_SHIFT)) &
                                                    pwm_snd_DEADBAND_COUNT_MASK;
            #else
                pwm_snd_DEADBAND_COUNT |= deadtime & pwm_snd_DEADBAND_COUNT_MASK;
            #endif /* (pwm_snd_DEADBAND_COUNT_SHIFT) */

        #endif /* (!pwm_snd_DeadBand2_4) */
    }


    /*******************************************************************************
    * Function Name: pwm_snd_ReadDeadTime
    ********************************************************************************
    *
    * Summary:
    *  This function reads the dead-band counts from the corresponding register
    *
    * Parameters:
    *  None
    *
    * Return:
    *  Dead Band Counts
    *
    *******************************************************************************/
    uint8 pwm_snd_ReadDeadTime(void) 
    {
        /* If using the Dead Band 1-255 mode then just read the register */
        #if(!pwm_snd_DeadBand2_4)
            return (CY_GET_REG8(pwm_snd_DEADBAND_COUNT_PTR));
        #else

            /* Otherwise the data has to be masked and offset */
            #if(pwm_snd_DEADBAND_COUNT_SHIFT)
                return ((uint8)(((uint8)(pwm_snd_DEADBAND_COUNT & pwm_snd_DEADBAND_COUNT_MASK)) >>
                                                                           pwm_snd_DEADBAND_COUNT_SHIFT));
            #else
                return (pwm_snd_DEADBAND_COUNT & pwm_snd_DEADBAND_COUNT_MASK);
            #endif /* (pwm_snd_DEADBAND_COUNT_SHIFT) */
        #endif /* (!pwm_snd_DeadBand2_4) */
    }
#endif /* DeadBandUsed */

#if (pwm_snd_UseStatus || pwm_snd_UsingFixedFunction)


    /*******************************************************************************
    * Function Name: pwm_snd_SetInterruptMode
    ********************************************************************************
    *
    * Summary:
    *  This function configures the interrupts mask control of theinterrupt
    *  source status register.
    *
    * Parameters:
    *  uint8 interruptMode: Bit field containing the interrupt sources enabled
    *
    * Return:
    *  None
    *
    *******************************************************************************/
    void pwm_snd_SetInterruptMode(uint8 interruptMode) 
    {
        CY_SET_REG8(pwm_snd_STATUS_MASK_PTR, interruptMode);
    }


    /*******************************************************************************
    * Function Name: pwm_snd_ReadStatusRegister
    ********************************************************************************
    *
    * Summary:
    *  This function returns the current state of the status register.
    *
    * Parameters:
    *  None
    *
    * Return:
    *  uint8 : Current status register value. The status register bits are:
    *  [7:6] : Unused(0)
    *  [5]   : Kill event output
    *  [4]   : FIFO not empty
    *  [3]   : FIFO full
    *  [2]   : Terminal count
    *  [1]   : Compare output 2
    *  [0]   : Compare output 1
    *
    *******************************************************************************/
    uint8 pwm_snd_ReadStatusRegister(void) 
    {
        return (CY_GET_REG8(pwm_snd_STATUS_PTR));
    }

#endif /* (pwm_snd_UseStatus || pwm_snd_UsingFixedFunction) */


#if (pwm_snd_UseControl)


    /*******************************************************************************
    * Function Name: pwm_snd_ReadControlRegister
    ********************************************************************************
    *
    * Summary:
    *  Returns the current state of the control register. This API is available
    *  only if the control register is not removed.
    *
    * Parameters:
    *  None
    *
    * Return:
    *  uint8 : Current control register value
    *
    *******************************************************************************/
    uint8 pwm_snd_ReadControlRegister(void) 
    {
        uint8 result;

        result = CY_GET_REG8(pwm_snd_CONTROL_PTR);
        return (result);
    }


    /*******************************************************************************
    * Function Name: pwm_snd_WriteControlRegister
    ********************************************************************************
    *
    * Summary:
    *  Sets the bit field of the control register. This API is available only if
    *  the control register is not removed.
    *
    * Parameters:
    *  uint8 control: Control register bit field, The status register bits are:
    *  [7]   : PWM Enable
    *  [6]   : Reset
    *  [5:3] : Compare Mode2
    *  [2:0] : Compare Mode2
    *
    * Return:
    *  None
    *
    *******************************************************************************/
    void pwm_snd_WriteControlRegister(uint8 control) 
    {
        CY_SET_REG8(pwm_snd_CONTROL_PTR, control);
    }

#endif /* (pwm_snd_UseControl) */


#if (!pwm_snd_UsingFixedFunction)


    /*******************************************************************************
    * Function Name: pwm_snd_ReadCapture
    ********************************************************************************
    *
    * Summary:
    *  Reads the capture value from the capture FIFO.
    *
    * Parameters:
    *  None
    *
    * Return:
    *  uint8/uint16: The current capture value
    *
    *******************************************************************************/
    uint8 pwm_snd_ReadCapture(void) 
    {
        return (CY_GET_REG8(pwm_snd_CAPTURE_LSB_PTR));
    }

#endif /* (!pwm_snd_UsingFixedFunction) */


#if (pwm_snd_UseOneCompareMode)


    /*******************************************************************************
    * Function Name: pwm_snd_ReadCompare
    ********************************************************************************
    *
    * Summary:
    *  Reads the compare value for the compare output when the PWM Mode parameter is
    *  set to Dither mode, Center Aligned mode, or One Output mode.
    *
    * Parameters:
    *  None
    *
    * Return:
    *  uint8/uint16: Current compare value
    *
    *******************************************************************************/
    uint8 pwm_snd_ReadCompare(void) 
    {
        #if(pwm_snd_UsingFixedFunction)
            return ((uint8)CY_GET_REG16(pwm_snd_COMPARE1_LSB_PTR));
        #else
            return (CY_GET_REG8(pwm_snd_COMPARE1_LSB_PTR));
        #endif /* (pwm_snd_UsingFixedFunction) */
    }

#else


    /*******************************************************************************
    * Function Name: pwm_snd_ReadCompare1
    ********************************************************************************
    *
    * Summary:
    *  Reads the compare value for the compare1 output.
    *
    * Parameters:
    *  None
    *
    * Return:
    *  uint8/uint16: Current compare value.
    *
    *******************************************************************************/
    uint8 pwm_snd_ReadCompare1(void) 
    {
        return (CY_GET_REG8(pwm_snd_COMPARE1_LSB_PTR));
    }


    /*******************************************************************************
    * Function Name: pwm_snd_ReadCompare2
    ********************************************************************************
    *
    * Summary:
    *  Reads the compare value for the compare2 output.
    *
    * Parameters:
    *  None
    *
    * Return:
    *  uint8/uint16: Current compare value.
    *
    *******************************************************************************/
    uint8 pwm_snd_ReadCompare2(void) 
    {
        return (CY_GET_REG8(pwm_snd_COMPARE2_LSB_PTR));
    }

#endif /* (pwm_snd_UseOneCompareMode) */


/*******************************************************************************
* Function Name: pwm_snd_ReadPeriod
********************************************************************************
*
* Summary:
*  Reads the period value used by the PWM hardware.
*
* Parameters:
*  None
*
* Return:
*  uint8/16: Period value
*
*******************************************************************************/
uint8 pwm_snd_ReadPeriod(void) 
{
    #if(pwm_snd_UsingFixedFunction)
        return ((uint8)CY_GET_REG16(pwm_snd_PERIOD_LSB_PTR));
    #else
        return (CY_GET_REG8(pwm_snd_PERIOD_LSB_PTR));
    #endif /* (pwm_snd_UsingFixedFunction) */
}

#if ( pwm_snd_KillModeMinTime)


    /*******************************************************************************
    * Function Name: pwm_snd_WriteKillTime
    ********************************************************************************
    *
    * Summary:
    *  Writes the kill time value used by the hardware when the Kill Mode
    *  is set to Minimum Time.
    *
    * Parameters:
    *  uint8: Minimum Time kill counts
    *
    * Return:
    *  None
    *
    *******************************************************************************/
    void pwm_snd_WriteKillTime(uint8 killtime) 
    {
        CY_SET_REG8(pwm_snd_KILLMODEMINTIME_PTR, killtime);
    }


    /*******************************************************************************
    * Function Name: pwm_snd_ReadKillTime
    ********************************************************************************
    *
    * Summary:
    *  Reads the kill time value used by the hardware when the Kill Mode is set
    *  to Minimum Time.
    *
    * Parameters:
    *  None
    *
    * Return:
    *  uint8: The current Minimum Time kill counts
    *
    *******************************************************************************/
    uint8 pwm_snd_ReadKillTime(void) 
    {
        return (CY_GET_REG8(pwm_snd_KILLMODEMINTIME_PTR));
    }

#endif /* ( pwm_snd_KillModeMinTime) */

/* [] END OF FILE */
