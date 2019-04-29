/*******************************************************************************
* File Name: random.h
* Version 2.40
*
* Description:
*  This file provides constants and parameter values for the PRS component.
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

#if !defined(CY_PRS_random_H)
#define CY_PRS_random_H

#include "cyfitter.h"
#include "cytypes.h"
#include "CyLib.h"


/***************************************
*   Conditional Compilation Parameters
****************************************/

#define random_PRS_SIZE                   (8u)
#define random_RUN_MODE                   (0u)
#define random_TIME_MULTIPLEXING_ENABLE   (0u)
#define random_WAKEUP_BEHAVIOUR           (1u)

#define random__CLOCKED 0
#define random__APISINGLESTEP 1


#define random__RESUMEWORK 1
#define random__STARTAFRESH 0



/***************************************
*       Type defines
***************************************/

/* Structure to save registers before go to sleep */
typedef struct
{
    uint8 enableState;
    
    #if (random_WAKEUP_BEHAVIOUR == random__RESUMEWORK)
        /* Save dff register for time mult */
        #if (random_TIME_MULTIPLEXING_ENABLE)
            uint8 dffStatus;
        #endif  /* End random_TIME_MULTIPLEXING_ENABLE */
    
        /* Save A0 and A1 registers are none-retention */
        #if(random_PRS_SIZE <= 32u)
            uint8 seed;
            
        #else
            uint32 seedUpper;
            uint32 seedLower;
            
        #endif  /* End (random_PRS_SIZE <= 32u) */ 
        
    #endif  /* End (random_WAKEUP_BEHAVIOUR == random__RESUMEWORK) */
    
} random_BACKUP_STRUCT;

extern uint8 random_initVar;

extern random_BACKUP_STRUCT random_backup;

#if ((random_TIME_MULTIPLEXING_ENABLE) && (random_WAKEUP_BEHAVIOUR == random__RESUMEWORK))
    extern uint8 random_sleepState;
#endif  /* End ((random_TIME_MULTIPLEXING_ENABLE) && 
          (random_WAKEUP_BEHAVIOUR == random__RESUMEWORK)) */

                                     
/***************************************
*        Function Prototypes
****************************************/

void random_Init(void) ;
void random_Enable(void) ;
void random_Start(void) ;
void random_Stop(void) ;
void random_SaveConfig(void) ;
void random_Sleep(void) ;


#if ((random_WAKEUP_BEHAVIOUR == random__RESUMEWORK) && (random_TIME_MULTIPLEXING_ENABLE))
    void random_RestoreConfig(void) ;
    void random_Wakeup(void) ;
#else
    void random_RestoreConfig(void) ;
    void random_Wakeup(void) ;
#endif  /* End ((random_WAKEUP_BEHAVIOUR == random__RESUMEWORK) && 
                (random_TIME_MULTIPLEXING_ENABLE)) */

#if (random_RUN_MODE == random__APISINGLESTEP)
    void random_Step(void) ;
#endif  /* End (random_RUN_MODE == random__APISINGLESTEP) */

#if (random_PRS_SIZE <= 32u)    /* 8-32 bits PRS */
    uint8 random_Read(void) ;
    void random_WriteSeed(uint8 seed)  ;
    uint8 random_ReadPolynomial(void) 
                                ;
    void random_WritePolynomial(uint8 polynomial) 
                                          ;
    
#else                                    /* 33-64 bits PRS */
    uint32 random_ReadUpper(void) ;
    uint32 random_ReadLower(void) ;
    void random_WriteSeedUpper(uint32 seed) ;
    void random_WriteSeedLower(uint32 seed) ;
    uint32 random_ReadPolynomialUpper(void) ;
    uint32 random_ReadPolynomialLower(void) ;
    void random_WritePolynomialUpper(uint32 polynomial) 
                                                ;
    void random_WritePolynomialLower(uint32 polynomial) 
                                                ;
    
#endif  /* End (random_PRS_SIZE <= 32u) */

#if (random_RUN_MODE == random__CLOCKED)
    #if (random_PRS_SIZE <= 32u) /* 8-32 bits PRS */
        void random_ResetSeedInit(uint8 seed)  
                                            ;
    #else
        void random_ResetSeedInitUpper(uint32 seed) ;
        void random_ResetSeedInitLower(uint32 seed) ;
    #endif  /* End (random_PRS_SIZE <= 32u) */
#endif  /* End (random_RUN_MODE == random__CLOCKED) */


/***************************************
*    Initial Parameter Constants
***************************************/

#if (random_PRS_SIZE <= 32u)
    #define random_DEFAULT_POLYNOM            (0xB8u)
    #define random_DEFAULT_SEED               (0xFFu)
    
#else
    #define random_DEFAULT_POLYNOM_UPPER      (0x0u)
    #define random_DEFAULT_POLYNOM_LOWER      (0xB8u)
    #define random_DEFAULT_SEED_UPPER         (0x0u)
    #define random_DEFAULT_SEED_LOWER         (0xFFu)
    
#endif  /* End (random_PRS_SIZE <= 32u) */


/***************************************
*           API Constants
***************************************/

#define random_MASK                           (0xFFu)


/***************************************
*             Registers
***************************************/


#if (random_RUN_MODE == random__CLOCKED)
    #define random_EXECUTE_DFF_RESET  \
        (random_CONTROL_REG |= (random_INIT_STATE | random_CTRL_RESET_COMMON))
    
    #define random_EXECUTE_DFF_SET    \
        (random_CONTROL_REG |= random_CTRL_RESET_COMMON)

#else
    #define random_EXECUTE_DFF_RESET  \
        do { \
            random_CONTROL_REG |= (random_INIT_STATE | random_CTRL_RESET_COMMON | \
            random_CTRL_RISING_EDGE );  \
            random_CONTROL_REG &= ((uint8)~(random_CTRL_RESET_COMMON | \
            random_CTRL_RISING_EDGE));    \
        } while (0)
    
    #define random_EXECUTE_DFF_SET    \
        do { \
            random_CONTROL_REG |= (random_CTRL_RESET_COMMON | \
            random_CTRL_RISING_EDGE);\
            random_CONTROL_REG &= ((uint8)~(random_CTRL_RESET_COMMON | \
            random_CTRL_RISING_EDGE));    \
        } while (0)

    #define random_EXECUTE_STEP       \
        do { \
            random_CONTROL_REG |= random_CTRL_RISING_EDGE; \
            random_CONTROL_REG &= ((uint8)~random_CTRL_RISING_EDGE);    \
        } while (0)
    
