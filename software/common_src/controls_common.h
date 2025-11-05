#pragma once

#include <cstdint>

typedef struct {
    bool pushed;
    uint8_t click_events_pending;
} push_button_event_t;