#pragma once

#include "PushButton.h"
#include <driver/gpio.h>

#define CALIBRATION_BUTTON_GPIO GPIO_NUM_3

class CalibrationButton : public PushButton {
public:
    // Access the singleton instance
    static CalibrationButton& instance() {
        static CalibrationButton inst;
        return inst;
    }

    // non-copyable, non-movable
    CalibrationButton(const CalibrationButton&) = delete;
    CalibrationButton& operator=(const CalibrationButton&) = delete;
    CalibrationButton(CalibrationButton&&) = delete;
    CalibrationButton& operator=(CalibrationButton&&) = delete;

private:
    CalibrationButton() : PushButton(CALIBRATION_BUTTON_GPIO, false) {}
};
