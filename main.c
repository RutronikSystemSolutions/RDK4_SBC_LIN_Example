/******************************************************************************
* File Name:   main.c
*
* Description: This is the source code for the RDK4 SBC LIN
*              Application for ModusToolbox.
*
* Related Document: See README.md
*
*
*  Created on: 2023-03-14
*  Company: Rutronik Elektronische Bauelemente GmbH
*  Address: Jonavos g. 30, Kaunas 44262, Lithuania
*  Author: GDR
*
*******************************************************************************
* (c) 2019-2021, Cypress Semiconductor Corporation. All rights reserved.
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
*
* Rutronik Elektronische Bauelemente GmbH Disclaimer: The evaluation board
* including the software is for testing purposes only and,
* because it has limited functions and limited resilience, is not suitable
* for permanent use under real conditions. If the evaluation board is
* nevertheless used under real conditions, this is done at one’s responsibility;
* any liability of Rutronik is insofar excluded
*******************************************************************************/

#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"
#include "TLE926x.h"
#include "mtbcfg_lin.h"

/*Priority for SBC interrupts*/
#define SBC_IRQ_PRIORITY		(1U)
/* Instance number */
#define LIN_IFC_HANDLE          (0U)
/* Interrupt priority for SCB ISR */
#define LIN_SCB_INT_PRIORITY    (2U)

/* Define start position and number of bytes for input & output signals */
/* One command byte received from Master */
#define LIN_SIGNALINPUT_START_BYTE      (0u)
#define LIN_SIGNALINPUT_NUM_OF_BYTES    (1u)

/* 2 status bytes from Slave to Master - previous command and LED status */
#define LIN_SIGNALOUTPUT_START_BYTE     (0u)
#define LIN_SIGNALOUTPUT_NUM_OF_BYTES   (2u)

/* Implement TLE9262 SBC Interrupt*/
void sbc_interrupt_handler(void *handler_arg, cyhal_gpio_event_t event);
/* Implement SCB ISR for LIN */
static void LIN_Isr(void);
/* Implement Inactivity ISR for LIN */
static void LIN_InactivityIsr(void);
/*LIN Initialization Function Prototype*/
void LIN_start(void);

/*TLE9262 SBC Interrupt Data*/
cyhal_gpio_callback_data_t sbc_int_data =
{
		.callback = sbc_interrupt_handler,
		.callback_arg = NULL,
};

/*SBC Power Supply Variables*/
sbc_vcc3_on_t vcc3_supp;
sbc_vcc2_on_t vcc2_supp;

/* Allocate context for LIN operation */
mtb_stc_lin_context_t lin_context;