#endif  /* End (random_RUN_MODE == random__CLOCKED) */

#if (random_TIME_MULTIPLEXING_ENABLE)
    
    #if (random_WAKEUP_BEHAVIOUR == random__RESUMEWORK)
        #define random_STATUS                     (*(reg8 *) random_Sts_StsReg__STATUS_REG )
        #define random_STATUS_PTR                 ( (reg8 *) random_Sts_StsReg__STATUS_REG )
    #endif  /* End (random_WAKEUP_BEHAVIOUR == random__RESUMEWORK) */
    
    #if (random_PRS_SIZE <= 16u)      /* 16 bits PRS */
        /* Polynomial */
        #define random_POLYNOM_A__D1_REG          (*(reg8 *) random_b0_PRSdp_a__D1_REG )
        #define random_POLYNOM_A__D1_PTR      	( (reg8 *) random_b0_PRSdp_a__D1_REG )
        #define random_POLYNOM_A__D0_REG          (*(reg8 *) random_b0_PRSdp_a__D0_REG )
        #define random_POLYNOM_A__D0_PTR      	( (reg8 *) random_b0_PRSdp_a__D0_REG )
        /* Seed */
        #define random_SEED_A__A1_REG             (*(reg8 *) random_b0_PRSdp_a__A1_REG )
        #define random_SEED_A__A1_PTR         	( (reg8 *) random_b0_PRSdp_a__A1_REG )
        #define random_SEED_A__A0_REG             (*(reg8 *) random_b0_PRSdp_a__A0_REG )
        #define random_SEED_A__A0_PTR         	( (reg8 *) random_b0_PRSdp_a__A0_REG )
        /* Seed COPY */
        #define random_SEED_COPY_A__A1_REG        (*(reg8 *) random_b0_PRSdp_a__F1_REG )
        #define random_SEED_COPY_A__A1_PTR    	( (reg8 *) random_b0_PRSdp_a__F1_REG )
        #define random_SEED_COPY_A__A0_REG        (*(reg8 *) random_b0_PRSdp_a__F0_REG )
        #define random_SEED_COPY_A__A0_PTR    	( (reg8 *) random_b0_PRSdp_a__F0_REG )
        
    #elif (random_PRS_SIZE <= 24u)      /* 24 bits PRS */
        /* Polynomial */
        #define random_POLYNOM_B__D1_REG          (*(reg8 *) random_b1_PRSdp_b__D1_REG )
        #define random_POLYNOM_B__D1_PTR      	( (reg8 *) random_b1_PRSdp_b__D1_REG )
        #define random_POLYNOM_B__D0_REG          (*(reg8 *) random_b1_PRSdp_b__D0_REG )
        #define random_POLYNOM_B__D0_PTR      	( (reg8 *) random_b1_PRSdp_b__D0_REG )
        #define random_POLYNOM_A__D0_REG          (*(reg8 *) random_b0_PRSdp_a__D0_REG )
        #define random_POLYNOM_A__D0_PTR     	 	( (reg8 *) random_b0_PRSdp_a__D0_REG )
        /* Seed */
        #define random_SEED_B__A1_REG             (*(reg8 *) random_b1_PRSdp_b__A1_REG )
        #define random_SEED_B__A1_PTR         	( (reg8 *) random_b1_PRSdp_b__A1_REG )
        #define random_SEED_B__A0_REG             (*(reg8 *) random_b1_PRSdp_b__A0_REG )
        #define random_SEED_B__A0_PTR     	    ( (reg8 *) random_b1_PRSdp_b__A0_REG )
        #define random_SEED_A__A0_REG             (*(reg8 *) random_b0_PRSdp_a__A0_REG )
        #define random_SEED_A__A0_PTR 	        ( (reg8 *) random_b0_PRSdp_a__A0_REG )
        /* Seed COPY */
        #define random_SEED_COPY_B__A1_REG        (*(reg8 *) random_b1_PRSdp_b__F1_REG )
        #define random_SEED_COPY_B__A1_PTR 	    ( (reg8 *) random_b1_PRSdp_b__F1_REG )
        #define random_SEED_COPY_B__A0_REG        (*(reg8 *) random_b1_PRSdp_b__F0_REG )
        #define random_SEED_COPY_B__A0_PTR	    ( (reg8 *) random_b1_PRSdp_b__F0_REG )
        #define random_SEED_COPY_A__A0_REG        (*(reg8 *) random_b0_PRSdp_a__F0_REG )
        #define random_SEED_COPY_A__A0_PTR    	( (reg8 *) random_b0_PRSdp_a__F0_REG )
        
    #elif (random_PRS_SIZE <= 32u)      /* 32 bits PRS */
        /* Polynomial */
        #define random_POLYNOM_B__D1_REG          (*(reg8 *) random_b1_PRSdp_b__D1_REG )
        #define random_POLYNOM_B__D1_PTR    		( (reg8 *) random_b1_PRSdp_b__D1_REG )
        #define random_POLYNOM_A__D1_REG          (*(reg8 *) random_b0_PRSdp_a__D1_REG )
        #define random_POLYNOM_A__D1_PTR     		( (reg8 *) random_b0_PRSdp_a__D1_REG )
        #define random_POLYNOM_B__D0_REG          (*(reg8 *) random_b1_PRSdp_b__D0_REG )
        #define random_POLYNOM_B__D0_PTR		    ( (reg8 *) random_b1_PRSdp_b__D0_REG )
        #define random_POLYNOM_A__D0_REG          (*(reg8 *) random_b0_PRSdp_a__D0_REG )
        #define random_POLYNOM_A__D0_PTR  	    ( (reg8 *) random_b0_PRSdp_a__D0_REG )
        /* Seed */
        #define random_SEED_B__A1_REG             (*(reg8 *) random_b1_PRSdp_b__A1_REG )
        #define random_SEED_B__A1_PTR    		    ( (reg8 *) random_b1_PRSdp_b__A1_REG )
        #define random_SEED_A__A1_REG         	(*(reg8 *) random_b0_PRSdp_a__A1_REG )
        #define random_SEED_A__A1_PTR      	    ( (reg8 *) random_b0_PRSdp_a__A1_REG )
        #define random_SEED_B__A0_REG             (*(reg8 *) random_b1_PRSdp_b__A0_REG )
        #define random_SEED_B__A0_PTR     	    ( (reg8 *) random_b1_PRSdp_b__A0_REG )
        #define random_SEED_A__A0_REG             (*(reg8 *) random_b0_PRSdp_a__A0_REG )
        #define random_SEED_A__A0_PTR     	    ( (reg8 *) random_b0_PRSdp_a__A0_REG )
        /* Seed COPY */
        #define random_SEED_COPY_B__A1_REG        (*(reg8 *) random_b1_PRSdp_b__F1_REG )
        #define random_SEED_COPY_B__A1_PTR  		( (reg8 *) random_b1_PRSdp_b__F1_REG )
        #define random_SEED_COPY_A__A1_REG        (*(reg8 *) random_b0_PRSdp_a__F1_REG )
        #define random_SEED_COPY_A__A1_PTR   	 	( (reg8 *) random_b0_PRSdp_a__F1_REG )
        #define random_SEED_COPY_B__A0_REG        (*(reg8 *) random_b1_PRSdp_b__F0_REG )
        #define random_SEED_COPY_B__A0_PTR  	  	( (reg8 *) random_b1_PRSdp_b__F0_REG )
        #define random_SEED_COPY_A__A0_REG        (*(reg8 *) random_b0_PRSdp_a__F0_REG )
        #define random_SEED_COPY_A__A0_PTR   	 	( (reg8 *) random_b0_PRSdp_a__F0_REG )
        
    #elif (random_PRS_SIZE <= 40u)      /* 40 bits PRS */
        /* Polynomial Upper */
        #define random_POLYNOM_UPPER_C__D1_REG            (*(reg8 *) random_b2_PRSdp_c__D1_REG )
        #define random_POLYNOM_UPPER_C__D1_PTR        	( (reg8 *) random_b2_PRSdp_c__D1_REG )
        /* Polynomial Lower */
        #define random_POLYNOM_LOWER_B__D1_REG            (*(reg8 *) random_b1_PRSdp_b__D1_REG )
        #define random_POLYNOM_LOWER_B__D1_PTR        	( (reg8 *) random_b1_PRSdp_b__D1_REG )
        #define random_POLYNOM_LOWER_C__D0_REG            (*(reg8 *) random_b2_PRSdp_c__D0_REG )
        #define random_POLYNOM_LOWER_C__D0_PTR        	( (reg8 *) random_b2_PRSdp_c__D0_REG )
        #define random_POLYNOM_LOWER_B__D0_REG            (*(reg8 *) random_b1_PRSdp_b__D0_REG )
        #define random_POLYNOM_LOWER_B__D0_PTR        	( (reg8 *) random_b1_PRSdp_b__D0_REG )
        #define random_POLYNOM_LOWER_A__D0_REG            (*(reg8 *) random_b0_PRSdp_a__D0_REG )
        #define random_POLYNOM_LOWER_A__D0_PTR        	( (reg8 *) random_b0_PRSdp_a__D0_REG )
        /* Seed Upper */
        #define random_SEED_UPPER_C__A1_REG               (*(reg8 *) random_b2_PRSdp_c__A1_REG )
        #define random_SEED_UPPER_C__A1_PTR           	( (reg8 *) random_b2_PRSdp_c__A1_REG )
        /* Seed Lower */
        #define random_SEED_LOWER_B__A1_REG               (*(reg8 *) random_b1_PRSdp_b__A1_REG )
        #define random_SEED_LOWER_B__A1_PTR           	( (reg8 *) random_b1_PRSdp_b__A1_REG )
        #define random_SEED_LOWER_C__A0_REG               (*(reg8 *) random_b2_PRSdp_c__A0_REG )
        #define random_SEED_LOWER_C__A0_PTR           	( (reg8 *) random_b2_PRSdp_c__A0_REG )
        #define random_SEED_LOWER_B__A0_REG               (*(reg8 *) random_b1_PRSdp_b__A0_REG )
        #define random_SEED_LOWER_B__A0_PTR           	( (reg8 *) random_b1_PRSdp_b__A0_REG )
        #define random_SEED_LOWER_A__A0_REG               (*(reg8 *) random_b0_PRSdp_a__A0_REG )
        #define random_SEED_LOWER_A__A0_PTR           	( (reg8 *) random_b0_PRSdp_a__A0_REG )
        /* Seed COPY Upper */
        #define random_SEED_UPPER_COPY_C__A1_REG          (*(reg8 *) random_b2_PRSdp_c__F1_REG )
        #define random_SEED_UPPER_COPY_C__A1_PTR      	( (reg8 *) random_b2_PRSdp_c__F1_REG )
        /* Seed COPY Lower */
        #define random_SEED_LOWER_COPY_B__A1_REG          (*(reg8 *) random_b1_PRSdp_b__F1_REG )
        #define random_SEED_LOWER_COPY_B__A1_PTR      	( (reg8 *) random_b1_PRSdp_b__F1_REG )
        #define random_SEED_LOWER_COPY_C__A0_REG          (*(reg8 *) random_b2_PRSdp_c__F0_REG )
        #define random_SEED_LOWER_COPY_C__A0_PTR      	( (reg8 *) random_b2_PRSdp_c__F0_REG )
        #define random_SEED_LOWER_COPY_B__A0_REG          (*(reg8 *) random_b1_PRSdp_b__F0_REG )
        #define random_SEED_LOWER_COPY_B__A0_PTR      	( (reg8 *) random_b1_PRSdp_b__F0_REG )
        #define random_SEED_LOWER_COPY_A__A0_REG          (*(reg8 *) random_b0_PRSdp_a__F0_REG )
        #define random_SEED_LOWER_COPY_A__A0_PTR      	( (reg8 *) random_b0_PRSdp_a__F0_REG )
        
    #elif (random_PRS_SIZE <= 48u)      /* 48 bits PRS */
        /* Polynomial Upper */
        #define random_POLYNOM_UPPER_C__D1_REG            (*(reg8 *) random_b2_PRSdp_c__D1_REG )
        #define random_POLYNOM_UPPER_C__D1_PTR        	( (reg8 *) random_b2_PRSdp_c__D1_REG )
        #define random_POLYNOM_UPPER_B__D1_REG            (*(reg8 *) random_b1_PRSdp_b__D1_REG )
        #define random_POLYNOM_UPPER_B__D1_PTR        	( (reg8 *) random_b1_PRSdp_b__D1_REG )
        /* Polynomial Lower */
        #define random_POLYNOM_LOWER_A__D1_REG            (*(reg8 *) random_b0_PRSdp_a__D1_REG )
        #define random_POLYNOM_LOWER_A__D1_PTR        	( (reg8 *) random_b0_PRSdp_a__D1_REG )
        #define random_POLYNOM_LOWER_C__D0_REG            (*(reg8 *) random_b2_PRSdp_c__D0_REG )
        #define random_POLYNOM_LOWER_C__D0_PTR        	( (reg8 *) random_b2_PRSdp_c__D0_REG )
        #define random_POLYNOM_LOWER_B__D0_REG            (*(reg8 *) random_b1_PRSdp_b__D0_REG )
        #define random_POLYNOM_LOWER_B__D0_PTR        	( (reg8 *) random_b1_PRSdp_b__D0_REG )
        #define random_POLYNOM_LOWER_A__D0_REG            (*(reg8 *) random_b0_PRSdp_a__D0_REG )
        #define random_POLYNOM_LOWER_A__D0_PTR        	( (reg8 *) random_b0_PRSdp_a__D0_REG )
        /* Seed Upper */
        #define random_SEED_UPPER_C__A1_REG               (*(reg8 *) random_b2_PRSdp_c__A1_REG )
        #define random_SEED_UPPER_C__A1_PTR           	( (reg8 *) random_b2_PRSdp_c__A1_REG )
        #define random_SEED_UPPER_B__A1_REG               (*(reg8 *) random_b1_PRSdp_b__A1_REG )
        #define random_SEED_UPPER_B__A1_PTR           	( (reg8 *) random_b1_PRSdp_b__A1_REG )
        /* Seed Lower */
        #define random_SEED_LOWER_A__A1_REG               (*(reg8 *) random_b0_PRSdp_a__A1_REG )
        #define random_SEED_LOWER_A__A1_PTR           	( (reg8 *) random_b0_PRSdp_a__A1_REG )
        #define random_SEED_LOWER_C__A0_REG               (*(reg8 *) random_b2_PRSdp_c__A0_REG )
        #define random_SEED_LOWER_C__A0_PTR           	( (reg8 *) random_b2_PRSdp_c__A0_REG )
        #define random_SEED_LOWER_B__A0_REG               (*(reg8 *) random_b1_PRSdp_b__A0_REG )
        #define random_SEED_LOWER_B__A0_PTR           	( (reg8 *) random_b1_PRSdp_b__A0_REG )
        #define random_SEED_LOWER_A__A0_REG               (*(reg8 *) random_b0_PRSdp_a__A0_REG )
        #define random_SEED_LOWER_A__A0_PTR           	( (reg8 *) random_b0_PRSdp_a__A0_REG )
        /* Seed COPY Upper */
        #define random_SEED_UPPER_COPY_C__A1_REG          (*(reg8 *) random_b2_PRSdp_c__F1_REG )
        #define random_SEED_UPPER_COPY_C__A1_PTR      	( (reg8 *) random_b2_PRSdp_c__F1_REG )
        #define random_SEED_UPPER_COPY_B__A1_REG          (*(reg8 *) random_b1_PRSdp_b__F1_REG )
        #define random_SEED_UPPER_COPY_B__A1_PTR      	( (reg8 *) random_b1_PRSdp_b__F1_REG )
        /* Seed COPY Lower */
        #define random_SEED_LOWER_COPY_A__A1_REG          (*(reg8 *) random_b0_PRSdp_a__F1_REG )
        #define random_SEED_LOWER_COPY_A__A1_PTR      	( (reg8 *) random_b0_PRSdp_a__F1_REG )
        #define random_SEED_LOWER_COPY_C__A0_REG          (*(reg8 *) random_b2_PRSdp_c__F0_REG )
        #define random_SEED_LOWER_COPY_C__A0_PTR      	( (reg8 *) random_b2_PRSdp_c__F0_REG )
        #define random_SEED_LOWER_COPY_B__A0_REG          (*(reg8 *) random_b1_PRSdp_b__F0_REG )
        #define random_SEED_LOWER_COPY_B__A0_PTR      	( (reg8 *) random_b1_PRSdp_b__F0_REG )
        #define random_SEED_LOWER_COPY_A__A0_REG          (*(reg8 *) random_b0_PRSdp_a__F0_REG )
        #define random_SEED_LOWER_COPY_A__A0_PTR      	( (reg8 *) random_b0_PRSdp_a__F0_REG )
        
    #elif (random_PRS_SIZE <= 56u)      /* 56 bits PRS */
        /* Polynom Upper */
        #define random_POLYNOM_UPPER_D__D1_REG            (*(reg8 *) random_b3_PRSdp_d__D1_REG )
        #define random_POLYNOM_UPPER_D__D1_PTR        	( (reg8 *) random_b3_PRSdp_d__D1_REG )
        #define random_POLYNOM_UPPER_C__D1_REG            (*(reg8 *) random_b2_PRSdp_c__D1_REG )
        #define random_POLYNOM_UPPER_C__D1_PTR        	( (reg8 *) random_b2_PRSdp_c__D1_REG )
        #define random_POLYNOM_UPPER_B__D1_REG            (*(reg8 *) random_b1_PRSdp_b__D1_REG )
        #define random_POLYNOM_UPPER_B__D1_PTR        	( (reg8 *) random_b1_PRSdp_b__D1_REG )
        /* Polynom Lower */
        #define random_POLYNOM_LOWER_D__D0_REG            (*(reg8 *) random_b3_PRSdp_d__D0_REG )
        #define random_POLYNOM_LOWER_D__D0_PTR        	( (reg8 *) random_b3_PRSdp_d__D0_REG )
        #define random_POLYNOM_LOWER_C__D0_REG            (*(reg8 *) random_b2_PRSdp_c__D0_REG )
        #define random_POLYNOM_LOWER_C__D0_PTR        	( (reg8 *) random_b2_PRSdp_c__D0_REG )
        #define random_POLYNOM_LOWER_B__D0_REG            (*(reg8 *) random_b1_PRSdp_b__D0_REG )
        #define random_POLYNOM_LOWER_B__D0_PTR        	( (reg8 *) random_b1_PRSdp_b__D0_REG )
        #define random_POLYNOM_LOWER_A__D0_REG            (*(reg8 *) random_b0_PRSdp_a__D0_REG )
        #define random_POLYNOM_LOWER_A__D0_PTR        	( (reg8 *) random_b0_PRSdp_a__D0_REG )
        /* Seed Upper */
        #define random_SEED_UPPER_D__A1_REG               (*(reg8 *) random_b3_PRSdp_d__A1_REG )
        #define random_SEED_UPPER_D__A1_PTR           	( (reg8 *) random_b3_PRSdp_d__A1_REG )
        #define random_SEED_UPPER_C__A1_REG               (*(reg8 *) random_b2_PRSdp_c__A1_REG )
        #define random_SEED_UPPER_C__A1_PTR           	( (reg8 *) random_b2_PRSdp_c__A1_REG )
        #define random_SEED_UPPER_B__A1_REG               (*(reg8 *) random_b1_PRSdp_b__A1_REG )
        #define random_SEED_UPPER_B__A1_PTR           	( (reg8 *) random_b1_PRSdp_b__A1_REG )
        /* Seed Lower */
        #define random_SEED_LOWER_D__A0_REG               (*(reg8 *) random_b3_PRSdp_d__A0_REG )
        #define random_SEED_LOWER_D__A0_PTR           	( (reg8 *) random_b3_PRSdp_d__A0_REG )
        #define random_SEED_LOWER_C__A0_REG               (*(reg8 *) random_b2_PRSdp_c__A0_REG )
        #define random_SEED_LOWER_C__A0_PTR           	( (reg8 *) random_b2_PRSdp_c__A0_REG )
        #define random_SEED_LOWER_B__A0_REG               (*(reg8 *) random_b1_PRSdp_b__A0_REG )
        #define random_SEED_LOWER_B__A0_PTR           	( (reg8 *) random_b1_PRSdp_b__A0_REG )
        #define random_SEED_LOWER_A__A0_REG               (*(reg8 *) random_b0_PRSdp_a__A0_REG )
        #define random_SEED_LOWER_A__A0_PTR           	( (reg8 *) random_b0_PRSdp_a__A0_REG )
        /* COPY Seed Upper */
        #define random_SEED_UPPER_COPY_D__A1_REG          (*(reg8 *) random_b3_PRSdp_d__F1_REG )
        #define random_SEED_UPPER_COPY_D__A1_PTR      	( (reg8 *) random_b3_PRSdp_d__F1_REG )
        #define random_SEED_UPPER_COPY_C__A1_REG          (*(reg8 *) random_b2_PRSdp_c__F1_REG )
        #define random_SEED_UPPER_COPY_C__A1_PTR      	( (reg8 *) random_b2_PRSdp_c__F1_REG )
        #define random_SEED_UPPER_COPY_B__A1_REG          (*(reg8 *) random_b1_PRSdp_b__F1_REG )
        #define random_SEED_UPPER_COPY_B__A1_PTR      	( (reg8 *) random_b1_PRSdp_b__F1_REG )
        /* COPY Seed Lower */
        #define random_SEED_LOWER_COPY_D__A0_REG          (*(reg8 *) random_b3_PRSdp_d__F0_REG )
        #define random_SEED_LOWER_COPY_D__A0_PTR      	( (reg8 *) random_b3_PRSdp_d__F0_REG )
        #define random_SEED_LOWER_COPY_C__A0_REG          (*(reg8 *) random_b2_PRSdp_c__F0_REG )
        #define random_SEED_LOWER_COPY_C__A0_PTR      	( (reg8 *) random_b2_PRSdp_c__F0_REG )
        #define random_SEED_LOWER_COPY_B__A0_REG          (*(reg8 *) random_b1_PRSdp_b__F0_REG )
        #define random_SEED_LOWER_COPY_B__A0_PTR      	( (reg8 *) random_b1_PRSdp_b__F0_REG )
        #define random_SEED_LOWER_COPY_A__A0_REG          (*(reg8 *) random_b0_PRSdp_a__F0_REG )
        #define random_SEED_LOWER_COPY_A__A0_PTR      	( (reg8 *) random_b0_PRSdp_a__F0_REG )
        
    #else                                        /* 64 bits PRS */
        /* Polynom Upper */
        #define random_POLYNOM_UPPER_D__D1_REG            (*(reg8 *) random_b3_PRSdp_d__D1_REG )
        #define random_POLYNOM_UPPER_D__D1_PTR        	( (reg8 *) random_b3_PRSdp_d__D1_REG )
        #define random_POLYNOM_UPPER_C__D1_REG            (*(reg8 *) random_b2_PRSdp_c__D1_REG )
        #define random_POLYNOM_UPPER_C__D1_PTR        	( (reg8 *) random_b2_PRSdp_c__D1_REG )
        #define random_POLYNOM_UPPER_B__D1_REG            (*(reg8 *) random_b1_PRSdp_b__D1_REG )
        #define random_POLYNOM_UPPER_B__D1_PTR        	( (reg8 *) random_b1_PRSdp_b__D1_REG )
        #define random_POLYNOM_UPPER_A__D1_REG            (*(reg8 *) random_b0_PRSdp_a__D1_REG )
        #define random_POLYNOM_UPPER_A__D1_PTR        	( (reg8 *) random_b0_PRSdp_a__D1_REG )
        /* Polynom Lower */
        #define random_POLYNOM_LOWER_D__D0_REG            (*(reg8 *) random_b3_PRSdp_d__D0_REG )
        #define random_POLYNOM_LOWER_D__D0_PTR        	( (reg8 *) random_b3_PRSdp_d__D0_REG )
        #define random_POLYNOM_LOWER_C__D0_REG            (*(reg8 *) random_b2_PRSdp_c__D0_REG )
        #define random_POLYNOM_LOWER_C__D0_PTR        	( (reg8 *) random_b2_PRSdp_c__D0_REG )
        #define random_POLYNOM_LOWER_B__D0_REG            (*(reg8 *) random_b1_PRSdp_b__D0_REG )
        #define random_POLYNOM_LOWER_B__D0_PTR        	( (reg8 *) random_b1_PRSdp_b__D0_REG )
        #define random_POLYNOM_LOWER_A__D0_REG            (*(reg8 *) random_b0_PRSdp_a__D0_REG )
        #define random_POLYNOM_LOWER_A__D0_PTR        	( (reg8 *) random_b0_PRSdp_a__D0_REG )
        /* Seed Upper */
        #define random_SEED_UPPER_D__A1_REG               (*(reg8 *) random_b3_PRSdp_d__A1_REG )
        #define random_SEED_UPPER_D__A1_PTR           	( (reg8 *) random_b3_PRSdp_d__A1_REG )
        #define random_SEED_UPPER_C__A1_REG               (*(reg8 *) random_b2_PRSdp_c__A1_REG )
        #define random_SEED_UPPER_C__A1_PTR           	( (reg8 *) random_b2_PRSdp_c__A1_REG )
        #define random_SEED_UPPER_B__A1_REG               (*(reg8 *) random_b1_PRSdp_b__A1_REG )
        #define random_SEED_UPPER_B__A1_PTR           	( (reg8 *) random_b1_PRSdp_b__A1_REG )
        #define random_SEED_UPPER_A__A1_REG               (*(reg8 *) random_b0_PRSdp_a__A1_REG )
        #define random_SEED_UPPER_A__A1_PTR           	( (reg8 *) random_b0_PRSdp_a__A1_REG )
        /* Seed Lower */
        #define random_SEED_LOWER_D__A0_REG               (*(reg8 *) random_b3_PRSdp_d__A0_REG )
        #define random_SEED_LOWER_D__A0_PTR           	( (reg8 *) random_b3_PRSdp_d__A0_REG )
        #define random_SEED_LOWER_C__A0_REG               (*(reg8 *) random_b2_PRSdp_c__A0_REG )
        #define random_SEED_LOWER_C__A0_PTR           	( (reg8 *) random_b2_PRSdp_c__A0_REG )
        #define random_SEED_LOWER_B__A0_REG               (*(reg8 *) random_b1_PRSdp_b__A0_REG )
        #define random_SEED_LOWER_B__A0_PTR           	( (reg8 *) random_b1_PRSdp_b__A0_REG )
        #define random_SEED_LOWER_A__A0_REG               (*(reg8 *) random_b0_PRSdp_a__A0_REG )
        #define random_SEED_LOWER_A__A0_PTR           	( (reg8 *) random_b0_PRSdp_a__A0_REG )
        /* COPY Seed Upper */
        #define random_SEED_UPPER_COPY_D__A1_REG          (*(reg8 *) random_b3_PRSdp_d__F1_REG )
        #define random_SEED_UPPER_COPY_D__A1_PTR      	( (reg8 *) random_b3_PRSdp_d__F1_REG )
        #define random_SEED_UPPER_COPY_C__A1_REG          (*(reg8 *) random_b2_PRSdp_c__F1_REG )
        #define random_SEED_UPPER_COPY_C__A1_PTR      	( (reg8 *) random_b2_PRSdp_c__F1_REG )
        #define random_SEED_UPPER_COPY_B__A1_REG          (*(reg8 *) random_b1_PRSdp_b__F1_REG )
        #define random_SEED_UPPER_COPY_B__A1_PTR      	( (reg8 *) random_b1_PRSdp_b__F1_REG )
        #define random_SEED_UPPER_COPY_A__A1_REG          (*(reg8 *) random_b0_PRSdp_a__F1_REG )
        #define random_SEED_UPPER_COPY_A__A1_PTR      	( (reg8 *) random_b0_PRSdp_a__F1_REG )
        /* COPY Seed Lower */
        #define random_SEED_LOWER_COPY_D__A0_REG          (*(reg8 *) random_b3_PRSdp_d__F0_REG )
        #define random_SEED_LOWER_COPY_D__A0_PTR      	( (reg8 *) random_b3_PRSdp_d__F0_REG )
        #define random_SEED_LOWER_COPY_C__A0_REG          (*(reg8 *) random_b2_PRSdp_c__F0_REG )
        #define random_SEED_LOWER_COPY_C__A0_PTR    		( (reg8 *) random_b2_PRSdp_c__F0_REG )
        #define random_SEED_LOWER_COPY_B__A0_REG          (*(reg8 *) random_b1_PRSdp_b__F0_REG )
        #define random_SEED_LOWER_COPY_B__A0_PTR      	( (reg8 *) random_b1_PRSdp_b__F0_REG )
        #define random_SEED_LOWER_COPY_A__A0_REG          (*(reg8 *) random_b0_PRSdp_a__F0_REG )
        #define random_SEED_LOWER_COPY_A__A0_PTR      	( (reg8 *) random_b0_PRSdp_a__F0_REG )
    #endif  /* End (random_PRS_SIZE <= 8u) */

