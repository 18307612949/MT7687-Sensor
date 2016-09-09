/* DHT library

MIT license
written by Adafruit Industries
*/
#ifndef DHT_H
#define DHT_H
#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#ifdef __cplusplus
 extern "C" {
#endif
#include "hal.h"
#include "hal_gpt.h"
#include "timer.h"
#include "hal_gpio_7687.h"
// Uncomment to enable printing out nice debug messages.
//#define DHT_DEBUG

// Setup debug printing macros.
#ifdef DHT_DEBUG
  #define DEBUG_PRINT(...) { log_hal_info(__VA_ARGS__); }
  #define DEBUG_PRINTLN(...) { log_hal_info(__VA_ARGS__);log_hal_info("\r\n"); }
#else
  #define DEBUG_PRINT(...) {}
  #define DEBUG_PRINTLN(...) {}
#endif
#ifdef __cplusplus
 }
#endif

// Define types of sensors.
#define DHT11 11
#define DHT22 22
#define DHT21 21
#define AM2301 21


class DHT {
  public:
   DHT();
   DHT(hal_gpio_pin_t pin, uint8_t type);

   void begin(void);
   float readTemperature(uint8_t S=false, uint8_t force=false);
   float convertCtoF(float);
   float convertFtoC(float);
   float computeHeatIndex(float temperature, float percentHumidity, uint8_t isFahrenheit=true);
   float readHumidity(uint8_t force=false);
   uint8_t read(uint8_t force=false);
   UINT8 Fast_gpio_read(UINT32 GPIO_pin);
   INT32 Fast_gpio_write (UINT32 GPIO_pin, UINT8 writeValue);


 //private:
  uint8_t data[5];
  hal_gpio_pin_t _pin;
  uint8_t _type;

  uint32_t _lastreadtime, _maxcycles;
  uint8_t _lastresult;

  uint32_t expectPulse(uint8_t level);

};



#endif
