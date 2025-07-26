#include "InteractionController.h"
#include <cstring>

InteractionController::InteractionController() : _mux(BUTTONS_MUX_S0_GPIO, BUTTONS_MUX_S1_GPIO, BUTTONS_MUX_S2_GPIO, BUTTONS_MUX_S3_GPIO)
{
    adc_oneshot_unit_init_cfg_t adc_init_config = {
        .unit_id = BPM_POT_ADC_UNIT,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&adc_init_config, &_adc_handle));

    _bpm_potentiometer = new BPMPotentiometer(_adc_handle);
    _play_pause_switch = new PlayPauseSwitch();

    _rotary_buttons_controller = new RotaryButtonsController(_adc_handle, _mux, BUTTONS_MUX_ADC_CHANNEL);
    // PushButtonsController must be initialized after RotaryButtonsController because of GPIO settings order.
    _push_buttons_controller = new PushButtonsController(_mux, BUTTONS_MUX_SIGNAL_GPIO);
}

void InteractionController::update()
{
    _play_pause_switch->update();
    _bpm_potentiometer->update();
    _push_buttons_controller->update();
    _rotary_buttons_controller->update();
}

controls_main_value_t InteractionController::get_value()
{
    controls_main_value_t value;
    value.bpm_potentiometer_value = _bpm_potentiometer->get_normalized_value();
    value.play_pause_switch_pushed = _play_pause_switch->is_pushed();
    
    push_button_event_t *push_buttons_events = _push_buttons_controller->get_push_buttons_events();
    memcpy(value.tracks_push_buttons, push_buttons_events, SEQUENCER_TRACKS_NUM * sizeof(push_button_event_t));

    rotary_button_state_t *rotary_buttons_states = _rotary_buttons_controller->get_rotary_buttons_states();
    memcpy(value.rotary_buttons_states, rotary_buttons_states, SEQUENCER_MEASURES_NUM * sizeof(rotary_button_state_t));

    return value;
}

void InteractionController::consume_events()
{
    _push_buttons_controller->consume_events();
}
