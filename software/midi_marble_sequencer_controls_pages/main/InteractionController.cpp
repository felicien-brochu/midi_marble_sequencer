#include "InteractionController.h"
#include <cstring>
#include <esp_log.h>

static const char *TAG = "InteractionController";

static const uint8_t _played_pages_buttons_indexes[SEQUENCER_PAGES_NUM] = {8, 9, 10, 11, 12, 13, 14, 15};
static const uint8_t _edited_pages_buttons_indexes[SEQUENCER_PAGES_NUM] = {7, 6, 5, 4, 3, 2, 1, 0};

InteractionController::InteractionController() : _mux(BUTTONS_MUX_S0_GPIO, BUTTONS_MUX_S1_GPIO, BUTTONS_MUX_S2_GPIO, BUTTONS_MUX_S3_GPIO), _played_pages_buttons_controller(_mux, BUTTONS_MUX_SIGNAL_GPIO, _played_pages_buttons_indexes, SEQUENCER_PAGES_NUM), _edited_pages_buttons_controller(_mux, BUTTONS_MUX_SIGNAL_GPIO, _edited_pages_buttons_indexes, SEQUENCER_PAGES_NUM)
{
}

void InteractionController::update()
{
    bool has_changed = _played_pages_buttons_controller.update();

    if (has_changed)
    {
        ESP_LOGI(TAG, "_played_pages_buttons_controller has changed: interaction%p", this);
    }
    _edited_pages_buttons_controller.update();
}

void InteractionController::get_value(controls_pages_value_t *value)
{
    // for (size_t i = 0; i < SEQUENCER_PAGES_NUM; i++)
    // {
    //     value->played_pages_buttons[i].click_events_pending = _played_pages_buttons_controller._push_buttons_events[i].click_events_pending;
    //     value->played_pages_buttons[i].pushed = _played_pages_buttons_controller._push_buttons_events[i].pushed;
        
    //     value->edited_pages_buttons[i].click_events_pending = _edited_pages_buttons_controller._push_buttons_events[i].click_events_pending;
    //     value->edited_pages_buttons[i].pushed = _edited_pages_buttons_controller._push_buttons_events[i].pushed;
    // }
    
    _played_pages_buttons_controller.get_push_buttons_events(value->played_pages_buttons);
    _edited_pages_buttons_controller.get_push_buttons_events(value->edited_pages_buttons);

    // memcpy(value.played_pages_buttons, push_buttons_events, SEQUENCER_PAGES_NUM * sizeof(push_button_event_t));
    
    // push_buttons_events = _edited_pages_buttons_controller.get_push_buttons_events();
    // memcpy(value.edited_pages_buttons, push_buttons_events, SEQUENCER_PAGES_NUM * sizeof(push_button_event_t));
}

void InteractionController::consume_events()
{
    _played_pages_buttons_controller.consume_events();
    _edited_pages_buttons_controller.consume_events();
}
