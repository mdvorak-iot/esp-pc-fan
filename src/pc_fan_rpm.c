#include "pc_fan_rpm.h"
#include <driver/pcnt.h>
#include <esp_log.h>
#include <esp_timer.h>
#include <string.h>

#define COUNTER_HIGH_LIMIT INT16_MAX

static const char TAG[] = "pc_fan_rpm";

esp_err_t pc_fan_rpm_create(const struct pc_fan_rpm_config *cfg, pc_fan_rpm_handle_ptr *out_handle)
{
    if (cfg == NULL
        || cfg->unit < 0 || cfg->unit >= PCNT_UNIT_MAX
        || cfg->channel < 0 || cfg->channel >= PCNT_CHANNEL_MAX
        || cfg->pin < 0 || !GPIO_IS_VALID_GPIO(cfg->pin))
    {
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGI(TAG, "configuring pc fan rpm on pin %d (unit %d)", cfg->pin, cfg->unit);

    // Config
    pcnt_config_t pcnt_config = {
        .pulse_gpio_num = cfg->pin,
        .ctrl_gpio_num = PCNT_PIN_NOT_USED,
        .lctrl_mode = PCNT_MODE_KEEP,
        .hctrl_mode = PCNT_MODE_KEEP,
        .pos_mode = PCNT_COUNT_INC,
        .neg_mode = PCNT_COUNT_INC,
        .counter_h_lim = COUNTER_HIGH_LIMIT,
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

    // Handle
    struct pc_fan_rpm_handle *handle = malloc(sizeof(*handle));
    handle->unit = pcnt_config.unit;

    // OK
    *out_handle = handle;
    return ESP_OK;
}

void pc_fan_rpm_delete(pc_fan_rpm_handle_ptr handle)
{
    if (handle == NULL)
    {
        return;
    }

    // No way to actually delete counter as of idf 4.2
    pcnt_counter_pause(handle->unit);
    pcnt_counter_clear(handle->unit);

    // Free
    free(handle);
}

esp_err_t pc_fan_rpm_counter_value(pc_fan_rpm_handle_ptr handle, int16_t *count)
{
    if (handle == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    return pcnt_get_counter_value(handle->unit, count);
}

esp_err_t pc_fan_rpm_sampling_create(size_t samples_len, pc_fan_rpm_handle_ptr handle, pc_fan_rpm_sampling_ptr *out_sampling)
{
    if (handle == NULL || out_sampling == NULL || samples_len < 1)
    {
        return ESP_ERR_INVALID_ARG;
    }

    // Current counter value
    // NOTE this also validates that counter is initialized
    int16_t count = 0;
    esp_err_t err = pc_fan_rpm_counter_value(handle, &count);
    if (err != ESP_OK)
    {
        return err;
    }

    // Prepare structure
    struct pc_fan_rpm_sampling *sampling = malloc(sizeof(*sampling));
    memset(sampling, 0, sizeof(*sampling));

    sampling->handle = handle;
    sampling->samples_len = samples_len;
    sampling->samples = calloc(samples_len, sizeof(*sampling->samples));

    int64_t now = esp_timer_get_time();

    for (size_t i = 0; i < samples_len; i++)
    {
        sampling->samples[i].timestamp = now;
        sampling->samples[i].count = count;
        sampling->samples[i].value = 0;
    }

    // Return
    *out_sampling = sampling;
    return ESP_OK;
}

void pc_fan_rpm_sampling_delete(pc_fan_rpm_sampling_ptr sampling)
{
    if (sampling == NULL)
    {
        return;
    }

    free(sampling->samples);
    free(sampling);
}

esp_err_t pc_fan_rpm_sample(pc_fan_rpm_sampling_ptr sampling, uint16_t *rpm)
{
    if (sampling == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    // Current time
    int64_t now = esp_timer_get_time();

    // Get count (ignore error, just use zero)
    int16_t count = 0;
    pc_fan_rpm_counter_value(sampling->handle, &count);

    // Calculate
    size_t oldest_index = (sampling->samples_index + 1) % sampling->samples_len;
    struct pc_fan_rpm_readout *oldest = &sampling->samples[oldest_index];

    uint32_t revs = ((uint32_t)COUNTER_HIGH_LIMIT + count - oldest->count) % COUNTER_HIGH_LIMIT;
    int64_t elapsed = now - oldest->timestamp;

    // 15000000 = 60000000 / pulses per rev / rising and falling edge = 60000000 / 2 / 2
    int16_t value = (int16_t)(15000000 * revs / elapsed);

    // Average
    sampling->value_total -= oldest->value;
    sampling->value_total += value;

    // Add new readout - overwrite the oldest index and use it as new one (this handles index overflow as well)
    oldest->timestamp = now;
    oldest->count = count;
    oldest->value = value;
    sampling->samples_index = oldest_index;

    // Expose final value
    sampling->rpm = sampling->value_total / sampling->samples_len;

    if (rpm != NULL)
    {
        *rpm = sampling->rpm;
    }

    // Success
    return ESP_OK;
}

int16_t pc_fan_rpm_sampling_last_count(pc_fan_rpm_sampling_ptr sampling)
{
    if (!sampling) return 0;
    return sampling->samples[sampling->samples_index].count;
}

uint16_t pc_fan_rpm_sampling_last_rpm(pc_fan_rpm_sampling_ptr sampling)
{
    if (!sampling) return 0;
    return sampling->rpm;
}

static void pc_fan_rpm_sampling_handler(void *arg)
{
    pc_fan_rpm_sampling_ptr sampling = (pc_fan_rpm_sampling_ptr)arg;
    esp_err_t err = pc_fan_rpm_sample(sampling, NULL);
    if (err != ESP_OK)
    {
        ESP_LOGD(TAG, "failed to sample rpm: %d", err);
    }
}

esp_err_t pc_fan_rpm_sampling_timer_create(pc_fan_rpm_sampling_ptr sampling, esp_timer_handle_t *out_timer)
{
    esp_timer_create_args_t rpm_timer_args = {
        .callback = pc_fan_rpm_sampling_handler,
        .arg = sampling,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "rpm_sampling",
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 4, 0)
        .skip_unhandled_events = true,
#endif
    };
    return esp_timer_create(&rpm_timer_args, out_timer);
}
