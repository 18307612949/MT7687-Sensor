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
const unsigned char OSS = 0;  // Oversampling Setting

// Calibration values
int16_t ac1;
int16_t ac2;
int16_t ac3;
uint16_t ac4;
uint16_t ac5;
uint16_t ac6;
int16_t b1;
int16_t b2;
int16_t mb;
int16_t mc;
int16_t md;

// b5 is calculated in bmp085GetTemperature(...), this variable is also used in bmp085GetPressure(...)
// so ...Temperature(...) must be called before ...Pressure(...).
int32_t b5; 
/* Private macro -------------------------------------------------------------*/
#define BMP085_ADDRESS 0x77


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
// Read 1 byte from the BMP085 at 'address'
uint8_t bmp085Read(unsigned char address)
{
    uint8_t data;

    hal_i2c_master_send_polling(i2c_port,BMP085_ADDRESS,&address,1);
    /* Read the data back */
    hal_i2c_master_receive_polling(i2c_port,BMP085_ADDRESS,&data,1);
    //log_hal_info("[I2C Result]=>%d\r\n",result);
    return data;
}

// Read 2 bytes from the BMP085
// First byte will be from 'address'
// Second byte will be from 'address'+1
uint16_t bmp085ReadInt(unsigned char address)
{
    uint16_t result;

    hal_i2c_master_send_polling(i2c_port,BMP085_ADDRESS,&address,1);
    /* Read the data back */
    uint8_t i2c_receive_data[2] = {0};
    hal_i2c_master_receive_polling(i2c_port,BMP085_ADDRESS,i2c_receive_data,2);

    //Store the result
    result = ((uint16_t)i2c_receive_data[0]<< 8);
    result += i2c_receive_data[1];
    return result;

}

// Read the uncompensated temperature value
uint16_t bmp085ReadUT(){
  uint16_t ut;

  // Write 0x2E into Register 0xF4
  // This requests a temperature reading
  uint8_t cmd[2] = {0xF4,0x2E};
  hal_i2c_master_send_polling(i2c_port,BMP085_ADDRESS,cmd,2);
  // Wait at least 4.5ms
  hal_gpt_delay_ms(5);

  // Read two bytes from registers 0xF6 and 0xF7
  ut = bmp085ReadInt(0xF6);
  return ut;
}

// Read the uncompensated pressure value
uint32_t bmp085ReadUP(){

  uint8_t msb, lsb, xlsb;
  uint32_t up = 0;

  // Write 0x34+(OSS<<6) into register 0xF4
  // Request a pressure reading w/ oversampling setting

  uint8_t cmd[2] = {0xF4,0x2E};
  cmd[1] = 0x34 + (OSS<<6);
  hal_i2c_master_send_polling(i2c_port,BMP085_ADDRESS,cmd,2);
  // Wait for conversion, delay time dependent on OSS
  hal_gpt_delay_ms(2 + (3<<OSS));

  // Read register 0xF6 (MSB), 0xF7 (LSB), and 0xF8 (XLSB)
  msb = bmp085Read(0xF6);
  lsb = bmp085Read(0xF7);
  xlsb = bmp085Read(0xF8);

  up = (((uint32_t) msb << 16) | ((uint32_t) lsb << 8) | (uint32_t) xlsb) >> (8-OSS);

  return up;
}
void bmp085Calibration()
{
  ac1 = bmp085ReadInt(0xAA);
  ac2 = bmp085ReadInt(0xAC);
  ac3 = bmp085ReadInt(0xAE);
  ac4 = bmp085ReadInt(0xB0);
  ac5 = bmp085ReadInt(0xB2);
  ac6 = bmp085ReadInt(0xB4);
  b1 = bmp085ReadInt(0xB6);
  b2 = bmp085ReadInt(0xB8);
  mb = bmp085ReadInt(0xBA);
  mc = bmp085ReadInt(0xBC);
  md = bmp085ReadInt(0xBE);
}

// Calculate temperature in deg C
float bmp085GetTemperature(uint16_t ut){
  int32_t x1, x2;

  x1 = (((int32_t)ut - (int32_t)ac6)*(int32_t)ac5) >> 15;
  x2 = ((int32_t)mc << 11)/(x1 + md);
  b5 = x1 + x2;

  float temp = ((b5 + 8)>>4);
  temp = temp /10;

  return temp;
}

// Calculate pressure given up
// calibration values must be known
// b5 is also required so bmp085GetTemperature(...) must be called first.
// Value returned will be pressure in units of Pa.
int32_t bmp085GetPressure(uint32_t up){
  int32_t x1, x2, x3, b3, b6, p;
  uint32_t b4, b7;

  b6 = b5 - 4000;
  // Calculate B3
  x1 = (b2 * (b6 * b6)>>12)>>11;
  x2 = (ac2 * b6)>>11;
  x3 = x1 + x2;
  b3 = (((((int32_t)ac1)*4 + x3)<<OSS) + 2)>>2;

  // Calculate B4
  x1 = (ac3 * b6)>>13;
  x2 = (b1 * ((b6 * b6)>>12))>>16;
  x3 = ((x1 + x2) + 2)>>2;
  b4 = (ac4 * (uint32_t)(x3 + 32768))>>15;

  b7 = ((int32_t)(up - b3) * (50000>>OSS));
  if (b7 < 0x80000000)
    p = (b7<<1)/b4;
  else
    p = (b7/b4)<<1;

  x1 = (p>>8) * (p>>8);
  x1 = (x1 * 3038)>>16;
  x2 = (-7357 * p)>>16;
  p += (x1 + x2 + 3791)>>4;

  int32_t temp = p;
  return temp;
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
    log_hal_info("BMP180 Example\r\n");
    log_hal_info("\r\n\r\n");
    bmp085Calibration();
    while(1){
        float temperature = bmp085GetTemperature(bmp085ReadUT()); //MUST be called first
        float pressure = bmp085GetPressure(bmp085ReadUP());
        float atm = pressure / 101325; // "standard atmosphere"

        printf("Temperature:%f\r\npressure:%f\r\natm:%f\r\n",temperature,pressure,atm);
        hal_gpt_delay_ms(1000); 
    }
}



