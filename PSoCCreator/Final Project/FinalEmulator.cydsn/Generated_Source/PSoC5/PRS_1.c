/*******************************************************************************
* File Name: prs_1.c
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

#include "prs_1.h"

uint8 prs_1_initVar = 0u;

prs_1_BACKUP_STRUCT prs_1_backup =
{
    0x00u, /* enableState; */

    #if (prs_1_WAKEUP_BEHAVIOUR == prs_1__RESUMEWORK)
        /* Save dff register for time mult */
        #if (prs_1_TIME_MULTIPLEXING_ENABLE)
            prs_1_INIT_STATE, /* dffStatus; */
        #endif  /* End prs_1_TIME_MULTIPLEXING_ENABLE */

        /* Save A0 and A1 registers are none-retention */
        #if(prs_1_PRS_SIZE <= 32u)
            prs_1_DEFAULT_SEED, /* seed */

        #else
            prs_1_DEFAULT_SEED_UPPER, /* seedUpper; */
            prs_1_DEFAULT_SEED_LOWER, /* seedLower; */

        #endif  /* End (prs_1_PRS_SIZE <= 32u) */

    #endif  /* End (prs_1_WAKEUP_BEHAVIOUR == prs_1__RESUMEWORK) */
};


/*******************************************************************************
* Function Name: prs_1_Init
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
void prs_1_Init(void) 
{
    /* Writes seed value and ponynom value provided for customizer */
    #if (prs_1_PRS_SIZE <= 32u)
        prs_1_WritePolynomial(prs_1_DEFAULT_POLYNOM);
        prs_1_WriteSeed(prs_1_DEFAULT_SEED);
        #if (prs_1_RUN_MODE == prs_1__CLOCKED)
            prs_1_ResetSeedInit(prs_1_DEFAULT_SEED);
        #endif  /* End (prs_1_RUN_MODE == prs_1__CLOCKED) */
            prs_1_Enable();
    #else
        prs_1_WritePolynomialUpper(prs_1_DEFAULT_POLYNOM_UPPER);
        prs_1_WritePolynomialLower(prs_1_DEFAULT_POLYNOM_LOWER);
        prs_1_WriteSeedUpper(prs_1_DEFAULT_SEED_UPPER);
        prs_1_WriteSeedLower(prs_1_DEFAULT_SEED_LOWER);
        #if (prs_1_RUN_MODE == prs_1__CLOCKED)
            prs_1_ResetSeedInitUpper(prs_1_DEFAULT_SEED_UPPER);
            prs_1_ResetSeedInitLower(prs_1_DEFAULT_SEED_LOWER);
        #endif  /* End (prs_1_RUN_MODE == prs_1__CLOCKED) */
            prs_1_Enable();
    #endif  /* End (prs_1_PRS_SIZE <= 32u) */
}


/*******************************************************************************
* Function Name: prs_1_Enable
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
void prs_1_Enable(void) 
{
        prs_1_CONTROL_REG |= prs_1_CTRL_ENABLE;
}


/*******************************************************************************
* Function Name: prs_1_Start
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
*  prs_1_initVar: global variable is used to indicate initial
*  configuration of this component.  The variable is initialized to zero and set
*  to 1 the first time prs_1_Start() is called. This allows
*  enable/disable component without re-initialization in all subsequent calls
*  to the prs_1_Start() routine.
*
*******************************************************************************/
void prs_1_Start(void) 
{
    /* Writes seed value and ponynom value provided from customizer */
    if (prs_1_initVar == 0u)
    {
        prs_1_Init();
        prs_1_initVar = 1u;
    }

    prs_1_Enable();
}


/*******************************************************************************
* Function Name: prs_1_Stop
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
void prs_1_Stop(void) 
{
    prs_1_CONTROL_REG &= ((uint8)~prs_1_CTRL_ENABLE);
}


#if (prs_1_RUN_MODE == prs_1__APISINGLESTEP)
    /*******************************************************************************
    * FUNCTION NAME: prs_1_Step
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
    void prs_1_Step(void) 
    {
        #if (prs_1_TIME_MULTIPLEXING_ENABLE)
            /* Makes 4 pulse, 4x for Time Mult */
            prs_1_EXECUTE_STEP;
            prs_1_EXECUTE_STEP;
            prs_1_EXECUTE_STEP;
            prs_1_EXECUTE_STEP;

        #else
            /* One pulse without Time mult required */
            prs_1_EXECUTE_STEP;

        #endif  /* End prs_1_TIME_MULTIPLEXING_ENABLE */
    }
