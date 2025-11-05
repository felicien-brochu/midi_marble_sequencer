#pragma once

#include <inttypes.h>
#include <Arduino.h>

class CD74HC4067
{
  public:
    CD74HC4067(uint8_t s0, uint8_t s1, uint8_t s2, uint8_t s3);
    void channel(uint8_t channel);

  private:
    uint8_t _address_pins[4];
    uint8_t _channel;
};