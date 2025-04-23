#ifndef CD74HC4067_h
#define CD74HC4067_h

#include <inttypes.h>
#include <driver/gpio.h>

class CD74HC4067
{
  public:
    CD74HC4067(gpio_num_t s0, gpio_num_t s1, gpio_num_t s2, gpio_num_t s3);
    void channel(uint8_t channel);

  private:
  	gpio_num_t _address_pins[4];
	uint8_t _channel;
};

#endif
