/* Copyright Statement:
 *
 * (C) 2005-2016  MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. ("MediaTek") and/or its licensors.
 * Without the prior written permission of MediaTek and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 * You may only use, reproduce, modify, or distribute (as applicable) MediaTek Software
 * if you have agreed to and been bound by the applicable license agreement with
 * MediaTek ("License Agreement") and been granted explicit permission to do so within
 * the License Agreement ("Permitted User").  If you are not a Permitted User,
 * please cease any access or use of MediaTek Software immediately.
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT MEDIATEK SOFTWARE RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES
 * ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 */

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* hal includes */
#include "hal.h"
#include "system_mt7687.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

#define eSHT2xAddress       0x40
#define eTempNoHoldCmd      0xF3
#define eRHumidityNoHoldCmd 0xF5


/* Private functions ---------------------------------------------------------*/
static void SystemClock_Config(void);
static void prvSetupHardware( void );
    hal_i2c_config_t i2c_init;
    hal_i2c_frequency_t input_frequency = HAL_I2C_FREQUENCY_100K;
    hal_i2c_port_t i2c_port = HAL_I2C_MASTER_0;

#ifdef __GNUC__
int __io_putchar(int ch)
#else
int fputc(int ch, FILE *f)
#endif
{
    /* Place your implementation of fputc here */ 
    /* E.g. write a character to the HAL_UART_0 one at a time */
    hal_uart_put_char(HAL_UART_0, ch);
    return ch;
}

/* UART hardware initialization for log output */
static void plain_log_uart_init(void)
{
    hal_uart_config_t uart_config;
    /* Set Pinmux to UART */
    hal_pinmux_set_function(HAL_GPIO_0, HAL_GPIO_0_UART1_RTS_CM4);
    hal_pinmux_set_function(HAL_GPIO_1, HAL_GPIO_1_UART1_CTS_CM4);
    hal_pinmux_set_function(HAL_GPIO_2, HAL_GPIO_2_UART1_RX_CM4);
    hal_pinmux_set_function(HAL_GPIO_3, HAL_GPIO_3_UART1_TX_CM4);

    /* COM port settings */
    uart_config.baudrate = HAL_UART_BAUDRATE_115200;
    uart_config.word_length = HAL_UART_WORD_LENGTH_8;
    uart_config.stop_bit = HAL_UART_STOP_BIT_1;
    uart_config.parity = HAL_UART_PARITY_NONE;
    hal_uart_init(HAL_UART_0, &uart_config);
}

static void SystemClock_Config(void)
{
    top_xtal_init();
}

static void prvSetupHardware( void )
{
    /* Peripherals initialization */
    plain_log_uart_init();

}
static void i2cinit(){
    /* I2C_1 initialization */
    hal_gpio_init(HAL_GPIO_27);
    hal_gpio_init(HAL_GPIO_28);
    /* Call hal_pinmux_set_function() to set GPIO pinmux, if EPT tool is not used to configure the related pinmux */
    hal_pinmux_set_function(HAL_GPIO_27,HAL_GPIO_27_I2C1_CLK);
    hal_pinmux_set_function(HAL_GPIO_28,HAL_GPIO_28_I2C1_DATA);
    /* Initialize I2C */
    i2c_init.frequency = input_frequency;
    hal_i2c_master_init(i2c_port,&i2c_init);
    hal_gpt_delay_ms(200);
}

uint16_t readSensor(uint8_t command)
{
    uint16_t result;

    hal_i2c_master_send_polling(i2c_port,eSHT2xAddress,&command,1);
    /*Wait some time till the conversion is done*/
    hal_gpt_delay_ms(100);
    /* Read the data back */
    uint8_t i2c_receive_data[3] = {0};
    hal_i2c_master_receive_polling(i2c_port,eSHT2xAddress,i2c_receive_data,3);

    //Store the result
    result = ((uint16_t)i2c_receive_data[0]<< 8);
    result += i2c_receive_data[1];
    result &= ~0x0003;   // clear two low bits (status bits)

    //log_hal_info("[I2C Result]=>%d\r\n",result);
    return result;
}

double GetHumidity()
{
    return (-6.0f + 125.0f / 65536.0f * (double)readSensor(eRHumidityNoHoldCmd));
}

double GetTemperature()
{
    return (-46.85f + 175.72f / 65536.0f * (double)readSensor(eTempNoHoldCmd));
}



int main(void)
{
    /* Configure system clock */
    SystemClock_Config();

    /* Configure the hardware */
    prvSetupHardware();
    i2cinit();

    /* Enable I,F bits */
    __enable_irq();
    __enable_fault_irq();

    log_hal_info("\r\n\r\n");/* The output UART used by log_hal_info is set by plain_log_uart_init() */
    log_hal_info("HTU21D Example\r\n");
    log_hal_info("\r\n\r\n");

    while(1){
        int temperature = (int)(GetTemperature()*100);
        int humidity = (int)(GetHumidity()*100);
        printf("Temperature:%d.%d\tHumidity:%d.%d\r\n",temperature/100,temperature%100,humidity/100,humidity%100);
        hal_gpt_delay_ms(1000); 
    }
}



