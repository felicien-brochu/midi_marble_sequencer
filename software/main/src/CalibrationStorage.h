#include "marble_type.h"
#include "SensorStatistics.h"

#include <inttypes.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <nvs.h>

#define CALIBRATION_STORAGE_NAMESPACE "ir_sens_calib"

class CalibrationStorage
{
public:

    CalibrationStorage();

    uint16_t get_uint16_value(const char *key);
    void set_uint16_value();
    void commit();

private:
    esp_err_t _init_nvs_storage();

    nvs_handle_t _nvs_handle;
};