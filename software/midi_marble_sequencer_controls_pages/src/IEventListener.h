#pragma once

#include <controls_pages_common.h>

class IEventListener {
public:
    virtual void on_event(controls_pages_value_t controls_pages_value) = 0;
};