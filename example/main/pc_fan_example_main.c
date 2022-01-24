#include "sdkconfig.h"
#include <driver/ledc.h>
#include <esp_event.h>
#include <esp_log.h>
#include <pc_fan_control.h>
#include <pc_fan_rpm.h>

#define HW_PWM_PIN CONFIG_HW_PWM_PIN
#define HW_PWM_TIMER LEDC_TIMER_0
#define HW_PWM_CHANNEL LEDC_CHANNEL_0
#define HW_RPM_PIN CONFIG_HW_RPM_PIN
#define HW_RPM_UNIT PCNT_UNIT_3
#define HW_RPM_SAMPLES CONFIG_HW_RPM_SAMPLES

static const char TAG[] = "example";

static pc_fan_rpm_sampling_ptr rpm_sampling = NULL;

void setup()
{
    // System services
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(ledc_fade_func_install(0));

    // Init controller
    ESP_ERROR_CHECK(pc_fan_control_init(HW_PWM_PIN, HW_PWM_TIMER, HW_PWM_CHANNEL));
    ESP_ERROR_CHECK(pc_fan_control_set_duty(HW_PWM_CHANNEL, 0.5f));

    // Init RPM
    struct pc_fan_rpm_config rpm_cfg = {
        .pin = (gpio_num_t)HW_RPM_PIN,
        .unit = (pcnt_unit_t)HW_RPM_UNIT,
    };
    pc_fan_rpm_handle_ptr rpm_handle = NULL;
    ESP_ERROR_CHECK(pc_fan_rpm_create(&rpm_cfg, &rpm_handle));
    ESP_ERROR_CHECK(pc_fan_rpm_sampling_create(HW_RPM_SAMPLES, rpm_handle, &rpm_sampling));
}

_Noreturn void app_main()
{
    // Setup
    setup();

    // Run
    ESP_LOGI(TAG, "started");

    while (1)
    {
        // Cycle duty by 25% every 10 seconds
        float duty = (float)((esp_timer_get_time() / 10000000) % 4) * 0.25f;
        ESP_ERROR_CHECK_WITHOUT_ABORT(pc_fan_control_set_duty(HW_PWM_CHANNEL, duty));

        // Print RPM
        uint16_t rpm = 0;
        ESP_ERROR_CHECK_WITHOUT_ABORT(pc_fan_rpm_sample(rpm_sampling, &rpm));
        ESP_LOGI(TAG, "rpm: %d\t duty: %d", rpm, (int)(duty * 100));

        // Wait
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
}
