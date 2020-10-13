/******************************************************************************
* File Name:   xip_blinky_app.cpp
*
* Description:
*   This file contains source code for LED blinky application which executes 
*   from external flash. Execute-In-Place (XIP) mode is enabled by the 
*   Mbed OS initialization code (mbed_sdk_init). This entire source file (object)
*   is placed in external memory with instructions given in the linker scripts.
*   For details, see the linker script files located at:
*   (/mbed-os-example-xip/COMPONENT_CUSTOM_CM4_LINKER/TARGET_XXX)
*
* Related Document: See Readme.md
*
*
*******************************************************************************
* (c) (2019-2020), Cypress Semiconductor Corporation. All rights reserved.
*******************************************************************************
* This software, including source code, documentation and related materials
* ("Software"), is owned by Cypress Semiconductor Corporation or one of its
* subsidiaries ("Cypress") and is protected by and subject to worldwide patent
* protection (United States and foreign), United States copyright laws and
* international treaty provisions. Therefore, you may use this Software only
* as provided in the license agreement accompanying the software package from
* which you obtained this Software ("EULA").
*
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software source
* code solely for use in connection with Cypress's integrated circuit products.
* Any reproduction, modification, translation, compilation, or representation
* of this Software except as specified above is prohibited without the express
* written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Cypress's product in a High Risk Product, the manufacturer of such
* system or application assumes all risk of such use and in doing so agrees to
* indemnify Cypress against all liability.
*******************************************************************************/

/*******************************************************************************
* Header files including
********************************************************************************/
#include "mbed.h"
#include "xip_blinky_app.h"

/*******************************************************************************
* Global constants
********************************************************************************/
/* LED blinking time period in milli seconds. */
#define  LED_BLINK_PERIOD_4_HZ         250ms
#define  LED_BLINK_PERIOD_1_HZ         1000ms

/*******************************************************************************
* Function Prototypes
********************************************************************************/
static void callback_lp_ticker(void);
static void isr_user_button(void);

/*******************************************************************************
* Global variables
*******************************************************************************/
static DigitalOut user_led(LED1);
static InterruptIn user_button(BUTTON1);
static LowPowerTicker led_ticker;

/* Variable to indicate LED blink rate: true - 4 Hz rate, false - 1 Hz rate */
volatile bool is_led_blink_rate_4hz, is_prev_led_blink_rate_4hz;

/*******************************************************************************
* Function Name: led_blink_external_memory
********************************************************************************
* Summary: This function blinks an LED either at 1 Hz or 4 Hz. The LED blinking 
* rate is changed on every user button press. 
*
* Parameters:
*  none
*
* Return:
*  none
*******************************************************************************/
void led_blink_external_memory(void)
{
    printf("============================================================"
           "=============\r\n");
    printf("Executing from external flash: LED is blinking at 1 Hz\r\n" );
    
    #ifdef ENABLE_INTERNAL_PULL_UP_RESISTOR
    /* For CY8CKIT-062-BLE target, external pull up resistor for BUTTON1 is
     * not loaded by default. Enable internal pull up resistor by setting the
     * drive mode of user button to pull up. This macro is enabled through
     * mbed_app.json file.
     */    
    user_button.mode(PullUp);
    #endif
    
    /* Register call back function on rising edge of user button (BUTTON1)
     * interrupt.
     */
    user_button.fall(&isr_user_button);
    
    led_ticker.attach(&callback_lp_ticker, LED_BLINK_PERIOD_1_HZ/2);
    
    while(true)
    {
        if(is_led_blink_rate_4hz != is_prev_led_blink_rate_4hz)
        {
            printf("============================================================"
                "=============\r\n");
            if(is_led_blink_rate_4hz)
            {
                printf("Executing from external flash: LED is blinking at 4 Hz\r\n" );
                led_ticker.attach(&callback_lp_ticker, LED_BLINK_PERIOD_4_HZ/2);
            }
            else
            {
                printf("Executing from external flash: LED is blinking at 1 Hz\r\n" ); 
                led_ticker.attach(&callback_lp_ticker, LED_BLINK_PERIOD_1_HZ/2);
            }
            
            is_prev_led_blink_rate_4hz = is_led_blink_rate_4hz;  
        }
        
        sleep();
    }    
}

/*******************************************************************************
* Function Name: isr_user_button
********************************************************************************
* Summary:
*  User button interrupt handler. This function is used to change the LED blink
*  rate whenver user button is pressed.
*
* Parameters:
*  none
*
* Return:
*  none
*******************************************************************************/
static void isr_user_button(void)
{
    if (is_led_blink_rate_4hz)
    {
        is_led_blink_rate_4hz = false;                   
    }
    else
    {
        is_led_blink_rate_4hz = true;       
    }        
}

/*******************************************************************************
* Function Name: callback_lp_ticker
********************************************************************************
* Summary:
*  Call back function that is invoked when the low power ticker expires.
*
* Parameters:
*  none
*
* Return:
*  none
*******************************************************************************/
static void callback_lp_ticker(void)
{
     user_led = !user_led;  
}

/* [] END OF FILE */
