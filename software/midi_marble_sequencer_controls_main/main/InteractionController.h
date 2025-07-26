#pragma once

#include "controls_main_common.h"
#include "CD74HC4067.h"
#include "BPMPotentiometer.h"
#include "PlayPauseSwitch.h"
#include "RotaryButtonsController.h"
#include "PushButtonsController.h"


#define BUTTONS_MUX_SIGNAL_GPIO GPIO_NUM_35
#define BUTTONS_MUX_S0_GPIO GPIO_NUM_26
#define BUTTONS_MUX_S1_GPIO GPIO_NUM_25
#define BUTTONS_MUX_S2_GPIO GPIO_NUM_33
#define BUTTONS_MUX_S3_GPIO GPIO_NUM_32

#define BUTTONS_MUX_ADC_UNIT ADC_UNIT_1
#define BUTTONS_MUX_ADC_CHANNEL ADC_CHANNEL_7


class InteractionController
{
public:
    InteractionController();

    void update();
    controls_main_value_t get_value();
    void consume_events();

private:
    adc_oneshot_unit_handle_t _adc_handle;
    CD74HC4067 _mux;

    BPMPotentiometer *_bpm_potentiometer;
    PlayPauseSwitch *_play_pause_switch;
    RotaryButtonsController *_rotary_buttons_controller;
    // PushButtonsController must be initialized after RotaryButtonsController because of GPIO settings order.
    PushButtonsController *_push_buttons_controller;
};