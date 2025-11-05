#include "CD74HC4067_arduino.h"

void init_address_gpio(uint8_t pin) {
  pinMode(pin, OUTPUT);
}

CD74HC4067::CD74HC4067(uint8_t s0, uint8_t s1, uint8_t s2, uint8_t s3)
{
  _channel = 0;

  bool gpio_nc_detected = false;
  uint8_t gpios[4] = {s0, s1, s2, s3};

  for (uint8_t i = 0; i < 4; i++)
  {
    if (gpios[i] < 255 && !gpio_nc_detected)
    {
      init_address_gpio(gpios[i]);
      _address_pins[i] = gpios[i];
    }
    else {
      _address_pins[i] = GPIO_NUM_NC;
      gpio_nc_detected = true;
    }
  }
}

void CD74HC4067::channel(uint8_t channel)
{
  if (_channel != channel) {
    _channel = channel;
    uint8_t address_mask = 1;

    for (uint8_t i = 0; i < 4 && _address_pins[i] != GPIO_NUM_NC; i++)
    {
      uint32_t level = (channel & address_mask) != 0;
      digitalWrite(_address_pins[i], level);
      address_mask = address_mask << 1;
    }
  }
}
