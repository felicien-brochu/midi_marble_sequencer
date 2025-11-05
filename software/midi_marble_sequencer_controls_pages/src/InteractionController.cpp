#include "InteractionController.h"
#include <cstring>
#include <esp32-hal-log.h>


static const uint8_t _played_pages_buttons_indexes[SEQUENCER_PAGES_NUM] = {8, 9, 10, 11, 12, 13, 14, 15};
static const uint8_t _edited_pages_buttons_indexes[SEQUENCER_PAGES_NUM] = {7, 6, 5, 4, 3, 2, 1, 0};

InteractionController::InteractionController() : _mux(BUTTONS_MUX_S0_GPIO, BUTTONS_MUX_S1_GPIO, BUTTONS_MUX_S2_GPIO, BUTTONS_MUX_S3_GPIO), _played_pages_buttons_controller(_mux, BUTTONS_MUX_SIGNAL_GPIO, _played_pages_buttons_indexes, SEQUENCER_PAGES_NUM), _edited_pages_buttons_controller(_mux, BUTTONS_MUX_SIGNAL_GPIO, _edited_pages_buttons_indexes, SEQUENCER_PAGES_NUM)
{
}

void InteractionController::update()
{
    _played_pages_buttons_controller.update();
    _edited_pages_buttons_controller.update();

    if (_event_listener)
    {
        _event_listener->on_event(get_event());
    }
}

controls_pages_value_t InteractionController::get_event()
{
    controls_pages_value_t controls_pages_value;

    controls_pages_value.played_pages_buttons = _played_pages_buttons_controller.get_push_buttons_events_flags();
    controls_pages_value.edited_pages_buttons = _edited_pages_buttons_controller.get_push_buttons_events_flags();

    return controls_pages_value;
}

void InteractionController::consume_events(controls_pages_display_t controls_pages_display)
{
    _played_pages_buttons_controller.consume_events(controls_pages_display.played_pages_buttons_clicks_consumed);
    _edited_pages_buttons_controller.consume_events(controls_pages_display.edited_pages_buttons_clicks_consumed);
}

void InteractionController::set_event_listener(IEventListener *event_listener)
{
    _event_listener = event_listener;
}
