#include "marble_type.h"
#include "IRSensBoards.h"

#include <inttypes.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <nvs.h>

#define CALIBRATION_STORAGE_NAMESPACE "calibration"
#define CALIBRATION_STORAGE_BLOB_KEY "ir_sens_thresh"

class CalibrationStorage
{
public:

    CalibrationStorage();

    esp_err_t save_calibration_data(uint16_t (*thresholds)[NUM_IR_SENS_BY_BOARD][NUM_MARBLE_TYPE - 1]);
    esp_err_t get_calibration_data(uint16_t (*thresholds)[NUM_IR_SENS_BY_BOARD][NUM_MARBLE_TYPE - 1]);
    
private:
    esp_err_t _init_nvs_storage();

    nvs_handle_t _nvs_handle;
};