#else
    #if (CY_PSOC3 || CY_PSOC5)
        #if (random_PRS_SIZE <= 32u)         /* 8-32 bits PRS */
            /* Polynomial */
            #define random_POLYNOM_PTR      ( (reg8 *)\
                                                random_sC8_PRSdp_u0__D0_REG )
            /* Seed */
            #define random_SEED_PTR         ( (reg8 *)\
                                                random_sC8_PRSdp_u0__A0_REG )
            /* Seed COPY */
            #define random_SEED_COPY_PTR    ( (reg8 *)\
                                                random_sC8_PRSdp_u0__F0_REG )
            
        #endif  /* End (random_PRS_SIZE <= 32u) */
    #else /* PSoC4 */
        #if (random_PRS_SIZE <= 8u)         /* 8 bits PRS */
            /* Polynomial */
            #define random_POLYNOM_PTR      ( (reg8 *)\
                                                random_sC8_PRSdp_u0__D0_REG )
            /* Seed */
            #define random_SEED_PTR         ( (reg8 *)\
                                                random_sC8_PRSdp_u0__A0_REG )
            /* Seed COPY */
            #define random_SEED_COPY_PTR    ( (reg8 *)\
                                                random_sC8_PRSdp_u0__F0_REG )
            
        #elif (random_PRS_SIZE <= 16u)         /* 16 bits PRS */
            /* Polynomial */
            #define random_POLYNOM_PTR      ( (reg8 *)\
                                            random_sC8_PRSdp_u0__16BIT_D0_REG )
            /* Seed */
            #define random_SEED_PTR         ( (reg8 *)\
                                            random_sC8_PRSdp_u0__16BIT_A0_REG )
            /* Seed COPY */
            #define random_SEED_COPY_PTR    ( (reg8 *)\
                                            random_sC8_PRSdp_u0__16BIT_F0_REG )

        #elif (random_PRS_SIZE <= 24u)         /* 24 bits PRS */
            /* Polynomial */
            #define random_POLYNOM_PTR      ( (reg8 *)\
                                                random_sC8_PRSdp_u0__D0_REG )
            /* Seed */
            #define random_SEED_PTR         ( (reg8 *)\
                                                random_sC8_PRSdp_u0__A0_REG )
            /* Seed COPY */
            #define random_SEED_COPY_PTR    ( (reg8 *)\
                                                random_sC8_PRSdp_u0__F0_REG )
         
         #else                                          /* 32 bits PRS */
            /* Polynomial */
            #define random_POLYNOM_PTR      ( (reg8 *)\
                                            random_sC8_PRSdp_u0__32BIT_D0_REG )
            /* Seed */
            #define random_SEED_PTR         ( (reg8 *)\
                                            random_sC8_PRSdp_u0__32BIT_A0_REG )
            /* Seed COPY */
            #define random_SEED_COPY_PTR    ( (reg8 *)\
                                            random_sC8_PRSdp_u0__32BIT_F0_REG )
         
        #endif  /* End (random_PRS_SIZE <= 32u) */

    #endif  /* End (CY_PSOC3 || CY_PSOC5) */
    
