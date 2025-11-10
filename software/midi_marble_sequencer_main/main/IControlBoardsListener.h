#pragma once
#include <controls_main_common.h>
#include <controls_pages_common.h>

class IControlBoardsListener {
public:
    virtual ~IControlBoardsListener() = default;

    virtual void handle_controls_main_event(controls_main_value_t controls_main_value) = 0;
    virtual void handle_controls_pages_event(controls_pages_value_t controls_pages_value) = 0;
    virtual controls_main_display_t get_controls_main_display() = 0;
    virtual controls_pages_display_t get_controls_pages_display() = 0;
};
