/* DHT library

MIT license
written by Adafruit Industries
*/

#include "DHT.h"


#define MIN_INTERVAL 2000

extern "C" {
  extern void delay_time(uint32_t count);
}

DHT::DHT(){

}

DHT::DHT(hal_gpio_pin_t pin, uint8_t type) {
  _pin = pin;
  _type = type;

  _maxcycles = 1000;  // 1 millisecond timeout for
                                                 // reading pulses from DHT sensor.
  // Note that count is now ignored as the DHT reading algorithm adjusts itself
  // basd on the speed of the processor.
}

void DHT::begin(void) {
  // set up the pins!

  hal_gpio_init(_pin);
  hal_pinmux_set_function(_pin, 8);
  hal_gpio_set_direction(_pin,HAL_GPIO_DIRECTION_INPUT);
  hal_gpio_pull_up(_pin);




  // Using this value makes sure that millis() - lastreadtime will be
  // >= MIN_INTERVAL right away. Note that this assignment wraps around,
  // but so will the subtraction.
  DEBUG_PRINT("Max clock cycles: "); DEBUG_PRINTLN("%d\r\n",_maxcycles);
}

//uint8_tean S == Scale.  True == Fahrenheit; False == Celcius
float DHT::readTemperature(uint8_t S, uint8_t force) {
  float f = 0.0;
  DEBUG_PRINT("readTemperature"); 
  
  if (read(force)) {
    switch (_type) {
    case DHT11:
      f = data[2];
      if(S) {
        f = convertCtoF(f);
      }
      break;
    case DHT22:
    case DHT21:
      f = data[2] & 0x7F;
      f *= 256;
      f += data[3];
      f *= 0.1;
      if (data[2] & 0x80) {
        f *= -1;
      }
      if(S) {
        f = convertCtoF(f);
      }
      break;
    }
  }
  return f;
}

float DHT::convertCtoF(float c) {
  return c * 1.8 + 32;
}

float DHT::convertFtoC(float f) {
  return (f - 32) * 0.55555;
}

float DHT::readHumidity(uint8_t force) {
  float f = 0.0;
  DEBUG_PRINT("readHumidity"); 
  if (read()) {
    switch (_type) {
    case DHT11:
      f = data[0];
      break;
    case DHT22:
    case DHT21:
      f = data[0];
      f *= 256;
      f += data[1];
      f *= 0.1;
      break;
    }
  }
  return f;
}

//uint8_tean isFahrenheit: True == Fahrenheit; False == Celcius
float DHT::computeHeatIndex(float temperature, float percentHumidity, uint8_t isFahrenheit) {
  // Using both Rothfusz and Steadman's equations
  // http://www.wpc.ncep.noaa.gov/html/heatindex_equation.shtml
  float hi;

  if (!isFahrenheit)
    temperature = convertCtoF(temperature);

  hi = 0.5 * (temperature + 61.0 + ((temperature - 68.0) * 1.2) + (percentHumidity * 0.094));

  if (hi > 79) {
    hi = -42.379 +
             2.04901523 * temperature +
            10.14333127 * percentHumidity +
            -0.22475541 * temperature*percentHumidity +
            -0.00683783 * pow(temperature, 2) +
            -0.05481717 * pow(percentHumidity, 2) +
             0.00122874 * pow(temperature, 2) * percentHumidity +
             0.00085282 * temperature*pow(percentHumidity, 2) +
            -0.00000199 * pow(temperature, 2) * pow(percentHumidity, 2);

    if((percentHumidity < 13) && (temperature >= 80.0) && (temperature <= 112.0))
      hi -= ((13.0 - percentHumidity) * 0.25) * sqrt((17.0 - abs(temperature - 95.0)) * 0.05882);

    else if((percentHumidity > 85.0) && (temperature >= 80.0) && (temperature <= 87.0))
      hi += ((percentHumidity - 85.0) * 0.1) * ((87.0 - temperature) * 0.2);
  }

  return isFahrenheit ? hi : convertFtoC(hi);
}