#endif  /* End (random_TIME_MULTIPLEXING_ENABLE) */


#if (random_RUN_MODE == random__CLOCKED)
    /* Aux control */
    #if (random_TIME_MULTIPLEXING_ENABLE)    
        #if (random_PRS_SIZE <= 16u)      /* 8-16 bits PRS */
            #define random_AUX_CONTROL_A_REG      (*(reg8 *) random_b0_PRSdp_a__DP_AUX_CTL_REG )
            #define random_AUX_CONTROL_A_PTR      ( (reg8 *) random_b0_PRSdp_a__DP_AUX_CTL_REG )
            
        #elif (random_PRS_SIZE <= 24u)      /* 24-39 bits PRS */
            #define random_AUX_CONTROL_A_REG      (*(reg8 *) random_b0_PRSdp_a__DP_AUX_CTL_REG )
            #define random_AUX_CONTROL_A_PTR      ( (reg8 *) random_b0_PRSdp_a__DP_AUX_CTL_REG )
            #define random_AUX_CONTROL_B_REG      (*(reg8 *) random_b1_PRSdp_b__DP_AUX_CTL_REG )
            #define random_AUX_CONTROL_B_PTR      ( (reg8 *) random_b1_PRSdp_b__DP_AUX_CTL_REG )
            
        #elif (random_PRS_SIZE <= 40u)      /* 40-55 bits PRS */
            #define random_AUX_CONTROL_A_REG      (*(reg8 *) random_b0_PRSdp_a__DP_AUX_CTL_REG )
            #define random_AUX_CONTROL_A_PTR      ( (reg8 *) random_b0_PRSdp_a__DP_AUX_CTL_REG )
            #define random_AUX_CONTROL_B_REG      (*(reg8 *) random_b1_PRSdp_b__DP_AUX_CTL_REG )
            #define random_AUX_CONTROL_B_PTR      ( (reg8 *) random_b1_PRSdp_b__DP_AUX_CTL_REG )
            #define random_AUX_CONTROL_C_REG      (*(reg8 *) random_b2_PRSdp_c__DP_AUX_CTL_REG )
            #define random_AUX_CONTROL_C_PTR      ( (reg8 *) random_b2_PRSdp_c__DP_AUX_CTL_REG )

        #else                                         /* 56-64 bits PRS */
            #define random_AUX_CONTROL_A_REG      (*(reg8 *) random_b0_PRSdp_a__DP_AUX_CTL_REG )
            #define random_AUX_CONTROL_A_PTR      ( (reg8 *) random_b0_PRSdp_a__DP_AUX_CTL_REG )
            #define random_AUX_CONTROL_B_REG      (*(reg8 *) random_b1_PRSdp_b__DP_AUX_CTL_REG )
            #define random_AUX_CONTROL_B_PTR      ( (reg8 *) random_b1_PRSdp_b__DP_AUX_CTL_REG )
            #define random_AUX_CONTROL_C_REG      (*(reg8 *) random_b2_PRSdp_c__DP_AUX_CTL_REG )
            #define random_AUX_CONTROL_C_PTR      ( (reg8 *) random_b2_PRSdp_c__DP_AUX_CTL_REG )
			#define random_AUX_CONTROL_D_REG      (*(reg8 *) random_b3_PRSdp_d__DP_AUX_CTL_REG )
            #define random_AUX_CONTROL_D_PTR      ( (reg8 *) random_b3_PRSdp_d__DP_AUX_CTL_REG )     
																							
        #endif  /* End (random_PRS_SIZE <= 8u) */
    
    #else
        #if (random_PRS_SIZE <= 8u)      /* 8 bits PRS */
            #define random_AUX_CONTROL_A_REG      (*(reg8 *) random_sC8_PRSdp_u0__DP_AUX_CTL_REG )
            #define random_AUX_CONTROL_A_PTR      ( (reg8 *) random_sC8_PRSdp_u0__DP_AUX_CTL_REG )
            
        #elif (random_PRS_SIZE <= 16u)      /* 16 bits PRS */
            #define random_AUX_CONTROL_A_REG      (*(reg8 *) random_sC16_PRSdp_u0__DP_AUX_CTL_REG )
            #define random_AUX_CONTROL_A_PTR      ( (reg8 *) random_sC16_PRSdp_u0__DP_AUX_CTL_REG )
            #define random_AUX_CONTROL_B_REG      (*(reg8 *) random_sC16_PRSdp_u1__DP_AUX_CTL_REG )
            #define random_AUX_CONTROL_B_PTR      ( (reg8 *) random_sC16_PRSdp_u1__DP_AUX_CTL_REG )
            
        #elif (random_PRS_SIZE <= 24u)      /* 24-39 bits PRS */
            #define random_AUX_CONTROL_A_REG      (*(reg8 *) random_sC24_PRSdp_u0__DP_AUX_CTL_REG )
            #define random_AUX_CONTROL_A_PTR      ( (reg8 *) random_sC24_PRSdp_u0__DP_AUX_CTL_REG )
            #define random_AUX_CONTROL_B_REG      (*(reg8 *) random_sC24_PRSdp_u1__DP_AUX_CTL_REG )
            #define random_AUX_CONTROL_B_PTR      ( (reg8 *) random_sC24_PRSdp_u1__DP_AUX_CTL_REG )
            #define random_AUX_CONTROL_C_REG      (*(reg8 *) random_sC24_PRSdp_u2__DP_AUX_CTL_REG )
            #define random_AUX_CONTROL_C_PTR      ( (reg8 *) random_sC24_PRSdp_u2__DP_AUX_CTL_REG )
                
        #elif (random_PRS_SIZE <= 32u)      /* 40-55 bits PRS */
            #define random_AUX_CONTROL_A_REG      (*(reg8 *) random_sC32_PRSdp_u0__DP_AUX_CTL_REG )
            #define random_AUX_CONTROL_A_PTR      ( (reg8 *) random_sC32_PRSdp_u0__DP_AUX_CTL_REG )
            #define random_AUX_CONTROL_B_REG      (*(reg8 *) random_sC32_PRSdp_u1__DP_AUX_CTL_REG )
            #define random_AUX_CONTROL_B_PTR      ( (reg8 *) random_sC32_PRSdp_u1__DP_AUX_CTL_REG )
            #define random_AUX_CONTROL_C_REG      (*(reg8 *) random_sC32_PRSdp_u2__DP_AUX_CTL_REG )
            #define random_AUX_CONTROL_C_PTR      ( (reg8 *) random_sC32_PRSdp_u2__DP_AUX_CTL_REG )
            #define random_AUX_CONTROL_D_REG      (*(reg8 *) random_sC32_PRSdp_u3__DP_AUX_CTL_REG )
            #define random_AUX_CONTROL_D_PTR      ( (reg8 *) random_sC32_PRSdp_u3__DP_AUX_CTL_REG )
    
        #else                                         /* 56-64 bits PRS */
            /* Not supported */                    
        #endif  /* End (random_PRS_SIZE <= 8u) */
    
    #endif  /* End (random_TIME_MULTIPLEXING_ENABLE) */
    
