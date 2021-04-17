#pragma once

#include <esp_idf_version.h>
#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(4, 3, 0)
#include <soc/pcnt_caps.h> // This must be first, this header is missing in hal/pcnt_types.h in IDF 4.2
#endif

#include <esp_err.h>
#include <hal/gpio_types.h>
#include <hal/pcnt_types.h>

#ifdef __cplusplus
extern "C" {
#endif

struct pc_fan_rpm_config
{
    gpio_num_t pin;
    pcnt_unit_t unit;
    pcnt_channel_t channel;
};

struct pc_fan_rpm_handle
{
    pcnt_unit_t unit;
    pcnt_channel_t channel;
    int16_t last_count;
    uint32_t last_timestamp;
};

esp_err_t pc_fan_rpm_init(const struct pc_fan_rpm_config *cfg);

esp_err_t pc_fan_rpm_start_sampling();

esp_err_t pc_fan_rpm_stop_sampling();

#ifdef __cplusplus
}
#endif
