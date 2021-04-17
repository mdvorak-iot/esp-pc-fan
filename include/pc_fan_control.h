#pragma once

#include <esp_err.h>
#include <hal/gpio_types.h>
#include <hal/ledc_types.h>

#ifdef __cplusplus
extern "C" {
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