#endif  /* End (random_RUN_MODE == random__CLOCKED) */

#define random_CONTROL_REG               (*(reg8 *) random_ClkSp_CtrlReg__CONTROL_REG )
#define random_CONTROL_PTR               ( (reg8 *) random_ClkSp_CtrlReg__CONTROL_REG )
    

/***************************************
*       Register Constants
***************************************/

/* Control register definitions */

#define random_CTRL_ENABLE                    (0x01u)
#define random_CTRL_RISING_EDGE               (0x02u)
#define random_CTRL_RESET_COMMON              (0x04u)
#define random_CTRL_RESET_CI                  (0x08u)
#define random_CTRL_RESET_SI                  (0x10u)
#define random_CTRL_RESET_SO                  (0x20u)
#define random_CTRL_RESET_STATE0              (0x40u)
#define random_CTRL_RESET_STATE1              (0x80u)

#define random_INIT_STATE                     ( random_CTRL_RESET_CI |\
                                                          random_CTRL_RESET_SI |\
                                                          random_CTRL_RESET_SO |\
                                                          random_CTRL_RESET_STATE0 |\
                                                          random_CTRL_RESET_STATE1 )
                                                          
/* Status register definitions */
#define random_STS_RESET_CI                   (0x08u)
#define random_STS_RESET_SI                   (0x10u)
#define random_STS_RESET_SO                   (0x20u)
#define random_STS_RESET_STATE0               (0x40u)
#define random_STS_RESET_STATE1               (0x80u)

/* Aux Control register definitions */
#define random_AUXCTRL_FIFO_SINGLE_REG        (0x03u)

#define random_NORMAL_SEQUENCE                (0x01u)

#define random_IS_PRS_ENABLE(reg)             (((reg) & random_CTRL_ENABLE) != 0u)

#endif  /* End CY_PRS_random_H */


/* [] END OF FILE */
