/*******************************************************************************
* File Name: random.c
* Version 2.40
*
* Description:
*  This file provides the source code to the API for the PRS component
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

uint8 random_initVar = 0u;

random_BACKUP_STRUCT random_backup =
{
    0x00u, /* enableState; */

    #if (random_WAKEUP_BEHAVIOUR == random__RESUMEWORK)
        /* Save dff register for time mult */
        #if (random_TIME_MULTIPLEXING_ENABLE)
            random_INIT_STATE, /* dffStatus; */
        #endif  /* End random_TIME_MULTIPLEXING_ENABLE */

        /* Save A0 and A1 registers are none-retention */
        #if(random_PRS_SIZE <= 32u)
            random_DEFAULT_SEED, /* seed */

        #else
            random_DEFAULT_SEED_UPPER, /* seedUpper; */
            random_DEFAULT_SEED_LOWER, /* seedLower; */

        #endif  /* End (random_PRS_SIZE <= 32u) */

    #endif  /* End (random_WAKEUP_BEHAVIOUR == random__RESUMEWORK) */
};


/*******************************************************************************
* Function Name: random_Init
********************************************************************************
*
* Summary:
*  Initializes seed and polynomial registers with initial values.
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void random_Init(void) 
{
    /* Writes seed value and ponynom value provided for customizer */
    #if (random_PRS_SIZE <= 32u)
        random_WritePolynomial(random_DEFAULT_POLYNOM);
        random_WriteSeed(random_DEFAULT_SEED);
        #if (random_RUN_MODE == random__CLOCKED)
            random_ResetSeedInit(random_DEFAULT_SEED);
        #endif  /* End (random_RUN_MODE == random__CLOCKED) */
            random_Enable();
    #else
        random_WritePolynomialUpper(random_DEFAULT_POLYNOM_UPPER);
        random_WritePolynomialLower(random_DEFAULT_POLYNOM_LOWER);
        random_WriteSeedUpper(random_DEFAULT_SEED_UPPER);
        random_WriteSeedLower(random_DEFAULT_SEED_LOWER);
        #if (random_RUN_MODE == random__CLOCKED)
            random_ResetSeedInitUpper(random_DEFAULT_SEED_UPPER);
            random_ResetSeedInitLower(random_DEFAULT_SEED_LOWER);
        #endif  /* End (random_RUN_MODE == random__CLOCKED) */
            random_Enable();
    #endif  /* End (random_PRS_SIZE <= 32u) */
}


/*******************************************************************************
* Function Name: random_Enable
********************************************************************************
*
* Summary:
*  Starts PRS computation on rising edge of input clock.
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void random_Enable(void) 
{
        random_CONTROL_REG |= random_CTRL_ENABLE;
}


/*******************************************************************************
* Function Name: random_Start
********************************************************************************
*
* Summary:
*  Initializes seed and polynomial registers with initial values. Computation
*  of PRS starts on rising edge of input clock.
*
* Parameters:
*  void
*
* Return:
*  void
*
* Global variables:
*  random_initVar: global variable is used to indicate initial
*  configuration of this component.  The variable is initialized to zero and set
*  to 1 the first time random_Start() is called. This allows
*  enable/disable component without re-initialization in all subsequent calls
*  to the random_Start() routine.
*
*******************************************************************************/
void random_Start(void) 
{
    /* Writes seed value and ponynom value provided from customizer */
    if (random_initVar == 0u)
    {
        random_Init();
        random_initVar = 1u;
    }

    random_Enable();
}


/*******************************************************************************
* Function Name: random_Stop
********************************************************************************
*
* Summary:
*  Stops PRS computation.
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void random_Stop(void) 
{
    random_CONTROL_REG &= ((uint8)~random_CTRL_ENABLE);
}


#if (random_RUN_MODE == random__APISINGLESTEP)
    /*******************************************************************************
    * FUNCTION NAME: random_Step
    ********************************************************************************
    *
    * Summary:
    *  Increments the PRS by one when API single step mode is used.
    *
    * Parameters:
    *  void
    *
    * Return:
    *  void
    *
    *******************************************************************************/
    void random_Step(void) 
    {
        #if (random_TIME_MULTIPLEXING_ENABLE)
            /* Makes 4 pulse, 4x for Time Mult */
            random_EXECUTE_STEP;
            random_EXECUTE_STEP;
            random_EXECUTE_STEP;
            random_EXECUTE_STEP;

        #else
            /* One pulse without Time mult required */
            random_EXECUTE_STEP;

        #endif  /* End random_TIME_MULTIPLEXING_ENABLE */
    }
#endif  /* End (random_RUN_MODE == random__APISINGLESTEP) */


