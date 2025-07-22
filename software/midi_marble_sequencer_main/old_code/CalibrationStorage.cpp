#include "CalibrationStorage.h"


CalibrationStorage::CalibrationStorage()
{
    esp_err_t err;
    err = _init_nvs_storage();

    ESP_ERROR_CHECK(err);
    if (err != ESP_OK)
    {
        printf("CalibrationStorage init failed: %d\n", err);
    }
}

uint16_t CalibrationStorage::get_uint16_value(const char *key)
{
    esp_err_t err;
    uint16_t value;

    err = nvs_get_u16(_nvs_handle, key, &value);
    if (err != ESP_OK)
    {
        if (err == ESP_ERR_NVS_NOT_FOUND)
        {
            printf("CalibrationStorage get uint16 value NOT FOUND: %s\n", key);
        }
    }

    return value;
}

void CalibrationStorage::set_uint16_value()
{

}

void CalibrationStorage::commit()
{
    esp_err_t err;

    err = nvs_commit(_nvs_handle);

    ESP_ERROR_CHECK(err);
    if (err != ESP_OK)
    {
        printf("CalibrationStorage commit failed: %d\n", err);
    }
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

        printf("NVS NO FREE PAGES or NVS_NEW_VERSION_FOUND\n");
    }
    ESP_ERROR_CHECK(err);

    err = nvs_open(CALIBRATION_STORAGE_NAMESPACE, NVS_READWRITE, &_nvs_handle);
    if (err != ESP_OK)
        return err;

    return err;
}
