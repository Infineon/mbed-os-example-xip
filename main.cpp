/*******************************************************************************
 * mbed Microcontroller Library
 * Copyright (c) 2006-2019 ARM Limited
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
 *
 ******************************************************************************/
#include "mbed.h"
#include "cycfg_qspi_memslot.h"

#define PACKET_SIZE     (32u)
#define LED_OFF			(1u)

DigitalOut LED(LED1);

/* Function prototypes */

/* long_calls attribute indicates that the function is outside
   the typical addressing range  */
#pragma long_calls
void function_external_memory (uint8_t *dataBuffer, uint8_t size);
#pragma long_calls_off

void print_data(uint8_t *dataBuffer, uint8_t size);

#if defined(__cplusplus)
extern "C" 
{
#endif
extern qspi_status_t qspi_start(void);
#if defined(__cplusplus)
}
#endif

/* main() runs in its own thread in the OS */
int main()
{
    LED = LED_OFF;
    uint8_t dataBuffer[PACKET_SIZE] = {0};
    uint8_t loopcount;
    qspi_status_t qspi_api_result = QSPI_STATUS_OK;

    printf("----- Mbed OS Execute-In-Place (XIP) Example -----\r\n");

    /* Initialize the QSPI interface and set the mode to XIP */    
    qspi_api_result = qspi_start();
   
     if(qspi_api_result == QSPI_STATUS_OK)
    {
        printf("Success: QSPI in XIP mode\r\n");           
    }
    else
    {
        printf("Fail: QSPI Initialization\r\n");
        MBED_ASSERT(1 != 1);
    }

     /* Fill dataBuffer with count 0-31 */
    for(loopcount = 0; loopcount < PACKET_SIZE; loopcount++)
    {
        dataBuffer[loopcount] = loopcount;
    }

    printf("Local data before XIP manipulation:\r\nData Buffer: ");
    print_data(dataBuffer, PACKET_SIZE);

    function_external_memory(dataBuffer, PACKET_SIZE);
    
    for(;;)
    {
        LED = !LED;
		wait_ms(500);
    }
}

/******************************************************************************
*  CY_SECTION uses ARM GCC attribute "Section(NAME)" to place code in 
*  the XIP section of the ELF file.   
*
*  Summary:
*  This function executes from the external memory.
*  Function increments the local data and then prints it on the 
*  UART terminal
*
*  Parameters:
*  dataBuffer - pointer to uint8_t array
*  size - number of elements of the array
*
*  Return:
*  void 
*
******************************************************************************/
CY_SECTION(".cy_xip") __USED
void function_external_memory (uint8_t *dataBuffer, uint8_t size)
{
    uint8 loopcount;
    
    printf("Executing from external flash\r\n");
    printf("Local data after XIP manipulation:\r\nData Buffer: ");

    for(loopcount = 0; loopcount < size; loopcount++)
    {
        dataBuffer[loopcount]++;
    }

    print_data(dataBuffer, PACKET_SIZE);
}

/******************************************************************************
* Function Name: print_data
*******************************************************************************
*  Summary:
*  Function to print the data to UART terminal

*  Parameters:
*  dataBuffer - pointer to uint8_t array
*  size - number of elements of the array
*
*  Return:
*  void
*
******************************************************************************/
void print_data(uint8_t *dataBuffer, uint8_t size)
{
    uint8 loopcount;
	
    for(loopcount = 0; loopcount < size; loopcount++)
    {
        printf("%02X ", dataBuffer[loopcount]);
    }
	
    printf("\r\n");
}