#if (random_RUN_MODE == random__CLOCKED)
    #if (random_PRS_SIZE <= 32u) /* 8-32 bits PRS */
        /*******************************************************************************
        * FUNCTION NAME: random_ResetSeedInit
        ********************************************************************************
        *
        * Summary:
        *  Increments the PRS by one when API single step mode is used.
        *
        * Parameters:
        *  void
        *
        * Return:
        *  void
        *
        *******************************************************************************/
        void random_ResetSeedInit(uint8 seed)
                                            
        {
            uint8 enableInterrupts;

            /* Mask the Seed to cut unused bits */
            seed &= random_MASK;

            /* Change AuxControl reg, need to be safety */
            enableInterrupts = CyEnterCriticalSection();

            #if (random_TIME_MULTIPLEXING_ENABLE)
                /* Set FIFOs to single register */
                random_AUX_CONTROL_A_REG |= random_AUXCTRL_FIFO_SINGLE_REG;

                #if(random_PRS_SIZE > 16u)       /* 17-32 bits PRS */
                    random_AUX_CONTROL_B_REG |= random_AUXCTRL_FIFO_SINGLE_REG;
                #endif  /* End (random_PRS_SIZE <= 8u) */

                /* AuxControl reg settings are done */
                CyExitCriticalSection(enableInterrupts);

                /* Write Seed COPY */
                #if (random_PRS_SIZE <= 16u)          /* 16 bits PRS */
                    random_SEED_COPY_A__A1_REG = HI8(seed);
                    random_SEED_COPY_A__A0_REG = LO8(seed);

                #elif (random_PRS_SIZE <= 24u)        /* 24 bits PRS */
                    random_SEED_COPY_B__A1_REG = LO8(HI16(seed));
                    random_SEED_COPY_B__A0_REG = HI8(seed);
                    random_SEED_COPY_A__A0_REG = LO8(seed);

                #else                                           /* 32 bits PRS */
                    random_SEED_COPY_B__A1_REG = HI8(HI16(seed));
                    random_SEED_COPY_A__A1_REG = LO8(HI16(seed));
                    random_SEED_COPY_B__A0_REG = HI8(seed);
                    random_SEED_COPY_A__A0_REG = LO8(seed);
                #endif  /* End (random_PRS_SIZE <= 32u) */

            #else
                /* Set FIFOs to single register */
                #if (random_PRS_SIZE <= 8u)      /* 8 bits PRS */
                    random_AUX_CONTROL_A_REG |= random_AUXCTRL_FIFO_SINGLE_REG;

                #elif (random_PRS_SIZE <= 16u)      /* 16 bits PRS */
                    random_AUX_CONTROL_A_REG  |= random_AUXCTRL_FIFO_SINGLE_REG;
                    random_AUX_CONTROL_B_REG  |= random_AUXCTRL_FIFO_SINGLE_REG;

                #elif (random_PRS_SIZE <= 24u)      /* 24-39 bits PRS */
                    random_AUX_CONTROL_A_REG  |= random_AUXCTRL_FIFO_SINGLE_REG;
                    random_AUX_CONTROL_B_REG  |= random_AUXCTRL_FIFO_SINGLE_REG;
                    random_AUX_CONTROL_C_REG  |= random_AUXCTRL_FIFO_SINGLE_REG;

                #elif (random_PRS_SIZE <= 32u)      /* 40-55 bits PRS */
                    random_AUX_CONTROL_A_REG  |= random_AUXCTRL_FIFO_SINGLE_REG;
                    random_AUX_CONTROL_B_REG  |= random_AUXCTRL_FIFO_SINGLE_REG;
                    random_AUX_CONTROL_C_REG  |= random_AUXCTRL_FIFO_SINGLE_REG;
                    random_AUX_CONTROL_D_REG  |= random_AUXCTRL_FIFO_SINGLE_REG;

                #endif  /* End (random_PRS_SIZE <= 8u) */

                /* AuxControl reg setting are done */
                CyExitCriticalSection(enableInterrupts);

                /* Write Seed COPY */
                CY_SET_REG8(random_SEED_COPY_PTR, seed);

            #endif  /* End (random_TIME_MULTIPLEXING_ENABLE) */
        }

    #else

        /*******************************************************************************
        * FUNCTION NAME: random_ResetSeedInitUpper
        ********************************************************************************
        *
        * Summary:
        *  Increments the PRS by one when API single step mode is used.
        *
        * Parameters:
        *  void
        *
        * Return:
        *  void
        *
        *******************************************************************************/
        void random_ResetSeedInitUpper(uint32 seed) 
        {
			uint8 enableInterrupts;

			/* Mask the Seed Upper half to cut unused bits */
            seed &= random_MASK;

			/* Change AuxControl reg, need to be safety */
            enableInterrupts = CyEnterCriticalSection();

            /* Set FIFOs to single register */
            random_AUX_CONTROL_A_REG |= random_AUXCTRL_FIFO_SINGLE_REG;
            random_AUX_CONTROL_B_REG |= random_AUXCTRL_FIFO_SINGLE_REG;
            random_AUX_CONTROL_C_REG |= random_AUXCTRL_FIFO_SINGLE_REG;

			#if (random_PRS_SIZE > 48u)               /* 49-64 bits PRS */
                random_AUX_CONTROL_D_REG |= random_AUXCTRL_FIFO_SINGLE_REG;
            #endif  /* End (random_PRS_SIZE <= 8u) */

            /* AuxControl reg settings are done */
            CyExitCriticalSection(enableInterrupts);

            /* Write Seed Upper COPY */
            #if (random_PRS_SIZE <= 40u)          /* 40 bits PRS */
                random_SEED_UPPER_COPY_C__A1_REG = LO8(seed);

            #elif (random_PRS_SIZE <= 48u)        /* 48 bits PRS */
                random_SEED_UPPER_COPY_C__A1_REG = HI8(seed);
                random_SEED_UPPER_COPY_B__A1_REG = LO8(seed);

            #elif (random_PRS_SIZE <= 56u)        /* 56 bits PRS */
                random_SEED_UPPER_COPY_D__A1_REG = LO8(HI16(seed));
                random_SEED_UPPER_COPY_C__A1_REG = HI8(seed);
                random_SEED_UPPER_COPY_B__A1_REG = HI8(seed);

            #else                                           /* 64 bits PRS */
                random_SEED_UPPER_COPY_D__A1_REG = HI8(HI16(seed));
                random_SEED_UPPER_COPY_C__A1_REG = LO8(HI16(seed));
                random_SEED_UPPER_COPY_B__A1_REG = HI8(seed);
                random_SEED_UPPER_COPY_A__A1_REG = LO8(seed);

            #endif  /* End (random_PRS_SIZE <= 32u) */
        }


        /*******************************************************************************
        * FUNCTION NAME: random_ResetSeedInitLower
        ********************************************************************************
        *
        * Summary:
        *  Increments the PRS by one when API single step mode is used.
        *
        * Parameters:
        *  void
        *
        * Return:
        *  void
        *
        *******************************************************************************/
        void random_ResetSeedInitLower(uint32 seed) 
        {
            /* Write Seed Lower COPY */
            #if (random_PRS_SIZE <= 40u)          /* 40 bits PRS */
                random_SEED_LOWER_COPY_B__A1_REG = HI8(HI16(seed));
                random_SEED_LOWER_COPY_C__A0_REG = LO8(HI16(seed));
                random_SEED_LOWER_COPY_B__A0_REG = HI8(seed);
                random_SEED_LOWER_COPY_A__A0_REG = LO8(seed);

            #elif (random_PRS_SIZE <= 48u)        /* 48 bits PRS */
                random_SEED_LOWER_COPY_A__A1_REG = HI8(HI16(seed));
                random_SEED_LOWER_COPY_C__A0_REG = LO8(HI16(seed));
                random_SEED_LOWER_COPY_B__A0_REG = HI8(seed);
                random_SEED_LOWER_COPY_A__A0_REG = LO8(seed);

            #else                                           /* 64 bits PRS */
                random_SEED_LOWER_COPY_D__A0_REG = HI8(HI16(seed));
                random_SEED_LOWER_COPY_C__A0_REG = LO8(HI16(seed));
                random_SEED_LOWER_COPY_B__A0_REG = HI8(seed);
                random_SEED_LOWER_COPY_A__A0_REG = LO8(seed);

            #endif  /* End (random_PRS_SIZE <= 32u) */
        }

    #endif  /* End (random_PRS_SIZE <= 32u) */

