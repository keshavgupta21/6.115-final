/*******************************************************************************
* File Name: eeprom.h
* Version 3.0
*
*  Description:
*   Provides the function definitions for the EEPROM APIs.
*
********************************************************************************
* Copyright 2008-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_EEPROM_eeprom_H)
#define CY_EEPROM_eeprom_H

#include "cydevice_trm.h"
#include "CyFlash.h"

#if !defined(CY_PSOC5LP)
    #error Component EEPROM_v3_0 requires cy_boot v3.0 or later
#endif /* (CY_PSOC5LP) */


/***************************************
*        Function Prototypes
***************************************/

void eeprom_Enable(void) ;
void eeprom_Start(void) ;
void eeprom_Stop (void) ;
cystatus eeprom_WriteByte(uint8 dataByte, uint16 address) \
                                            ;
uint8 eeprom_ReadByte(uint16 address) ;
uint8 eeprom_UpdateTemperature(void) ;
cystatus eeprom_EraseSector(uint8 sectorNumber) ;
cystatus eeprom_Write(const uint8 * rowData, uint8 rowNumber) ;
cystatus eeprom_StartWrite(const uint8 * rowData, uint8 rowNumber) \
                                                ;
cystatus eeprom_StartErase(uint8 sectorNumber) ;
cystatus eeprom_Query(void) ;
cystatus eeprom_ByteWritePos(uint8 dataByte, uint8 rowNumber, uint8 byteNumber) \
                                                ;


/****************************************
*           API Constants
****************************************/

#define eeprom_EEPROM_SIZE            CYDEV_EE_SIZE
#define eeprom_SPC_BYTE_WRITE_SIZE    (0x01u)

#define eeprom_SECTORS_NUMBER         (CYDEV_EE_SIZE / CYDEV_EEPROM_SECTOR_SIZE)

#define eeprom_AHB_REQ_SHIFT          (0x00u)
#define eeprom_AHB_REQ                ((uint8)(0x01u << eeprom_AHB_REQ_SHIFT))
#define eeprom_AHB_ACK_SHIFT          (0x01u)
#define eeprom_AHB_ACK_MASK           ((uint8)(0x01u << eeprom_AHB_ACK_SHIFT))


/***************************************
* Registers
***************************************/
#define eeprom_SPC_EE_SCR_REG                 (*(reg8 *) CYREG_SPC_EE_SCR)
#define eeprom_SPC_EE_SCR_PTR                 ( (reg8 *) CYREG_SPC_EE_SCR)



/***************************************
* The following code is DEPRECATED and
* should not be used in new projects.
***************************************/
#define eeprom_ByteWrite                  eeprom_ByteWritePos
#define eeprom_QueryWrite                 eeprom_Query

#endif /* CY_EEPROM_eeprom_H */

/* [] END OF FILE */
