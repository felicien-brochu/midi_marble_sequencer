#pragma once

#include "controls_main_common.h"
#include "CD74HC4067_arduino.h"
#include "BPMPotentiometer.h"
#include "PlayPauseSwitch.h"
#include "RotaryButtonsController.h"
#include "PushButtonsController.h"
#include "IEventListener.h"


#define BUTTONS_MUX_SIGNAL_GPIO 35
#define BUTTONS_MUX_S0_GPIO 26
#define BUTTONS_MUX_S1_GPIO 25
#define BUTTONS_MUX_S2_GPIO 33
#define BUTTONS_MUX_S3_GPIO 32

// #define BUTTONS_MUX_ADC_UNIT ADC_UNIT_1
// #define BUTTONS_MUX_ADC_CHANNEL ADC_CHANNEL_7
#define BUTTONS_MUX_ADC_GPIO 35


class InteractionController
{
public:
    InteractionController();

    void update();
    controls_main_value_t get_event();
    void consume_events(controls_main_display_t controls_main_display);
    void set_event_listener(IEventListener *event_listener);

private:
    IEventListener *_event_listener;
    CD74HC4067 _mux;

    BPMPotentiometer *_bpm_potentiometer;
    PlayPauseSwitch *_play_pause_switch;
    RotaryButtonsController *_rotary_buttons_controller;
    PushButtonsController *_push_buttons_controller;
};