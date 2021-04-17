#pragma once

#include <esp_err.h>
#include <esp_timer.h>
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
};

typedef struct pc_fan_rpm_handle *pc_fan_rpm_handle_ptr;

esp_err_t pc_fan_rpm_create(const struct pc_fan_rpm_config *cfg, pc_fan_rpm_handle_ptr *out_handle);

void pc_fan_rpm_delete(pc_fan_rpm_handle_ptr handle);

esp_err_t pc_fan_rpm_counter_value(pc_fan_rpm_handle_ptr handle, int16_t *count);

struct pc_fan_rpm_sampling
{
    pc_fan_rpm_handle_ptr handle;
    uint16_t rpm;
    uint32_t value_total;
    size_t samples_len;
    size_t samples_index;
    struct pc_fan_rpm_readout
    {
        int64_t timestamp;
        int16_t count;
        uint16_t value;
    } * samples;
};
typedef struct pc_fan_rpm_sampling *pc_fan_rpm_sampling_ptr;

esp_err_t pc_fan_rpm_sampling_create(size_t samples_len, pc_fan_rpm_handle_ptr handle, pc_fan_rpm_sampling_ptr *out_sampling);

void pc_fan_rpm_sampling_delete(pc_fan_rpm_sampling_ptr sampling);

esp_err_t pc_fan_rpm_sample(pc_fan_rpm_sampling_ptr sampling, uint16_t *rpm);

uint16_t pc_fan_rpm_last_value(pc_fan_rpm_sampling_ptr sampling);

esp_err_t pc_fan_rpm_sampling_timer_create(pc_fan_rpm_sampling_ptr sampling, esp_timer_handle_t *out_timer);

#ifdef __cplusplus
}
#endif