uint8_t DHT::read(uint8_t force) {

    hal_rtc_status_t ret;
    hal_rtc_time_t time;
    ret = hal_rtc_init();
    if(HAL_RTC_STATUS_OK != ret) {
        //error handling
    }
    ret = hal_rtc_get_time(&time);
    if(HAL_RTC_STATUS_OK != ret) {
        //error handling
    }
    //The user has to define the base year and the RTC year is defined
    //as an offset. For example, define the base year to 2000 and assign 15 to RTC year to represent the year of 2015.
  
  uint32_t currenttime = time.rtc_sec;
  if (!force && ((currenttime - _lastreadtime) < 2)) {
    return _lastresult; // return last correct measurement
  }
  _lastreadtime = currenttime;

  // Reset 40 bits of received data to zero.
  data[0] = data[1] = data[2] = data[3] = data[4] = 0;

  // Send start signal.  See DHT datasheet for full signal diagram:
  //   http://www.adafruit.com/datasheets/Digital%20humidity%20and%20temperature%20sensor%20AM2302.pdf

  // Go into high impedence state to let pull-up raise data line level and
  // start the reading process.

  hal_gpio_set_direction(_pin,HAL_GPIO_DIRECTION_INPUT);

  hal_gpt_delay_ms(250);

  // First set data line low for 20 milliseconds.
  hal_gpio_set_direction(_pin,HAL_GPIO_DIRECTION_OUTPUT);
  hal_gpio_set_output(_pin, HAL_GPIO_DATA_LOW);
  hal_gpt_delay_ms(20);
  hal_gpio_set_direction(_pin,HAL_GPIO_DIRECTION_INPUT);
/*
  while(1){
    Fast_gpio_write(HAL_GPIO_35,1);
    Fast_gpio_read(_pin);
    Fast_gpio_write(HAL_GPIO_35,0);
    Fast_gpio_read(_pin);
  }
  */
  uint32_t cycles[80];
  {
    // Turn off interrupts temporarily because the next sections are timing critical
    // and we don't want any interruptions.
    __disable_irq();

    // End the start signal by setting data line high for 40 microseconds.
    //hal_gpio_set_output(_pin, HAL_GPIO_DATA_HIGH);
    
      Fast_gpio_write(HAL_GPIO_35,1);
    //delay_time(1); 
      Fast_gpio_write(HAL_GPIO_35,0);
    // Now start reading the data line to get the value from the DHT sensor.
    //hal_gpio_set_direction(_pin,HAL_GPIO_DIRECTION_INPUT);
    
    delay_time(1);   // Delay a bit to let sensor pull data line low.

    // First expect a low signal for ~80 microseconds followed by a high signal
    // for ~80 microseconds again.
    if (expectPulse(0) == 0) {
      DEBUG_PRINTLN("Timeout waiting for start signal low pulse.\r\n");
      _lastresult = false;
      return _lastresult;
    }
    if (expectPulse(1) == 0) {
      DEBUG_PRINTLN("Timeout waiting for start signal high pulse.\r\n");
      _lastresult = false;
      return _lastresult;
    }

    // Now read the 40 bits sent by the sensor.  Each bit is sent as a 50
    // microsecond low pulse followed by a variable length high pulse.  If the
    // high pulse is ~28 microseconds then it's a 0 and if it's ~70 microseconds
    // then it's a 1.  We measure the cycle count of the initial 50us low pulse
    // and use that to compare to the cycle count of the high pulse to determine
    // if the bit is a 0 (high state cycle count < low state cycle count), or a
    // 1 (high state cycle count > low state cycle count). Note that for speed all
    // the pulses are read into a array and then examined in a later step.
    for (int i=0; i<80; i+=2) {
      cycles[i]   = expectPulse(0);
      cycles[i+1] = expectPulse(1);
    }
    __enable_irq();
  } // Timing critical code is now complete.

  // Inspect pulses and determine which ones are 0 (high state cycle count < low
  // state cycle count), or 1 (high state cycle count > low state cycle count).
  for (int i=0; i<40; ++i) {
    uint32_t lowCycles  = cycles[2*i];
    uint32_t highCycles = cycles[2*i+1];
    if ((lowCycles == 0) || (highCycles == 0)) {
      DEBUG_PRINTLN("Timeout waiting for pulse.");
      _lastresult = false;
      return _lastresult;
    }
    data[i/8] <<= 1;
    // Now compare the low and high cycle times to see if the bit is a 0 or 1.
    if (highCycles > lowCycles) {
      // High cycles are greater than 50us low cycle count, must be a 1.
      data[i/8] |= 1;
    }
    // Else high cycles are less than (or equal to, a weird case) the 50us low
    // cycle count so this must be a zero.  Nothing needs to be changed in the
    // stored data.
  }

  DEBUG_PRINTLN("Received:");
  DEBUG_PRINT("%X",data[0]); DEBUG_PRINT(", ");
  DEBUG_PRINT("%X",data[1]); DEBUG_PRINT(", ");
  DEBUG_PRINT("%X",data[2]); DEBUG_PRINT(", ");
  DEBUG_PRINT("%X",data[3]); DEBUG_PRINT(", ");
  DEBUG_PRINT("%X",data[4]); DEBUG_PRINT(" =? ");
  DEBUG_PRINTLN("%X",(data[0] + data[1] + data[2] + data[3]) & 0xFF);

  // Check we read 40 bits and that the checksum matches.
  if (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) {
    _lastresult = true;
    return _lastresult;
  }
  else {
    DEBUG_PRINTLN("Checksum failure!");
    _lastresult = false;
    return _lastresult;
  }
}