#endif  /* End (random_RUN_MODE == random__CLOCKED) */


#if(random_PRS_SIZE <= 32u) /* 8-32 bits PRS */
    /*******************************************************************************
    * Function Name: random_Read
    ********************************************************************************
    *
    * Summary:
    *  Reads PRS value.
    *
    * Parameters:
    *  void
    *
    * Return:
    *  Returns PRS value.
    *
    * Side Effects:
    *  The seed value is cut according to mask = 2^(Resolution) - 1.
    *  For example if PRS Resolution is 14 bits the mask value is:
    *  mask = 2^(14) - 1 = 0x3FFFu.
    *  The seed value = 0xFFFFu is cut:
    *  seed & mask = 0xFFFFu & 0x3FFFu = 0x3FFFu.
    *
    *******************************************************************************/
    uint8 random_Read(void) 
    {
        /* Read PRS */
        #if (random_TIME_MULTIPLEXING_ENABLE)

            uint8 seed;

            #if (random_PRS_SIZE <= 16u)          /* 16 bits PRS */
                seed = ((uint16) random_SEED_A__A1_REG) << 8u;
                seed |= random_SEED_A__A0_REG;

            #elif (random_PRS_SIZE <= 24u)        /* 24 bits PRS */
                seed = ((uint32) (random_SEED_B__A1_REG)) << 16u;
                seed |= ((uint32) (random_SEED_B__A0_REG)) << 8u;
                seed |= random_SEED_A__A0_REG;

            #else                                           /* 32 bits PRS */
                seed = ((uint32) random_SEED_B__A1_REG) << 24u;
                seed |= ((uint32) random_SEED_A__A1_REG) << 16u;
                seed |= ((uint32) random_SEED_B__A0_REG) << 8u;
                seed |= random_SEED_A__A0_REG;

            #endif  /* End (random_PRS_SIZE <= 8u) */

            return (seed  & random_MASK);

        #else

            return (CY_GET_REG8(random_SEED_PTR) & random_MASK);

        #endif  /* End (random_TIME_MULTIPLEXING_ENABLE) */
    }


    /*******************************************************************************
    * Function Name: random_WriteSeed
    ********************************************************************************
    *
    * Summary:
    *  Writes seed value.
    *
    * Parameters:
    *  seed:  Seed value.
    *
    * Return:
    *  void
    *
    * Side Effects:
    *  The seed value is cut according to mask = 2^(Resolution) - 1.
    *  For example if PRS Resolution is 14 bits the mask value is:
    *  mask = 2^(14) - 1 = 0x3FFFu.
    *  The seed value = 0xFFFFu is cut:
    *  seed & mask = 0xFFFFu & 0x3FFFu = 0x3FFFu.
    *
    *******************************************************************************/
    void random_WriteSeed(uint8 seed) 
    {
        /* Masks the Seed to cut unused bits */
        seed &= random_MASK;

        /* Writes Seed */
        #if (random_TIME_MULTIPLEXING_ENABLE)

            #if (random_PRS_SIZE <= 16u)          /* 16 bits PRS */
                random_SEED_A__A1_REG = HI8(seed);
                random_SEED_A__A0_REG = LO8(seed);

            #elif (random_PRS_SIZE <= 24u)        /* 24 bits PRS */
                random_SEED_B__A1_REG = LO8(HI16(seed));
                random_SEED_B__A0_REG = HI8(seed);
                random_SEED_A__A0_REG = LO8(seed);

            #else                                           /* 32 bits PRS */
                random_SEED_B__A1_REG = HI8(HI16(seed));
                random_SEED_A__A1_REG = LO8(HI16(seed));
                random_SEED_B__A0_REG = HI8(seed);
                random_SEED_A__A0_REG = LO8(seed);
            #endif  /* End (random_PRS_SIZE <= 32u) */

            /* Resets triggers */
            #if (random_WAKEUP_BEHAVIOUR == random__RESUMEWORK)
                if((random_sleepState & random_NORMAL_SEQUENCE) != 0u)
                {
                    random_EXECUTE_DFF_RESET;
                }
            #else
                random_EXECUTE_DFF_RESET;
            #endif  /* (random_WAKEUP_BEHAVIOUR == random__RESUMEWORK) */

        #else

            CY_SET_REG8(random_SEED_PTR, seed);

        #endif  /* End (random_TIME_MULTIPLEXING_ENABLE) */
    }


    /*******************************************************************************
    * Function Name: random_ReadPolynomial
    ********************************************************************************
    *
    * Summary:
    *  Reads PRS polynomial value.
    *
    * Parameters:
    *  void
    *
    * Return:
    *  Returns PRS polynomial value.
    *
    *******************************************************************************/
    uint8 random_ReadPolynomial(void)
                                
    {
        /* Reads polynomial */
        #if (random_TIME_MULTIPLEXING_ENABLE)

            uint8 polynomial;

            #if (random_PRS_SIZE <= 16u)          /* 16 bits PRS */
                polynomial = ((uint16) random_POLYNOM_A__D1_REG) << 8u;
                polynomial |= (random_POLYNOM_A__D0_REG);

            #elif (random_PRS_SIZE <= 24u)        /* 24 bits PRS */
                polynomial = ((uint32) random_POLYNOM_B__D1_REG) << 16u;
                polynomial |= ((uint32) random_POLYNOM_B__D0_REG) << 8u;
                polynomial |= random_POLYNOM_A__D0_REG;

            #else                                           /* 32 bits PRS */
                polynomial = ((uint32) random_POLYNOM_B__D1_REG) << 24u;
                polynomial |= ((uint32) random_POLYNOM_A__D1_REG) << 16u;
                polynomial |= ((uint32) random_POLYNOM_B__D0_REG) << 8u;
                polynomial |= random_POLYNOM_A__D0_REG;

            #endif  /* End (random_PRS_SIZE <= 32u) */

            return polynomial;

        #else

            return CY_GET_REG8(random_POLYNOM_PTR);

        #endif  /* End (random_TIME_MULTIPLEXING_ENABLE) */
    }


    /*******************************************************************************
    * Function Name: random_WritePolynomial
    ********************************************************************************
    *
    * Summary:
    *  Writes PRS polynomial value.
    *
    * Parameters:
    *  polynomial:  PRS polynomial.
    *
    * Return:
    *  void
    *
    * Side Effects:
    *  The polynomial value is cut according to mask = 2^(Resolution) - 1.
    *  For example if PRS Resolution is 14 bits the mask value is:
    *  mask = 2^(14) - 1 = 0x3FFFu.
    *  The polynomial value = 0xFFFFu is cut:
    *  polynomial & mask = 0xFFFFu & 0x3FFFu = 0x3FFFu.
    *
    *******************************************************************************/
    void random_WritePolynomial(uint8 polynomial)
                                          
    {
        /* Mask polynomial to cut unused bits */
        polynomial &= random_MASK;

        /* Write polynomial */
        #if (random_TIME_MULTIPLEXING_ENABLE)

            #if (random_PRS_SIZE <= 16u)          /* 16 bits PRS */

                random_POLYNOM_A__D1_REG = HI8(polynomial);
                random_POLYNOM_A__D0_REG = LO8(polynomial);

            #elif (random_PRS_SIZE <= 24u)        /* 24 bits PRS */
                random_POLYNOM_B__D1_REG = LO8(HI16(polynomial));
                random_POLYNOM_B__D0_REG = HI8(polynomial);
                random_POLYNOM_A__D0_REG = LO8(polynomial);

            #else                                           /* 32 bits PRS */
                random_POLYNOM_B__D1_REG = HI8(HI16(polynomial));
                random_POLYNOM_A__D1_REG = LO8(HI16(polynomial));
                random_POLYNOM_B__D0_REG = HI8(polynomial);
                random_POLYNOM_A__D0_REG = LO8(polynomial);

            #endif  /* End (random_PRS_SIZE <= 32u) */

            /* Resets triggers */
            #if (random_WAKEUP_BEHAVIOUR == random__RESUMEWORK)
                if((random_sleepState & random_NORMAL_SEQUENCE) != 0u)
                {
                    random_EXECUTE_DFF_RESET;
                }
            #else
                random_EXECUTE_DFF_RESET;
            #endif  /* (random_WAKEUP_BEHAVIOUR == random__RESUMEWORK) */

        #else

            CY_SET_REG8(random_POLYNOM_PTR, polynomial);

        #endif  /* End (random_TIME_MULTIPLEXING_ENABLE) */
    }

