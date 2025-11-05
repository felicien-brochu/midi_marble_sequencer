#include "InteractionController.h"
#include <cstring>
#include <esp32-hal-log.h>

static const uint8_t _tracks_buttons_indexes[SEQUENCER_TRACKS_NUM] = {8, 9, 10, 11, 12, 13, 14, 15};

InteractionController::InteractionController() : _mux(BUTTONS_MUX_S0_GPIO, BUTTONS_MUX_S1_GPIO, BUTTONS_MUX_S2_GPIO, BUTTONS_MUX_S3_GPIO)
{
    _event_listener = NULL;
    _bpm_potentiometer = new BPMPotentiometer();
    _play_pause_switch = new PlayPauseSwitch();

    _rotary_buttons_controller = new RotaryButtonsController(_mux, BUTTONS_MUX_ADC_GPIO);
    // PushButtonsController must be initialized after RotaryButtonsController because of GPIO settings order.
    _push_buttons_controller = new PushButtonsController(_mux, BUTTONS_MUX_SIGNAL_GPIO, _tracks_buttons_indexes, SEQUENCER_TRACKS_NUM);
}

void InteractionController::update()
{
    _play_pause_switch->update();
    _bpm_potentiometer->update();
    _push_buttons_controller->update();
    _rotary_buttons_controller->update();

    if (_event_listener)
    {
        _event_listener->on_event(get_event());
    }
}

controls_main_value_t InteractionController::get_event()
{
    controls_main_value_t controls_main_value;

    controls_main_value.bpm_potentiometer_value = _bpm_potentiometer->get_normalized_value();
    controls_main_value.play_pause_switch_pushed = _play_pause_switch->is_pushed();
    
    controls_main_value.tracks_push_buttons = _push_buttons_controller->get_push_buttons_events_flags();

    // for (size_t i = 0; i < SEQUENCER_TRACKS_NUM; i++)
    // {
    //     if (push_buttons_events[i].click_events_pending > 0)
    //     {
    //         log_i("click[%d] registered", i);
    //     }
    // }

    rotary_button_state_t *rotary_buttons_states = _rotary_buttons_controller->get_rotary_buttons_states();
    memcpy(controls_main_value.rotary_buttons_states, rotary_buttons_states, SEQUENCER_MEASURES_NUM * sizeof(rotary_button_state_t));

    return controls_main_value;
}

void InteractionController::consume_events(controls_main_display_t controls_main_display)
{
    _push_buttons_controller->consume_events(controls_main_display.tracks_buttons_clicks_consumed);
}

void InteractionController::set_event_listener(IEventListener *event_listener)
{
    _event_listener = event_listener;
}
