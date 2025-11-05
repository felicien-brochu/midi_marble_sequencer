#pragma once

#include <controls_main_common.h>

class IEventListener {
public:
    virtual void on_event(controls_main_value_t controls_main_value) = 0;
};