#endif  /* End (prs_1_RUN_MODE == prs_1__APISINGLESTEP) */


#if (prs_1_RUN_MODE == prs_1__CLOCKED)
    #if (prs_1_PRS_SIZE <= 32u) /* 8-32 bits PRS */
        /*******************************************************************************
        * FUNCTION NAME: prs_1_ResetSeedInit
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
        void prs_1_ResetSeedInit(uint8 seed)
                                            
        {
            uint8 enableInterrupts;

            /* Mask the Seed to cut unused bits */
            seed &= prs_1_MASK;

            /* Change AuxControl reg, need to be safety */
            enableInterrupts = CyEnterCriticalSection();

            #if (prs_1_TIME_MULTIPLEXING_ENABLE)
                /* Set FIFOs to single register */
                prs_1_AUX_CONTROL_A_REG |= prs_1_AUXCTRL_FIFO_SINGLE_REG;

                #if(prs_1_PRS_SIZE > 16u)       /* 17-32 bits PRS */
                    prs_1_AUX_CONTROL_B_REG |= prs_1_AUXCTRL_FIFO_SINGLE_REG;
                #endif  /* End (prs_1_PRS_SIZE <= 8u) */

                /* AuxControl reg settings are done */
                CyExitCriticalSection(enableInterrupts);

                /* Write Seed COPY */
                #if (prs_1_PRS_SIZE <= 16u)          /* 16 bits PRS */
                    prs_1_SEED_COPY_A__A1_REG = HI8(seed);
                    prs_1_SEED_COPY_A__A0_REG = LO8(seed);

                #elif (prs_1_PRS_SIZE <= 24u)        /* 24 bits PRS */
                    prs_1_SEED_COPY_B__A1_REG = LO8(HI16(seed));
                    prs_1_SEED_COPY_B__A0_REG = HI8(seed);
                    prs_1_SEED_COPY_A__A0_REG = LO8(seed);

                #else                                           /* 32 bits PRS */
                    prs_1_SEED_COPY_B__A1_REG = HI8(HI16(seed));
                    prs_1_SEED_COPY_A__A1_REG = LO8(HI16(seed));
                    prs_1_SEED_COPY_B__A0_REG = HI8(seed);
                    prs_1_SEED_COPY_A__A0_REG = LO8(seed);
                #endif  /* End (prs_1_PRS_SIZE <= 32u) */

            #else
                /* Set FIFOs to single register */
                #if (prs_1_PRS_SIZE <= 8u)      /* 8 bits PRS */
                    prs_1_AUX_CONTROL_A_REG |= prs_1_AUXCTRL_FIFO_SINGLE_REG;

                #elif (prs_1_PRS_SIZE <= 16u)      /* 16 bits PRS */
                    prs_1_AUX_CONTROL_A_REG  |= prs_1_AUXCTRL_FIFO_SINGLE_REG;
                    prs_1_AUX_CONTROL_B_REG  |= prs_1_AUXCTRL_FIFO_SINGLE_REG;

                #elif (prs_1_PRS_SIZE <= 24u)      /* 24-39 bits PRS */
                    prs_1_AUX_CONTROL_A_REG  |= prs_1_AUXCTRL_FIFO_SINGLE_REG;
                    prs_1_AUX_CONTROL_B_REG  |= prs_1_AUXCTRL_FIFO_SINGLE_REG;
                    prs_1_AUX_CONTROL_C_REG  |= prs_1_AUXCTRL_FIFO_SINGLE_REG;

                #elif (prs_1_PRS_SIZE <= 32u)      /* 40-55 bits PRS */
                    prs_1_AUX_CONTROL_A_REG  |= prs_1_AUXCTRL_FIFO_SINGLE_REG;
                    prs_1_AUX_CONTROL_B_REG  |= prs_1_AUXCTRL_FIFO_SINGLE_REG;
                    prs_1_AUX_CONTROL_C_REG  |= prs_1_AUXCTRL_FIFO_SINGLE_REG;
                    prs_1_AUX_CONTROL_D_REG  |= prs_1_AUXCTRL_FIFO_SINGLE_REG;

                #endif  /* End (prs_1_PRS_SIZE <= 8u) */

                /* AuxControl reg setting are done */
                CyExitCriticalSection(enableInterrupts);

                /* Write Seed COPY */
                CY_SET_REG8(prs_1_SEED_COPY_PTR, seed);

            #endif  /* End (prs_1_TIME_MULTIPLEXING_ENABLE) */
        }

    #else

        /*******************************************************************************
        * FUNCTION NAME: prs_1_ResetSeedInitUpper
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
        void prs_1_ResetSeedInitUpper(uint32 seed) 
        {
			uint8 enableInterrupts;

			/* Mask the Seed Upper half to cut unused bits */
            seed &= prs_1_MASK;

			/* Change AuxControl reg, need to be safety */
            enableInterrupts = CyEnterCriticalSection();

            /* Set FIFOs to single register */
            prs_1_AUX_CONTROL_A_REG |= prs_1_AUXCTRL_FIFO_SINGLE_REG;
            prs_1_AUX_CONTROL_B_REG |= prs_1_AUXCTRL_FIFO_SINGLE_REG;
            prs_1_AUX_CONTROL_C_REG |= prs_1_AUXCTRL_FIFO_SINGLE_REG;

			#if (prs_1_PRS_SIZE > 48u)               /* 49-64 bits PRS */
                prs_1_AUX_CONTROL_D_REG |= prs_1_AUXCTRL_FIFO_SINGLE_REG;
            #endif  /* End (prs_1_PRS_SIZE <= 8u) */

            /* AuxControl reg settings are done */
            CyExitCriticalSection(enableInterrupts);

            /* Write Seed Upper COPY */
            #if (prs_1_PRS_SIZE <= 40u)          /* 40 bits PRS */
                prs_1_SEED_UPPER_COPY_C__A1_REG = LO8(seed);

            #elif (prs_1_PRS_SIZE <= 48u)        /* 48 bits PRS */
                prs_1_SEED_UPPER_COPY_C__A1_REG = HI8(seed);
                prs_1_SEED_UPPER_COPY_B__A1_REG = LO8(seed);

            #elif (prs_1_PRS_SIZE <= 56u)        /* 56 bits PRS */
                prs_1_SEED_UPPER_COPY_D__A1_REG = LO8(HI16(seed));
                prs_1_SEED_UPPER_COPY_C__A1_REG = HI8(seed);
                prs_1_SEED_UPPER_COPY_B__A1_REG = HI8(seed);

            #else                                           /* 64 bits PRS */
                prs_1_SEED_UPPER_COPY_D__A1_REG = HI8(HI16(seed));
                prs_1_SEED_UPPER_COPY_C__A1_REG = LO8(HI16(seed));
                prs_1_SEED_UPPER_COPY_B__A1_REG = HI8(seed);
                prs_1_SEED_UPPER_COPY_A__A1_REG = LO8(seed);

            #endif  /* End (prs_1_PRS_SIZE <= 32u) */
        }


        /*******************************************************************************
        * FUNCTION NAME: prs_1_ResetSeedInitLower
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
        void prs_1_ResetSeedInitLower(uint32 seed) 
        {
            /* Write Seed Lower COPY */
            #if (prs_1_PRS_SIZE <= 40u)          /* 40 bits PRS */
                prs_1_SEED_LOWER_COPY_B__A1_REG = HI8(HI16(seed));
                prs_1_SEED_LOWER_COPY_C__A0_REG = LO8(HI16(seed));
                prs_1_SEED_LOWER_COPY_B__A0_REG = HI8(seed);
                prs_1_SEED_LOWER_COPY_A__A0_REG = LO8(seed);

            #elif (prs_1_PRS_SIZE <= 48u)        /* 48 bits PRS */
                prs_1_SEED_LOWER_COPY_A__A1_REG = HI8(HI16(seed));
                prs_1_SEED_LOWER_COPY_C__A0_REG = LO8(HI16(seed));
                prs_1_SEED_LOWER_COPY_B__A0_REG = HI8(seed);
                prs_1_SEED_LOWER_COPY_A__A0_REG = LO8(seed);

            #else                                           /* 64 bits PRS */
                prs_1_SEED_LOWER_COPY_D__A0_REG = HI8(HI16(seed));
                prs_1_SEED_LOWER_COPY_C__A0_REG = LO8(HI16(seed));
                prs_1_SEED_LOWER_COPY_B__A0_REG = HI8(seed);
                prs_1_SEED_LOWER_COPY_A__A0_REG = LO8(seed);

            #endif  /* End (prs_1_PRS_SIZE <= 32u) */
        }

    #endif  /* End (prs_1_PRS_SIZE <= 32u) */

