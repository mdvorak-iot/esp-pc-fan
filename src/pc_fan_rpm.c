#include "pc_fan_rpm.h"
#include <driver/pcnt.h>
#include <esp_log.h>

static const char TAG[] = "pc_fan_rpm";

esp_err_t  pc_fan_rpm_init(const struct pc_fan_rpm_config *cfg)
{
    if (cfg == NULL
        || cfg->unit < 0 || cfg->unit >= PCNT_UNIT_MAX
        || cfg->channel < 0 || cfg->channel <= PCNT_CHANNEL_MAX
        || cfg->pin < 0 || !GPIO_IS_VALID_GPIO(cfg->pin))
    {
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGI(TAG, "configuring pc fan rpm %d/%d on pin %d", cfg->channel, cfg->unit, cfg->pin);

    // Config
    pcnt_config_t pcnt_config = {
        .pulse_gpio_num = cfg->pin,
        .ctrl_gpio_num = PCNT_PIN_NOT_USED,
        .lctrl_mode = PCNT_MODE_KEEP,
        .hctrl_mode = PCNT_MODE_KEEP,
        .pos_mode = PCNT_COUNT_INC,
        .neg_mode = PCNT_COUNT_INC,
        .counter_h_lim = INT16_MAX,
        .counter_l_lim = 0,
        .unit = cfg->unit,
        .channel = cfg->channel,
    };

    // Init
    esp_err_t err;
    err = pcnt_unit_config(&pcnt_config);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "pcnt_unit_config failed: %d %s", err, esp_err_to_name(err));
        return err;
    }

    // Filtering
    // NOTE since we are measuring very low frequencies, we can filter out all short pulses (noise)
    // NOTE counter is 10-bit in APB_CLK cycles (80Mhz), so 1000 = 80kHz, 1023 is max allowed value
    err = pcnt_set_filter_value(pcnt_config.unit, 1023);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "pcnt_set_filter_value failed: %d %s", err, esp_err_to_name(err));
        return err;
    }

    err = pcnt_filter_enable(pcnt_config.unit);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "pcnt_filter_enable failed: %d %s", err, esp_err_to_name(err));
        return err;
    }

    // Start the counter
    err = pcnt_counter_clear(pcnt_config.unit);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "pcnt_counter_clear failed: %d %s", err, esp_err_to_name(err));
        return err;
    }

    err = pcnt_counter_resume(pcnt_config.unit);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "pcnt_counter_resume failed: %d %s", err, esp_err_to_name(err));
        return err;
    }

    // Context


    // OK
    return ESP_OK;
}

esp_err_t pc_fan_rpm_start_sampling()
{
    return ESP_OK;
}

esp_err_t pc_fan_rpm_stop_sampling()
{
    return ESP_OK;
}
