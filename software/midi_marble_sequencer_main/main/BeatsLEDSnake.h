#pragma once

#include "LEDArray.h"
#include "IRSensBoards.h"

#define BEATS_SNAKE_LED_POWER_PIN GPIO_NUM_10

#define BEATS_SNAKE_MUX_S0_PIN    GPIO_NUM_14
#define BEATS_SNAKE_MUX_S1_PIN    GPIO_NUM_13
#define BEATS_SNAKE_MUX_S2_PIN    GPIO_NUM_12
#define BEATS_SNAKE_MUX_S3_PIN    GPIO_NUM_11

#define BEATS_SNAKE_NUM_LEDS NUM_IR_SENS_BOARDS

class BeatsLEDSnake : public LEDArray
{
public:
    BeatsLEDSnake();
};