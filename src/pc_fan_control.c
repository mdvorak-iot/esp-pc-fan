#include "pc_fan_control.h"
#include <driver/ledc.h>
#include <esp_log.h>

static const char TAG[] = "pc_fan_control";

static const uint32_t FAN_CONTROL_FREQ_HZ = 25000;
static const ledc_timer_bit_t FAN_CONTROL_RESOLUTION = LEDC_TIMER_10_BIT;
static const float FAN_CONTROL_MAX_DUTY = (float)((1u << FAN_CONTROL_RESOLUTION) - 1);

esp_err_t pc_fan_control_init(gpio_num_t pin, ledc_timer_t timer, ledc_channel_t channel)
{
    if (pin < 0 || !GPIO_IS_VALID_OUTPUT_GPIO(pin))
    {
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGI(TAG, "initializing fan pwm on pin %d (timer %d, channel %d)", pin, timer, channel);

    // Timer
    ledc_timer_config_t timerConfig = {};
    timerConfig.timer_num = timer;
    timerConfig.speed_mode = LEDC_HIGH_SPEED_MODE;
    timerConfig.duty_resolution = FAN_CONTROL_RESOLUTION;
    timerConfig.freq_hz = FAN_CONTROL_FREQ_HZ;
    esp_err_t err = ledc_timer_config(&timerConfig);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "ledc_timer_config failed: %d %s", err, esp_err_to_name(err));
        return err;
    }

    // Channel
    ledc_channel_config_t channelConfig = {};
    channelConfig.timer_sel = timer;
    channelConfig.channel = channel;
    channelConfig.gpio_num = pin;
    channelConfig.speed_mode = LEDC_HIGH_SPEED_MODE;
    channelConfig.duty = 0;
    err = ledc_channel_config(&channelConfig);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "ledc_channel_config failed: %d %s", err, esp_err_to_name(err));
        return err;
    }

    // Success
    return ESP_OK;
}

esp_err_t pc_fan_control_set_duty(ledc_channel_t channel, float duty_percent)
{
    if (duty_percent < 0.0f || duty_percent > 1.0f)
    {
        return ESP_ERR_INVALID_ARG;
    }

    uint32_t duty = (uint32_t)(duty_percent * FAN_CONTROL_MAX_DUTY);
    return ledc_set_duty_and_update(LEDC_HIGH_SPEED_MODE, channel, duty, 0);
}