// Expect the signal line to be at the specified level for a period of time and
// return a count of loop cycles spent at that level (this cycle count can be
// used to compare the relative time of two pulses).  If more than a millisecond
// ellapses without the level changing then the call fails with a 0 response.
// This is adapted from Arduino's pulseInLong function (which is only available
// in the very latest IDE versions):
//   https://github.com/arduino/Arduino/blob/master/hardware/arduino/avr/cores/arduino/wiring_pulse.c
uint32_t DHT::expectPulse(uint8_t level) {
  uint32_t count = 0;
  uint8_t flag = 1;

  Fast_gpio_write(HAL_GPIO_35, 1);
    while (Fast_gpio_read(_pin) == level) {
      //DEBUG_PRINTLN("PIN:%d\r\n",Fast_gpio_read(_pin));
      if (count++ >= _maxcycles) {
        Fast_gpio_write(HAL_GPIO_35, 0);
        return 0; // Exceeded timeout, fail.
      }
    }
  Fast_gpio_write(HAL_GPIO_35, 0);
  return count;
}




UINT8 DHT::Fast_gpio_read(UINT32 GPIO_pin)
{
    UINT16 no;
    UINT16 remainder;
    no = GPIO_pin / 32;
    remainder = GPIO_pin % 32;
    UINT8 din = 0;
    switch (no) {

        case 0:
            din = ((DRV_Reg32(IOT_GPIO_AON_BASE + IOT_GPIO_DIN1) >> remainder) & 1);
            break;
        case 1:
            din = ((DRV_Reg32(IOT_GPIO_AON_BASE + IOT_GPIO_DIN2) >> remainder) & 1);
            break;
        default:
            return 0;

    }
    return din;
}


INT32 DHT::Fast_gpio_write (UINT32 GPIO_pin, UINT8 writeValue)
{
    UINT16 no;
    UINT16 remainder;
    no = GPIO_pin / 32;
    remainder = GPIO_pin % 32;
    switch (no) {
        case 0:

            if (writeValue) {
                DRV_WriteReg32(IOT_GPIO_AON_BASE + IOT_GPIO_DOUT1_SET, (1 << remainder));
            } else {
                DRV_WriteReg32(IOT_GPIO_AON_BASE + IOT_GPIO_DOUT1_RESET, (1 << remainder));
            }
            break;
        case 1:

            if (writeValue) {
                DRV_WriteReg32(IOT_GPIO_AON_BASE + IOT_GPIO_DOUT2_SET, (1 << remainder));
            } else {
                DRV_WriteReg32(IOT_GPIO_AON_BASE + IOT_GPIO_DOUT2_RESET, (1 << remainder));
            }
            break;
        default:
            return -1;
    }
    return 0;
}