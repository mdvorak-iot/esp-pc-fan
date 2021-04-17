#include <driver/ledc.h>
#include <esp_event.h>
#include <esp_log.h>
#include <pc_fan_control.h>
#include <pc_fan_rpm.h>

#define HW_PWM_PIN CONFIG_HW_PWM_PIN
#define HW_RPM_PIN CONFIG_HW_RPM_PIN
#define HW_PWM_TIMER LEDC_TIMER_0
#define HW_PWM_CHANNEL LEDC_CHANNEL_0

static const char TAG[] = "example";

void app_main()
{
    // System services
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(ledc_fade_func_install(0));

    // Init controller
    ESP_ERROR_CHECK_WITHOUT_ABORT(pc_fan_control_init(HW_PWM_PIN, HW_PWM_TIMER, HW_PWM_CHANNEL));
    ESP_ERROR_CHECK_WITHOUT_ABORT(pc_fan_control_set_duty(HW_PWM_CHANNEL, 0.5f));

    // Init RPM
    // TODO

    // Setup complete
    ESP_LOGI(TAG, "started");
}
