#include "hal.h"
#include "DHT.h"

DHT dht;


void _init(){
	  //Debug
  	hal_gpio_init(HAL_GPIO_35);
  	hal_pinmux_set_function(HAL_GPIO_35, 8);
  	hal_gpio_set_direction(HAL_GPIO_35,HAL_GPIO_DIRECTION_OUTPUT);
    hal_gpio_set_output(HAL_GPIO_35, HAL_GPIO_DATA_HIGH);
    hal_gpt_delay_ms(20);
    hal_gpio_set_output(HAL_GPIO_35, HAL_GPIO_DATA_LOW);


	dht = DHT(HAL_GPIO_38, DHT22);
	dht.begin();
}


float testfloat()
{
	return 0.5f;
}

void _print(){

	int temperature = dht.readTemperature()*100;
    int humidity = dht.readHumidity()*100;
    log_hal_info("Temperature:%d.%d\tHumidity:%d.%d\r\n",temperature/100,temperature%100,humidity/100,humidity%100);

}

extern "C"{

void print_temp_humidity()
{
	_print();	
}
void dht_init()
{
	_init();	
}

}

