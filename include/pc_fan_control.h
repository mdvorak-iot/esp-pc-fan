#pragma once

#include <esp_err.h>
#include <hal/gpio_types.h>
#include <hal/ledc_types.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef PC_FAN_LEDC_SPEED_MODE
#if CONFIG_PC_FAN_LEDC_HIGH_SPEED_MODE
#define PC_FAN_LEDC_SPEED_MODE LEDC_HIGH_SPEED_MODE
#elif CONFIG_PC_FAN_LEDC_LOW_SPEED_MODE
#define PC_FAN_LEDC_SPEED_MODE LEDC_LOW_SPEED_MODE
#endif
#endif

esp_err_t pc_fan_control_init(gpio_num_t pin, ledc_timer_t timer, ledc_channel_t channel);

/**
 * Sets the duty according to percentual value.
 *
 * @param channel Channel which was used to configure the PWM.
 * @param duty_percent Duty value from 0.0 to 1.0
 * @return ESP_OK on success.
 */
esp_err_t pc_fan_control_set_duty(ledc_channel_t channel, float duty_percent);

#ifdef __cplusplus
}
#endif
