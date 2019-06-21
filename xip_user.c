/******************************************************************************
* File Name: xip_user.c
*
* Description:
* Provides functions for configuring SMIF for XIP mode of operation.
*  
*******************************************************************************
* Copyright 2017-2019 Cypress Semiconductor Corporation
* SPDX-License-Identifier: Apache-2.0
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
******************************************************************************/
#include "cy_smif.h"
#include "cy_smif_memslot.h"
#include "cycfg_qspi_memslot.h"
#include "qspi_api.h"

#define QSPI_FREQUENCY		(50000000UL)
/*****************************************************************************
* NULL terminated array of SMIF structures for use in TOC2
******************************************************************************/
typedef struct
{
 const cy_stc_smif_block_config_t * smifCfg; /* Pointer to SMIF top-level configuration */
 const uint32_t null_t; /* NULL termination */
} stc_smif_ipblocks_arr_t;

/* Function prototypes */
cy_en_smif_status_t EnableQuadMode(const cy_stc_smif_mem_config_t *memConfig);

/* 
* PSoC 6 boot sequence is such that the Table of Contents (TOC2) needs to be verifed
* before switching to the application code. In the mbed build system the CRC for
* TOC2 is not calculated. Hence CRC has to be manually placed in the TOC2 SMIF 
* configuration structure is placed at a fixed address so as to fix the CRC for TOC2.
* For more details, refer to PSoC 6 Programming Specification:
* https://www.cypress.com/documentation/programming-specifications/psoc-6-programming-specifications
*/
__attribute__((section(".cy_sflash_user_data"))) __attribute__((used))
const stc_smif_ipblocks_arr_t smifIpBlocksArr = {&smifBlockConfig, 0x00000000};

/******************************************************************************
* Point to the SMIF block structure in the table of contents2 (TOC2).
* 
* This enables memory reads using Cypress Programmer, without this
* structure, external memory access from Cypress Programmer will not 
* work 
*******************************************************************************/

__attribute__((section(".cy_toc_part2"))) __attribute__((used))
const int cyToc[128] =
{
   /* Offset=0x00: Object Size, bytes */ 
   0x200-4,   
   /* Offset=0x04: Magic Number (TOC Part 2, ID) */ 	   
   0x01211220,
   /* Offset=0x08: Key Storage Address */    
   0,
   /* Offset=0x0C: This points to a null terminated array of SMIF structures. */   
   (int)&smifIpBlocksArr,
   /* Offset=0x10: App image start address */
   0x10000000u,
   /* Offset=0x1FC: CRC16-CCITT (the upper 2 bytes contain the CRC and the 
      lower 2 bytes are 0) */   
   [127] =  0x0B1F0000       
  };

/* QSPI HAL object */
static qspi_t QSPI_OBJ; 

/******************************************************************************
* Function Name: qspi_start
*******************************************************************************
*  Summary:
*  This function sets the QE (QUAD Enable) bit in the external memory
*  configuration register to enable Quad SPI mode
*
*  Return:
*  Status of the operation. See qspi_status_t. 
*
******************************************************************************/
qspi_status_t qspi_start(void)
{
	QSPI_OBJ.base = SMIF0;
    QSPI_OBJ.slaveSelect = smifMemConfigs[0]->slaveSelect; 
    qspi_status_t qspi_api_result = QSPI_STATUS_OK;
	cy_en_smif_status_t status = CY_SMIF_SUCCESS;

    /* Initialize the QSPI interface */
    qspi_api_result = qspi_init(&QSPI_OBJ, QSPI_IO_0, QSPI_IO_1, QSPI_IO_2, QSPI_IO_3, QSPI_CLK, QSPI_SEL, QSPI_FREQUENCY, 0);
    
    if(qspi_api_result == QSPI_STATUS_OK)
    {
        /* Initialize the memory device connected to SMIF slot */
        status = Cy_SMIF_Memslot_Init(QSPI_OBJ.base, (cy_stc_smif_block_config_t*)&smifBlockConfig, &(QSPI_OBJ.context));
		
		if(status == CY_SMIF_SUCCESS)
		{
			/* Enable quad mode of operation */
			status = EnableQuadMode((const cy_stc_smif_mem_config_t*)smifMemConfigs[0]);
			if(status == CY_SMIF_SUCCESS)
			{
				/* Set the operation mode to XIP */
				Cy_SMIF_SetMode(QSPI_OBJ.base, CY_SMIF_MEMORY);
	        }
		}
		
		/* Remap the cy_en_smif_status_t to qspi_status_t */
		if(status == CY_SMIF_SUCCESS)
		{
			qspi_api_result = QSPI_STATUS_OK;
		}
		else
		{
			qspi_api_result = QSPI_STATUS_ERROR;
		}
	}

    return qspi_api_result;    
}

/******************************************************************************
* Function Name: EnableQuadMode
*******************************************************************************
*  Summary:
*  This function sets the QE (QUAD Enable) bit in the external memory
*  configuration register to enable Quad SPI mode
*
*  Parameters:
*  memConfig - pointer to memory configuration structure
*  
*  Return:
*  Status of the operation. See cy_en_smif_status_t. 
*
******************************************************************************/
cy_en_smif_status_t EnableQuadMode(const cy_stc_smif_mem_config_t *memConfig)
{
	cy_en_smif_status_t status;

	/* Send Write Enable to external memory */
	status = Cy_SMIF_Memslot_CmdWriteEnable(QSPI_OBJ.base, memConfig,
        &QSPI_OBJ.context);

	if(CY_SMIF_SUCCESS == status)
	{
		status = Cy_SMIF_Memslot_QuadEnable(QSPI_OBJ.base,
            (cy_stc_smif_mem_config_t* )memConfig, &(QSPI_OBJ.context));
	}
	return status;
}