#endif  /* End (prs_1_RUN_MODE == prs_1__CLOCKED) */


#if(prs_1_PRS_SIZE <= 32u) /* 8-32 bits PRS */
    /*******************************************************************************
    * Function Name: prs_1_Read
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
    uint8 prs_1_Read(void) 
    {
        /* Read PRS */
        #if (prs_1_TIME_MULTIPLEXING_ENABLE)

            uint8 seed;

            #if (prs_1_PRS_SIZE <= 16u)          /* 16 bits PRS */
                seed = ((uint16) prs_1_SEED_A__A1_REG) << 8u;
                seed |= prs_1_SEED_A__A0_REG;

            #elif (prs_1_PRS_SIZE <= 24u)        /* 24 bits PRS */
                seed = ((uint32) (prs_1_SEED_B__A1_REG)) << 16u;
                seed |= ((uint32) (prs_1_SEED_B__A0_REG)) << 8u;
                seed |= prs_1_SEED_A__A0_REG;

            #else                                           /* 32 bits PRS */
                seed = ((uint32) prs_1_SEED_B__A1_REG) << 24u;
                seed |= ((uint32) prs_1_SEED_A__A1_REG) << 16u;
                seed |= ((uint32) prs_1_SEED_B__A0_REG) << 8u;
                seed |= prs_1_SEED_A__A0_REG;

            #endif  /* End (prs_1_PRS_SIZE <= 8u) */

            return (seed  & prs_1_MASK);

        #else

            return (CY_GET_REG8(prs_1_SEED_PTR) & prs_1_MASK);

        #endif  /* End (prs_1_TIME_MULTIPLEXING_ENABLE) */
    }


    /*******************************************************************************
    * Function Name: prs_1_WriteSeed
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
    void prs_1_WriteSeed(uint8 seed) 
    {
        /* Masks the Seed to cut unused bits */
        seed &= prs_1_MASK;

        /* Writes Seed */
        #if (prs_1_TIME_MULTIPLEXING_ENABLE)

            #if (prs_1_PRS_SIZE <= 16u)          /* 16 bits PRS */
                prs_1_SEED_A__A1_REG = HI8(seed);
                prs_1_SEED_A__A0_REG = LO8(seed);

            #elif (prs_1_PRS_SIZE <= 24u)        /* 24 bits PRS */
                prs_1_SEED_B__A1_REG = LO8(HI16(seed));
                prs_1_SEED_B__A0_REG = HI8(seed);
                prs_1_SEED_A__A0_REG = LO8(seed);

            #else                                           /* 32 bits PRS */
                prs_1_SEED_B__A1_REG = HI8(HI16(seed));
                prs_1_SEED_A__A1_REG = LO8(HI16(seed));
                prs_1_SEED_B__A0_REG = HI8(seed);
                prs_1_SEED_A__A0_REG = LO8(seed);
            #endif  /* End (prs_1_PRS_SIZE <= 32u) */

            /* Resets triggers */
            #if (prs_1_WAKEUP_BEHAVIOUR == prs_1__RESUMEWORK)
                if((prs_1_sleepState & prs_1_NORMAL_SEQUENCE) != 0u)
                {
                    prs_1_EXECUTE_DFF_RESET;
                }
            #else
                prs_1_EXECUTE_DFF_RESET;
            #endif  /* (prs_1_WAKEUP_BEHAVIOUR == prs_1__RESUMEWORK) */

        #else

            CY_SET_REG8(prs_1_SEED_PTR, seed);

        #endif  /* End (prs_1_TIME_MULTIPLEXING_ENABLE) */
    }


    /*******************************************************************************
    * Function Name: prs_1_ReadPolynomial
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
    uint8 prs_1_ReadPolynomial(void)
                                
    {
        /* Reads polynomial */
        #if (prs_1_TIME_MULTIPLEXING_ENABLE)

            uint8 polynomial;

            #if (prs_1_PRS_SIZE <= 16u)          /* 16 bits PRS */
                polynomial = ((uint16) prs_1_POLYNOM_A__D1_REG) << 8u;
                polynomial |= (prs_1_POLYNOM_A__D0_REG);

            #elif (prs_1_PRS_SIZE <= 24u)        /* 24 bits PRS */
                polynomial = ((uint32) prs_1_POLYNOM_B__D1_REG) << 16u;
                polynomial |= ((uint32) prs_1_POLYNOM_B__D0_REG) << 8u;
                polynomial |= prs_1_POLYNOM_A__D0_REG;

            #else                                           /* 32 bits PRS */
                polynomial = ((uint32) prs_1_POLYNOM_B__D1_REG) << 24u;
                polynomial |= ((uint32) prs_1_POLYNOM_A__D1_REG) << 16u;
                polynomial |= ((uint32) prs_1_POLYNOM_B__D0_REG) << 8u;
                polynomial |= prs_1_POLYNOM_A__D0_REG;

            #endif  /* End (prs_1_PRS_SIZE <= 32u) */

            return polynomial;

        #else

            return CY_GET_REG8(prs_1_POLYNOM_PTR);

        #endif  /* End (prs_1_TIME_MULTIPLEXING_ENABLE) */
    }


    /*******************************************************************************
    * Function Name: prs_1_WritePolynomial
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
    void prs_1_WritePolynomial(uint8 polynomial)
                                          
    {
        /* Mask polynomial to cut unused bits */
        polynomial &= prs_1_MASK;

        /* Write polynomial */
        #if (prs_1_TIME_MULTIPLEXING_ENABLE)

            #if (prs_1_PRS_SIZE <= 16u)          /* 16 bits PRS */

                prs_1_POLYNOM_A__D1_REG = HI8(polynomial);
                prs_1_POLYNOM_A__D0_REG = LO8(polynomial);

            #elif (prs_1_PRS_SIZE <= 24u)        /* 24 bits PRS */
                prs_1_POLYNOM_B__D1_REG = LO8(HI16(polynomial));
                prs_1_POLYNOM_B__D0_REG = HI8(polynomial);
                prs_1_POLYNOM_A__D0_REG = LO8(polynomial);

            #else                                           /* 32 bits PRS */
                prs_1_POLYNOM_B__D1_REG = HI8(HI16(polynomial));
                prs_1_POLYNOM_A__D1_REG = LO8(HI16(polynomial));
                prs_1_POLYNOM_B__D0_REG = HI8(polynomial);
                prs_1_POLYNOM_A__D0_REG = LO8(polynomial);

            #endif  /* End (prs_1_PRS_SIZE <= 32u) */

            /* Resets triggers */
            #if (prs_1_WAKEUP_BEHAVIOUR == prs_1__RESUMEWORK)
                if((prs_1_sleepState & prs_1_NORMAL_SEQUENCE) != 0u)
                {
                    prs_1_EXECUTE_DFF_RESET;
                }
            #else
                prs_1_EXECUTE_DFF_RESET;
            #endif  /* (prs_1_WAKEUP_BEHAVIOUR == prs_1__RESUMEWORK) */

        #else

            CY_SET_REG8(prs_1_POLYNOM_PTR, polynomial);

        #endif  /* End (prs_1_TIME_MULTIPLEXING_ENABLE) */
    }

