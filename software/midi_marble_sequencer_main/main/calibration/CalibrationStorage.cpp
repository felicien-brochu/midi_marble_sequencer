#include "CalibrationStorage.h"
#include <esp_log.h>

static const char *TAG = "CalibrationStorage";


CalibrationStorage::CalibrationStorage()
{
    esp_err_t err;
    err = _init_nvs_storage();

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "CalibrationStorage init failed: %d\n", err);
    }
    ESP_ERROR_CHECK(err);
}

esp_err_t CalibrationStorage::save_calibration_data(uint16_t (*thresholds)[NUM_IR_SENS_BY_BOARD][NUM_MARBLE_TYPE - 1])
{
    esp_err_t err;

    // Open NVS handle
    err = nvs_open(CALIBRATION_STORAGE_NAMESPACE, NVS_READWRITE, &_nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
        return err;
    }

    // Write blob
    ESP_LOGI(TAG, "Saving calibration data...");
    err = nvs_set_blob(_nvs_handle, CALIBRATION_STORAGE_BLOB_KEY, thresholds, sizeof(uint16_t) * NUM_IR_SENS_BOARDS * NUM_IR_SENS_BY_BOARD * (NUM_MARBLE_TYPE - 1));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to write calibration data blob! error: (%s)", esp_err_to_name(err));
        nvs_close(_nvs_handle);
        return err;
    }

    // Commit
    err = nvs_commit(_nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to commit calibration data");
    }

    nvs_close(_nvs_handle);
    return err;
}

esp_err_t CalibrationStorage::get_calibration_data(uint16_t (*thresholds)[NUM_IR_SENS_BY_BOARD][NUM_MARBLE_TYPE - 1])
{
    esp_err_t err;

    // Open NVS handle
    err = nvs_open(CALIBRATION_STORAGE_NAMESPACE, NVS_READONLY, &_nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
        return err;
    }

    // Read calibration data blob
    size_t read_len = sizeof(uint16_t) * NUM_IR_SENS_BOARDS * NUM_IR_SENS_BY_BOARD * (NUM_MARBLE_TYPE - 1);

    err = nvs_get_blob(_nvs_handle, CALIBRATION_STORAGE_BLOB_KEY, thresholds, &read_len);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read calibration data blob! error: (%s)", esp_err_to_name(err));
    }

    nvs_close(_nvs_handle);
    return err;
}

esp_err_t CalibrationStorage::_init_nvs_storage()
{
    esp_err_t err;
    
    err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();

        ESP_LOGE(TAG, "NVS NO FREE PAGES or NVS_NEW_VERSION_FOUND\n");
    }
    
    return err;
}