#else   /* 33-64 bits PRS */

    /*******************************************************************************
    *  Function Name: random_ReadUpper
    ********************************************************************************
    *
    * Summary:
    *  Reads upper half of PRS value. Only generated for 33-64-bit PRS.
    *
    * Parameters:
    *  void
    *
    * Return:
    *  Returns upper half of PRS value.
    *
    * Side Effects:
    *  The upper half of seed value is cut according to
    *  mask = 2^(Resolution - 32) - 1.
    *  For example if PRS Resolution is 35 bits the mask value is:
    *  2^(35 - 32) - 1 = 2^(3) - 1 = 0x0000 0007u.
    *  The upper half of seed value = 0x0000 00FFu is cut:
    *  upper half of seed & mask = 0x0000 00FFu & 0x0000 0007u = 0x0000 0007u.
    *
    *******************************************************************************/
    uint32 random_ReadUpper(void) 
    {
        uint32 seed;

        /* Read PRS Upper */
        #if (random_PRS_SIZE <= 40u)          /* 40 bits PRS */
            seed = random_SEED_UPPER_C__A1_REG;

        #elif (random_PRS_SIZE <= 48u)        /* 48 bits PRS */
            seed = ((uint32) random_SEED_UPPER_C__A1_REG) << 8u;
            seed |= random_SEED_UPPER_B__A1_REG;

        #elif (random_PRS_SIZE <= 56u)        /* 56 bits PRS */
            seed = ((uint32) random_SEED_UPPER_D__A1_REG) << 16u;
            seed |= ((uint32) random_SEED_UPPER_C__A1_REG) << 8u;
            seed |= random_SEED_UPPER_B__A1_REG;

        #else                                           /* 64 bits PRS */
            seed = ((uint32) random_SEED_UPPER_D__A1_REG) << 24u;
            seed |= ((uint32) random_SEED_UPPER_C__A1_REG) << 16u;
            seed |= ((uint32) random_SEED_UPPER_B__A1_REG) << 8u;
            seed |= random_SEED_UPPER_A__A1_REG;

        #endif  /* End (random_PRS_SIZE <= 32u) */

        return (seed & random_MASK);
    }


    /*******************************************************************************
    *  Function Name: random_ReadLower
    ********************************************************************************
    *
    * Summary:
    *  Reads lower half of PRS value. Only generated for 33-64-bit PRS.
    *
    * Parameters:
    *  void
    *
    * Return:
    *  Returns lower half of PRS value.
    *
    *******************************************************************************/
    uint32 random_ReadLower(void) 
    {
        uint32 seed;

        /* Read PRS Lower */
        #if (random_PRS_SIZE <= 40u)          /* 40 bits PRS */
            seed = ((uint32) random_SEED_LOWER_B__A1_REG) << 24u;
            seed |= ((uint32) random_SEED_LOWER_C__A0_REG) << 16u;
            seed |= ((uint32) random_SEED_LOWER_B__A0_REG) << 8u;
            seed |= random_SEED_LOWER_A__A0_REG;

        #elif (random_PRS_SIZE <= 48u)        /* 48 bits PRS */
            seed = ((uint32) random_SEED_LOWER_A__A1_REG) << 24u;
            seed |= ((uint32) random_SEED_LOWER_C__A0_REG) << 16u;
            seed |= ((uint32) random_SEED_LOWER_B__A0_REG) << 8u;
            seed |= random_SEED_LOWER_A__A0_REG;

        #else                                           /* 64 bits PRS */
            seed = ((uint32) random_SEED_LOWER_D__A0_REG) << 24u;
            seed |= ((uint32) random_SEED_LOWER_C__A0_REG) << 16u;
            seed |= ((uint32) random_SEED_LOWER_B__A0_REG) << 8u;
            seed |= random_SEED_LOWER_A__A0_REG;

        #endif  /* End (random_PRS_SIZE <= 32u) */

        return seed;
    }


    /*******************************************************************************
    * Function Name: random_WriteSeedUpper
    ********************************************************************************
    *
    * Summary:
    *  Writes upper half of seed value. Only generated for 33-64-bit PRS.
    *
    * Parameters:
    *  seed:  Upper half of seed value.
    *
    * Return:
    *  void
    *
    * Side Effects:
    *  The upper half of seed value is cut according to
    *  mask = 2^(Resolution - 32) - 1.
    *  For example if PRS Resolution is 35 bits the mask value is:
    *  2^(35 - 32) - 1 = 2^(3) - 1 = 0x0000 0007u.
    *  The upper half of seed value = 0x0000 00FFu is cut:
    *  upper half of seed & mask = 0x0000 00FFu & 0x0000 0007u = 0x0000 0007u.
    *
    *******************************************************************************/
    void random_WriteSeedUpper(uint32 seed) 
    {
        /* Mask the Seed Upper half to cut unused bits */
        seed &= random_MASK;

        /* Write Seed Upper */
        #if (random_PRS_SIZE <= 40u)          /* 40 bits PRS */
            random_SEED_UPPER_C__A1_REG = LO8(seed);

        #elif (random_PRS_SIZE <= 48u)        /* 48 bits PRS */
            random_SEED_UPPER_C__A1_REG = HI8(seed);
            random_SEED_UPPER_B__A1_REG = LO8(seed);

        #elif (random_PRS_SIZE <= 56u)        /* 56 bits PRS */
            random_SEED_UPPER_D__A1_REG = LO8(HI16(seed));
            random_SEED_UPPER_C__A1_REG = HI8(seed);
            random_SEED_UPPER_B__A1_REG = HI8(seed);

        #else                                           /* 64 bits PRS */
            random_SEED_UPPER_D__A1_REG = HI8(HI16(seed));
            random_SEED_UPPER_C__A1_REG = LO8(HI16(seed));
            random_SEED_UPPER_B__A1_REG = HI8(seed);
            random_SEED_UPPER_A__A1_REG = LO8(seed);

        #endif  /* End (random_PRS_SIZE <= 32u) */

        /* Resets triggers */
        #if (random_WAKEUP_BEHAVIOUR == random__RESUMEWORK)
            if((random_sleepState & random_NORMAL_SEQUENCE) != 0u)
            {
                random_EXECUTE_DFF_RESET;
            }
        #else
            random_EXECUTE_DFF_RESET;
        #endif  /* (random_WAKEUP_BEHAVIOUR == random__RESUMEWORK) */
    }


    /*******************************************************************************
    * Function Name: random_WriteSeedLower
    ********************************************************************************
    *
    * Summary:
    *  Writes lower half of seed value. Only generated for 33-64-bit PRS.
    *
    * Parameters:
    *  seed:  Lower half of seed value.
    *
    * Return:
    *  void
    *
    *******************************************************************************/
    void random_WriteSeedLower(uint32 seed) 
    {
        /* Write Seed Lower */
        #if (random_PRS_SIZE <= 40u)          /* 40 bits PRS */
            random_SEED_LOWER_B__A1_REG = HI8(HI16(seed));
            random_SEED_LOWER_C__A0_REG = LO8(HI16(seed));
            random_SEED_LOWER_B__A0_REG = HI8(seed);
            random_SEED_LOWER_A__A0_REG = LO8(seed);

        #elif (random_PRS_SIZE <= 48u)        /* 48 bits PRS */
            random_SEED_LOWER_A__A1_REG = HI8(HI16(seed));
            random_SEED_LOWER_C__A0_REG = LO8(HI16(seed));
            random_SEED_LOWER_B__A0_REG = HI8(seed);
            random_SEED_LOWER_A__A0_REG = LO8(seed);

        #else                                           /* 64 bits PRS */
            random_SEED_LOWER_D__A0_REG = HI8(HI16(seed));
            random_SEED_LOWER_C__A0_REG = LO8(HI16(seed));
            random_SEED_LOWER_B__A0_REG = HI8(seed);
            random_SEED_LOWER_A__A0_REG = LO8(seed);

        #endif  /* End (random_PRS_SIZE <= 32u) */

        /* Resets triggers */
        #if (random_WAKEUP_BEHAVIOUR == random__RESUMEWORK)
            if((random_sleepState & random_NORMAL_SEQUENCE) != 0u)
            {
                random_EXECUTE_DFF_RESET;
            }
        #else
            random_EXECUTE_DFF_RESET;
        #endif  /* (random_WAKEUP_BEHAVIOUR == random__RESUMEWORK) */
    }


    /*******************************************************************************
    * Function Name: random_ReadPolynomialUpper
    ********************************************************************************
    *
    * Summary:
    *  Reads upper half of PRS polynomial value. Only generated for 33-64-bit PRS.
    *
    * Parameters:
    *  void
    *
    * Return:
    *  Returns upper half of PRS polynomial value.
    *
    *******************************************************************************/
    uint32 random_ReadPolynomialUpper(void) 
    {
        uint32 polynomial;

        /* Read Polynomial Upper */
        #if (random_PRS_SIZE <= 40u)          /* 40 bits PRS */
            polynomial = random_POLYNOM_UPPER_C__D1_REG;

        #elif (random_PRS_SIZE <= 48u)        /* 48 bits PRS */
            polynomial = ((uint32) random_POLYNOM_UPPER_C__D1_REG) << 8u;
            polynomial |= random_POLYNOM_UPPER_B__D1_REG;

        #elif (random_PRS_SIZE <= 56u)        /* 56 bits PRS */
            polynomial = ((uint32) random_POLYNOM_UPPER_D__D1_REG) << 16u;
            polynomial |= ((uint32) random_POLYNOM_UPPER_C__D1_REG) << 8u;
            polynomial |= random_POLYNOM_UPPER_B__D1_REG;

        #else                                           /* 64 bits PRS */
            polynomial = ((uint32) random_POLYNOM_UPPER_D__D1_REG) << 24u;
            polynomial |= ((uint32) random_POLYNOM_UPPER_C__D1_REG) << 16u;
            polynomial |= ((uint32) random_POLYNOM_UPPER_B__D1_REG) << 8u;
            polynomial |= random_POLYNOM_UPPER_A__D1_REG;

        #endif  /* End (random_PRS_SIZE <= 32u) */

        return polynomial;
    }


    /*******************************************************************************
    * Function Name: random_ReadPolynomialLower
    ********************************************************************************
    *
    * Summary:
    *  Reads lower half of PRS polynomial value. Only generated for 33-64-bit PRS.
    *
    * Parameters:
    *  void
    *
    * Return:
    *  Returns lower half of PRS polynomial value.
    *
    *******************************************************************************/
    uint32 random_ReadPolynomialLower(void) 
    {
        uint32 polynomial;

        /* Read Polynomial Lower */
        #if (random_PRS_SIZE <= 40u)          /* 40 bits PRS */
            polynomial = ( (uint32) random_POLYNOM_LOWER_B__D1_REG) << 24u;
            polynomial |= ( (uint32) random_POLYNOM_LOWER_C__D0_REG) << 16u;
            polynomial |= ( (uint32) random_POLYNOM_LOWER_B__D0_REG) << 8u;
            polynomial |= random_POLYNOM_LOWER_A__D0_REG;

        #elif (random_PRS_SIZE <= 48u)        /* 48 bits PRS */
            polynomial = ((uint32) random_POLYNOM_LOWER_A__D1_REG) << 24u;
            polynomial |= ((uint32) random_POLYNOM_LOWER_C__D0_REG) << 16u;
            polynomial |= ((uint32) random_POLYNOM_LOWER_B__D0_REG) << 8u;
            polynomial |= random_POLYNOM_LOWER_A__D0_REG;

        #else                                           /* 64 bits PRS */
            polynomial = ((uint32) random_POLYNOM_LOWER_D__D0_REG) << 24u;
            polynomial |= ((uint32) random_POLYNOM_LOWER_C__D0_REG) << 16u;
            polynomial |= ((uint32) random_POLYNOM_LOWER_B__D0_REG) << 8u;
            polynomial |= random_POLYNOM_LOWER_A__D0_REG;

        #endif  /* End (random_PRS_SIZE <= 32u) */

        return polynomial;
    }


    /*******************************************************************************
    * Function Name: random_WritePolynomialUpper
    ********************************************************************************
    *
    * Summary:
    *  Writes upper half of PRS polynomial value. Only generated for 33-64-bit PRS.
    *
    * Parameters:
    *  polynomial:  Upper half PRS polynomial value.
    *
    * Return:
    *  void
    *
    * Side Effects:
    *  The upper half of polynomial value is cut according to
    *  mask = 2^(Resolution - 32) - 1.
    *  For example if PRS Resolution is 35 bits the mask value is:
    *  2^(35 - 32) - 1 = 2^(3) - 1 = 0x0000 0007u.
    *  The upper half of polynomial value = 0x0000 00FFu is cut:
    *  upper half of polynomial & mask = 0x0000 00FFu & 0x0000 0007u = 0x0000 0007u.
    *
    *******************************************************************************/
    void random_WritePolynomialUpper(uint32 polynomial)
                                                
    {
        /* Mask the polynomial upper half to cut unused bits */
        polynomial &= random_MASK;

        /* Write Polynomial Upper */
        #if (random_PRS_SIZE <= 40u)          /* 40 bits PRS */
            random_POLYNOM_UPPER_C__D1_REG = LO8(polynomial);

        #elif (random_PRS_SIZE <= 48u)        /* 48 bits PRS */
            random_POLYNOM_UPPER_C__D1_REG = HI8(polynomial);
            random_POLYNOM_UPPER_B__D1_REG = LO8(polynomial);

        #elif (random_PRS_SIZE <= 56u)        /* 56 bits PRS */
            random_POLYNOM_UPPER_D__D1_REG = LO8(HI16(polynomial));
            random_POLYNOM_UPPER_C__D1_REG = HI8(polynomial);
            random_POLYNOM_UPPER_B__D1_REG = LO8(polynomial);

        #else                                           /* 64 bits PRS */
            random_POLYNOM_UPPER_D__D1_REG = HI8(HI16(polynomial));
            random_POLYNOM_UPPER_C__D1_REG = LO8(HI16(polynomial));
            random_POLYNOM_UPPER_B__D1_REG = HI8(polynomial);
            random_POLYNOM_UPPER_A__D1_REG = LO8(polynomial);

        #endif  /* End (random_PRS_SIZE <= 32u) */

        /* Resets triggers */
        #if (random_WAKEUP_BEHAVIOUR == random__RESUMEWORK)
            if((random_sleepState & random_NORMAL_SEQUENCE) != 0u)
            {
                random_EXECUTE_DFF_RESET;
            }
        #else
            random_EXECUTE_DFF_RESET;
        #endif  /* (random_WAKEUP_BEHAVIOUR == random__RESUMEWORK) */
    }


    /*******************************************************************************
    * Function Name: random_WritePolynomialLower
    ********************************************************************************
    *
    * Summary:
    *  Writes lower half of PRS polynomial value. Only generated for 33-64-bit PRS.
    *
    * Parameters:
    *  polynomial:  Lower half of PRS polynomial value.
    *
    * Return:
    *  void
    *
    *******************************************************************************/
    void random_WritePolynomialLower(uint32 polynomial)
                                                
    {
        /* Write Polynomial Lower */
        #if (random_PRS_SIZE <= 40u)          /* 40 bits PRS */
            random_POLYNOM_LOWER_B__D1_REG = HI8(HI16(polynomial));
            random_POLYNOM_LOWER_C__D0_REG = LO8(HI16(polynomial));
            random_POLYNOM_LOWER_B__D0_REG = HI8(polynomial);
            random_POLYNOM_LOWER_A__D0_REG = LO8(polynomial);

        #elif (random_PRS_SIZE <= 48u)        /* 48 bits PRS */
            random_POLYNOM_LOWER_A__D1_REG = HI8(HI16(polynomial));
            random_POLYNOM_LOWER_C__D0_REG = LO8(HI16(polynomial));
            random_POLYNOM_LOWER_B__D0_REG = HI8(polynomial);
            random_POLYNOM_LOWER_A__D0_REG = LO8(polynomial);

        #else                                           /* 64 bits PRS */
            random_POLYNOM_LOWER_D__D0_REG = HI8(HI16(polynomial));
            random_POLYNOM_LOWER_C__D0_REG = LO8(HI16(polynomial));
            random_POLYNOM_LOWER_B__D0_REG = HI8(polynomial);
            random_POLYNOM_LOWER_A__D0_REG = LO8(polynomial);

        #endif  /* End (random_PRS_SIZE <= 32u) */

        /* Resets triggers */
        #if (random_WAKEUP_BEHAVIOUR == random__RESUMEWORK)
            if((random_sleepState & random_NORMAL_SEQUENCE) != 0u)
            {
                random_EXECUTE_DFF_RESET;
            }
        #else
            random_EXECUTE_DFF_RESET;
        #endif  /* (random_WAKEUP_BEHAVIOUR == random__RESUMEWORK) */
    }
#endif  /* End (random_PRS_SIZE <= 32u) */


/* [] END OF FILE */