int main(void)
{
	SBC_ErrorCode sbc_err;
    cy_rslt_t result;

    /* Local variables */
    uint8_t dataReceived = 0;
    uint8_t dataArray[2] = {0};

    /* Initialize the device and board peripherals */
    result = cybsp_init() ;
    if (result != CY_RSLT_SUCCESS)
    {
    	CY_ASSERT(0);
    }

    /*Initialize LEDs*/
    result = cyhal_gpio_init( USER_LED_GREEN, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, CYBSP_LED_STATE_ON);
    if (result != CY_RSLT_SUCCESS)
    {CY_ASSERT(0);}
    result = cyhal_gpio_init( USER_LED_RED, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, CYBSP_LED_STATE_OFF);
    if (result != CY_RSLT_SUCCESS)
    {CY_ASSERT(0);}
    result = cyhal_gpio_init( USER_LED_BLUE, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, CYBSP_LED_STATE_OFF);
    if (result != CY_RSLT_SUCCESS)
    {CY_ASSERT(0);}

    __enable_irq();

    /*Initialize SBC Interrupt Pin*/
    result = cyhal_gpio_init(INT_SBC, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_NONE, false);
    if (result != CY_RSLT_SUCCESS)
    {CY_ASSERT(0);}
    /*Register callback functions */
    cyhal_gpio_register_callback(INT_SBC, &sbc_int_data);
    /* Enable rising edge interrupt events */
    cyhal_gpio_enable_event(INT_SBC, CYHAL_GPIO_IRQ_RISE, SBC_IRQ_PRIORITY, true);

    /*SBC Initializations*/
    sbc_err = sbc_init();
    if(sbc_err.flippedBitsMask)
    {
    	CY_ASSERT(0);
    }

    /*Turn ON the 5V Power Supply VCC2 for the Display */
    vcc2_supp = VCC2_ON_ALWAYS;
    sbc_err = sbc_switch_vcc2(vcc2_supp);
    if(sbc_err.flippedBitsMask)
    {
    	CY_ASSERT(0);
    }

    /*Turn ON the 3.3V Power Supply VCC3 for the Arduino Shield(s) */
    vcc3_supp = VCC3_ENABLED;
    sbc_err = sbc_switch_vcc3(vcc3_supp);
    if(sbc_err.flippedBitsMask)
    {
    	CY_ASSERT(0);
    }

    sbc_err = sbc_lin_mode(LIN1, LIN_NORMAL_MODE);
    if(sbc_err.flippedBitsMask)
    {
    	CY_ASSERT(0);
    }

    /*SBC Watchdog Configuration*/
    sbc_err = sbc_configure_watchdog(TIME_OUT_WD, NO_WD_AFTER_CAN_LIN_WAKE, WD_1000MS);
    if(sbc_err.flippedBitsMask)
    {
    	CY_ASSERT(0);
    }

    /*Initialize LIN Stack*/
    LIN_start();

    for (;;)
    {
     	/*Feed the watchdog*/
    	sbc_wd_trigger();

        /***********************************************************************
         * Check if "InFrame" frame is received from LIN Master
         **********************************************************************/
        if (true == l_flg_tst(MTB_LIN_0_FLAG_HANDLE_InFrame, &lin_context))
        {
            /* Read the 1st byte command received from the LIN Master */
            l_bytes_rd(MTB_LIN_0_SIGNAL_HANDLE_SignalInput, \
                    LIN_SIGNALINPUT_START_BYTE, \
                    LIN_SIGNALINPUT_NUM_OF_BYTES, \
                    &dataReceived, &lin_context);

            /* Clear frame flag */
            l_flg_clr(MTB_LIN_0_FLAG_HANDLE_InFrame, &lin_context);

            /* Store the received command in dataArray */
            dataArray[0] = dataReceived;

            /* Send the previous command and the status of LEDs to LIN Master */
            l_bytes_wr(MTB_LIN_0_SIGNAL_HANDLE_SignalOutput, \
                    LIN_SIGNALOUTPUT_START_BYTE, \
                    LIN_SIGNALOUTPUT_NUM_OF_BYTES, \
                    dataArray, &lin_context);
        }

        /***********************************************************************
         * Check if the data in "OutFrame" frame is sent to LIN Master
         **********************************************************************/
        if (true == l_flg_tst(MTB_LIN_0_FLAG_HANDLE_OutFrame, &lin_context))
        {
            /* Clear frame flag */
            l_flg_clr(MTB_LIN_0_FLAG_HANDLE_OutFrame, &lin_context);
        }

        /* Switch the LEDs */
        switch(dataArray[0])
        {
        case 0x00: /*All LEDs OFF*/
        	cyhal_gpio_write(USER_LED_RED, CYBSP_LED_STATE_OFF);
        	cyhal_gpio_write(USER_LED_GREEN, CYBSP_LED_STATE_OFF);
        	cyhal_gpio_write(USER_LED_BLUE, CYBSP_LED_STATE_OFF);
        	break;
        case 0x11:  /*RED LED ON*/
        	cyhal_gpio_write(USER_LED_RED, CYBSP_LED_STATE_ON);
        	cyhal_gpio_write(USER_LED_GREEN, CYBSP_LED_STATE_OFF);
        	cyhal_gpio_write(USER_LED_BLUE, CYBSP_LED_STATE_OFF);
        	break;
        case 0x22: /*GREEN LED ON*/
        	cyhal_gpio_write(USER_LED_RED, CYBSP_LED_STATE_OFF);
        	cyhal_gpio_write(USER_LED_GREEN, CYBSP_LED_STATE_ON);
        	cyhal_gpio_write(USER_LED_BLUE, CYBSP_LED_STATE_OFF);
        	break;
        case 0x33: /*BLUE LED ON*/
        	cyhal_gpio_write(USER_LED_RED, CYBSP_LED_STATE_OFF);
        	cyhal_gpio_write(USER_LED_GREEN, CYBSP_LED_STATE_OFF);
        	cyhal_gpio_write(USER_LED_BLUE, CYBSP_LED_STATE_ON);
        	break;
        default: /*All LEDs OFF*/
        	cyhal_gpio_write(USER_LED_RED, CYBSP_LED_STATE_OFF);
        	cyhal_gpio_write(USER_LED_GREEN, CYBSP_LED_STATE_ON);
        	cyhal_gpio_write(USER_LED_BLUE, CYBSP_LED_STATE_OFF);
        }
    }
}

/* Implement SCB ISR for LIN */
static void LIN_Isr(void)
{
    l_ifc_rx(LIN_IFC_HANDLE, &lin_context);
}
/* Implement Inactivity ISR for LIN */
static void LIN_InactivityIsr(void)
{
    l_ifc_aux(LIN_IFC_HANDLE, &lin_context);
}

/*******************************************************************************
* Function Name: LIN_start
****************************************************************************/
void LIN_start(void)
{
    /* Initializes the LIN core that is specified by the context structure */
    if (l_sys_init(&mtb_lin_0_config, &lin_context, &LIN_Isr, LIN_SCB_INT_PRIORITY, &LIN_InactivityIsr) != MTB_LIN_STATUS_SUCCESS)
    {
        /* Stop program execution if sys init failed */
        CY_ASSERT(0U);
    }
    /* Initializes the LIN instance that is specified by the context structure.
     * Choose appropriate pins for tx and rx direction from HAL library. */
    if (l_ifc_init(LIN_IFC_HANDLE, &lin_context, KITPROG_TX, KITPROG_RX))
    {
        /* Stop program execution if ifc init failed */
        CY_ASSERT(0U);
    }
}

/* Interrupt handler callback function */
void btn_interrupt_handler(void *handler_arg, cyhal_gpio_event_t event)
{
	CY_UNUSED_PARAMETER(handler_arg);
    CY_UNUSED_PARAMETER(event);
}

/* SBC Interrupt handler callback function */
void sbc_interrupt_handler(void *handler_arg, cyhal_gpio_event_t event)
{
	CY_UNUSED_PARAMETER(handler_arg);
    CY_UNUSED_PARAMETER(event);

    SBC_ISR();
}

/* [] END OF FILE */