#else   /* 33-64 bits PRS */

    /*******************************************************************************
    *  Function Name: prs_1_ReadUpper
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
    uint32 prs_1_ReadUpper(void) 
    {
        uint32 seed;

        /* Read PRS Upper */
        #if (prs_1_PRS_SIZE <= 40u)          /* 40 bits PRS */
            seed = prs_1_SEED_UPPER_C__A1_REG;

        #elif (prs_1_PRS_SIZE <= 48u)        /* 48 bits PRS */
            seed = ((uint32) prs_1_SEED_UPPER_C__A1_REG) << 8u;
            seed |= prs_1_SEED_UPPER_B__A1_REG;

        #elif (prs_1_PRS_SIZE <= 56u)        /* 56 bits PRS */
            seed = ((uint32) prs_1_SEED_UPPER_D__A1_REG) << 16u;
            seed |= ((uint32) prs_1_SEED_UPPER_C__A1_REG) << 8u;
            seed |= prs_1_SEED_UPPER_B__A1_REG;

        #else                                           /* 64 bits PRS */
            seed = ((uint32) prs_1_SEED_UPPER_D__A1_REG) << 24u;
            seed |= ((uint32) prs_1_SEED_UPPER_C__A1_REG) << 16u;
            seed |= ((uint32) prs_1_SEED_UPPER_B__A1_REG) << 8u;
            seed |= prs_1_SEED_UPPER_A__A1_REG;

        #endif  /* End (prs_1_PRS_SIZE <= 32u) */

        return (seed & prs_1_MASK);
    }


    /*******************************************************************************
    *  Function Name: prs_1_ReadLower
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
    uint32 prs_1_ReadLower(void) 
    {
        uint32 seed;

        /* Read PRS Lower */
        #if (prs_1_PRS_SIZE <= 40u)          /* 40 bits PRS */
            seed = ((uint32) prs_1_SEED_LOWER_B__A1_REG) << 24u;
            seed |= ((uint32) prs_1_SEED_LOWER_C__A0_REG) << 16u;
            seed |= ((uint32) prs_1_SEED_LOWER_B__A0_REG) << 8u;
            seed |= prs_1_SEED_LOWER_A__A0_REG;

        #elif (prs_1_PRS_SIZE <= 48u)        /* 48 bits PRS */
            seed = ((uint32) prs_1_SEED_LOWER_A__A1_REG) << 24u;
            seed |= ((uint32) prs_1_SEED_LOWER_C__A0_REG) << 16u;
            seed |= ((uint32) prs_1_SEED_LOWER_B__A0_REG) << 8u;
            seed |= prs_1_SEED_LOWER_A__A0_REG;

        #else                                           /* 64 bits PRS */
            seed = ((uint32) prs_1_SEED_LOWER_D__A0_REG) << 24u;
            seed |= ((uint32) prs_1_SEED_LOWER_C__A0_REG) << 16u;
            seed |= ((uint32) prs_1_SEED_LOWER_B__A0_REG) << 8u;
            seed |= prs_1_SEED_LOWER_A__A0_REG;

        #endif  /* End (prs_1_PRS_SIZE <= 32u) */

        return seed;
    }


    /*******************************************************************************
    * Function Name: prs_1_WriteSeedUpper
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
    void prs_1_WriteSeedUpper(uint32 seed) 
    {
        /* Mask the Seed Upper half to cut unused bits */
        seed &= prs_1_MASK;

        /* Write Seed Upper */
        #if (prs_1_PRS_SIZE <= 40u)          /* 40 bits PRS */
            prs_1_SEED_UPPER_C__A1_REG = LO8(seed);

        #elif (prs_1_PRS_SIZE <= 48u)        /* 48 bits PRS */
            prs_1_SEED_UPPER_C__A1_REG = HI8(seed);
            prs_1_SEED_UPPER_B__A1_REG = LO8(seed);

        #elif (prs_1_PRS_SIZE <= 56u)        /* 56 bits PRS */
            prs_1_SEED_UPPER_D__A1_REG = LO8(HI16(seed));
            prs_1_SEED_UPPER_C__A1_REG = HI8(seed);
            prs_1_SEED_UPPER_B__A1_REG = HI8(seed);

        #else                                           /* 64 bits PRS */
            prs_1_SEED_UPPER_D__A1_REG = HI8(HI16(seed));
            prs_1_SEED_UPPER_C__A1_REG = LO8(HI16(seed));
            prs_1_SEED_UPPER_B__A1_REG = HI8(seed);
            prs_1_SEED_UPPER_A__A1_REG = LO8(seed);

        #endif  /* End (prs_1_PRS_SIZE <= 32u) */

        /* Resets triggers */
        #if (prs_1_WAKEUP_BEHAVIOUR == prs_1__RESUMEWORK)
            if((prs_1_sleepState & prs_1_NORMAL_SEQUENCE) != 0u)
            {
                prs_1_EXECUTE_DFF_RESET;
            }
        #else
            prs_1_EXECUTE_DFF_RESET;
        #endif  /* (prs_1_WAKEUP_BEHAVIOUR == prs_1__RESUMEWORK) */
    }


    /*******************************************************************************
    * Function Name: prs_1_WriteSeedLower
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
    void prs_1_WriteSeedLower(uint32 seed) 
    {
        /* Write Seed Lower */
        #if (prs_1_PRS_SIZE <= 40u)          /* 40 bits PRS */
            prs_1_SEED_LOWER_B__A1_REG = HI8(HI16(seed));
            prs_1_SEED_LOWER_C__A0_REG = LO8(HI16(seed));
            prs_1_SEED_LOWER_B__A0_REG = HI8(seed);
            prs_1_SEED_LOWER_A__A0_REG = LO8(seed);

        #elif (prs_1_PRS_SIZE <= 48u)        /* 48 bits PRS */
            prs_1_SEED_LOWER_A__A1_REG = HI8(HI16(seed));
            prs_1_SEED_LOWER_C__A0_REG = LO8(HI16(seed));
            prs_1_SEED_LOWER_B__A0_REG = HI8(seed);
            prs_1_SEED_LOWER_A__A0_REG = LO8(seed);

        #else                                           /* 64 bits PRS */
            prs_1_SEED_LOWER_D__A0_REG = HI8(HI16(seed));
            prs_1_SEED_LOWER_C__A0_REG = LO8(HI16(seed));
            prs_1_SEED_LOWER_B__A0_REG = HI8(seed);
            prs_1_SEED_LOWER_A__A0_REG = LO8(seed);

        #endif  /* End (prs_1_PRS_SIZE <= 32u) */

        /* Resets triggers */
        #if (prs_1_WAKEUP_BEHAVIOUR == prs_1__RESUMEWORK)
            if((prs_1_sleepState & prs_1_NORMAL_SEQUENCE) != 0u)
            {
                prs_1_EXECUTE_DFF_RESET;
            }
        #else
            prs_1_EXECUTE_DFF_RESET;
        #endif  /* (prs_1_WAKEUP_BEHAVIOUR == prs_1__RESUMEWORK) */
    }


    /*******************************************************************************
    * Function Name: prs_1_ReadPolynomialUpper
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
    uint32 prs_1_ReadPolynomialUpper(void) 
    {
        uint32 polynomial;

        /* Read Polynomial Upper */
        #if (prs_1_PRS_SIZE <= 40u)          /* 40 bits PRS */
            polynomial = prs_1_POLYNOM_UPPER_C__D1_REG;

        #elif (prs_1_PRS_SIZE <= 48u)        /* 48 bits PRS */
            polynomial = ((uint32) prs_1_POLYNOM_UPPER_C__D1_REG) << 8u;
            polynomial |= prs_1_POLYNOM_UPPER_B__D1_REG;

        #elif (prs_1_PRS_SIZE <= 56u)        /* 56 bits PRS */
            polynomial = ((uint32) prs_1_POLYNOM_UPPER_D__D1_REG) << 16u;
            polynomial |= ((uint32) prs_1_POLYNOM_UPPER_C__D1_REG) << 8u;
            polynomial |= prs_1_POLYNOM_UPPER_B__D1_REG;

        #else                                           /* 64 bits PRS */
            polynomial = ((uint32) prs_1_POLYNOM_UPPER_D__D1_REG) << 24u;
            polynomial |= ((uint32) prs_1_POLYNOM_UPPER_C__D1_REG) << 16u;
            polynomial |= ((uint32) prs_1_POLYNOM_UPPER_B__D1_REG) << 8u;
            polynomial |= prs_1_POLYNOM_UPPER_A__D1_REG;

        #endif  /* End (prs_1_PRS_SIZE <= 32u) */

        return polynomial;
    }


    /*******************************************************************************
    * Function Name: prs_1_ReadPolynomialLower
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
    uint32 prs_1_ReadPolynomialLower(void) 
    {
        uint32 polynomial;

        /* Read Polynomial Lower */
        #if (prs_1_PRS_SIZE <= 40u)          /* 40 bits PRS */
            polynomial = ( (uint32) prs_1_POLYNOM_LOWER_B__D1_REG) << 24u;
            polynomial |= ( (uint32) prs_1_POLYNOM_LOWER_C__D0_REG) << 16u;
            polynomial |= ( (uint32) prs_1_POLYNOM_LOWER_B__D0_REG) << 8u;
            polynomial |= prs_1_POLYNOM_LOWER_A__D0_REG;

        #elif (prs_1_PRS_SIZE <= 48u)        /* 48 bits PRS */
            polynomial = ((uint32) prs_1_POLYNOM_LOWER_A__D1_REG) << 24u;
            polynomial |= ((uint32) prs_1_POLYNOM_LOWER_C__D0_REG) << 16u;
            polynomial |= ((uint32) prs_1_POLYNOM_LOWER_B__D0_REG) << 8u;
            polynomial |= prs_1_POLYNOM_LOWER_A__D0_REG;

        #else                                           /* 64 bits PRS */
            polynomial = ((uint32) prs_1_POLYNOM_LOWER_D__D0_REG) << 24u;
            polynomial |= ((uint32) prs_1_POLYNOM_LOWER_C__D0_REG) << 16u;
            polynomial |= ((uint32) prs_1_POLYNOM_LOWER_B__D0_REG) << 8u;
            polynomial |= prs_1_POLYNOM_LOWER_A__D0_REG;

        #endif  /* End (prs_1_PRS_SIZE <= 32u) */

        return polynomial;
    }


    /*******************************************************************************
    * Function Name: prs_1_WritePolynomialUpper
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
    void prs_1_WritePolynomialUpper(uint32 polynomial)
                                                
    {
        /* Mask the polynomial upper half to cut unused bits */
        polynomial &= prs_1_MASK;

        /* Write Polynomial Upper */
        #if (prs_1_PRS_SIZE <= 40u)          /* 40 bits PRS */
            prs_1_POLYNOM_UPPER_C__D1_REG = LO8(polynomial);

        #elif (prs_1_PRS_SIZE <= 48u)        /* 48 bits PRS */
            prs_1_POLYNOM_UPPER_C__D1_REG = HI8(polynomial);
            prs_1_POLYNOM_UPPER_B__D1_REG = LO8(polynomial);

        #elif (prs_1_PRS_SIZE <= 56u)        /* 56 bits PRS */
            prs_1_POLYNOM_UPPER_D__D1_REG = LO8(HI16(polynomial));
            prs_1_POLYNOM_UPPER_C__D1_REG = HI8(polynomial);
            prs_1_POLYNOM_UPPER_B__D1_REG = LO8(polynomial);

        #else                                           /* 64 bits PRS */
            prs_1_POLYNOM_UPPER_D__D1_REG = HI8(HI16(polynomial));
            prs_1_POLYNOM_UPPER_C__D1_REG = LO8(HI16(polynomial));
            prs_1_POLYNOM_UPPER_B__D1_REG = HI8(polynomial);
            prs_1_POLYNOM_UPPER_A__D1_REG = LO8(polynomial);

        #endif  /* End (prs_1_PRS_SIZE <= 32u) */

        /* Resets triggers */
        #if (prs_1_WAKEUP_BEHAVIOUR == prs_1__RESUMEWORK)
            if((prs_1_sleepState & prs_1_NORMAL_SEQUENCE) != 0u)
            {
                prs_1_EXECUTE_DFF_RESET;
            }
        #else
            prs_1_EXECUTE_DFF_RESET;
        #endif  /* (prs_1_WAKEUP_BEHAVIOUR == prs_1__RESUMEWORK) */
    }


    /*******************************************************************************
    * Function Name: prs_1_WritePolynomialLower
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
    void prs_1_WritePolynomialLower(uint32 polynomial)
                                                
    {
        /* Write Polynomial Lower */
        #if (prs_1_PRS_SIZE <= 40u)          /* 40 bits PRS */
            prs_1_POLYNOM_LOWER_B__D1_REG = HI8(HI16(polynomial));
            prs_1_POLYNOM_LOWER_C__D0_REG = LO8(HI16(polynomial));
            prs_1_POLYNOM_LOWER_B__D0_REG = HI8(polynomial);
            prs_1_POLYNOM_LOWER_A__D0_REG = LO8(polynomial);

        #elif (prs_1_PRS_SIZE <= 48u)        /* 48 bits PRS */
            prs_1_POLYNOM_LOWER_A__D1_REG = HI8(HI16(polynomial));
            prs_1_POLYNOM_LOWER_C__D0_REG = LO8(HI16(polynomial));
            prs_1_POLYNOM_LOWER_B__D0_REG = HI8(polynomial);
            prs_1_POLYNOM_LOWER_A__D0_REG = LO8(polynomial);

        #else                                           /* 64 bits PRS */
            prs_1_POLYNOM_LOWER_D__D0_REG = HI8(HI16(polynomial));
            prs_1_POLYNOM_LOWER_C__D0_REG = LO8(HI16(polynomial));
            prs_1_POLYNOM_LOWER_B__D0_REG = HI8(polynomial);
            prs_1_POLYNOM_LOWER_A__D0_REG = LO8(polynomial);

        #endif  /* End (prs_1_PRS_SIZE <= 32u) */

        /* Resets triggers */
        #if (prs_1_WAKEUP_BEHAVIOUR == prs_1__RESUMEWORK)
            if((prs_1_sleepState & prs_1_NORMAL_SEQUENCE) != 0u)
            {
                prs_1_EXECUTE_DFF_RESET;
            }
        #else
            prs_1_EXECUTE_DFF_RESET;
        #endif  /* (prs_1_WAKEUP_BEHAVIOUR == prs_1__RESUMEWORK) */
    }
#endif  /* End (prs_1_PRS_SIZE <= 32u) */


/* [] END OF FILE */
