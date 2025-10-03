#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include "driver/ledc.h"   // ใช้ PWM
#include "esp_log.h"

#define LDR_CHANNEL ADC1_CHANNEL_6   // GPIO34
#define LED_PIN GPIO_NUM_18          // GPIO18
#define THRESHOLD 2000               // ค่ากลาง
#define ALERT_THRESHOLD 3800         // ค่าขีดจำกัดแจ้งเตือน

static const char *TAG = "LDR_LED";

void app_main(void)
{
    // กำหนด ADC
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(LDR_CHANNEL, ADC_ATTEN_DB_11);

    // กำหนด PWM (LEDC)
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_HIGH_SPEED_MODE,
        .duty_resolution  = LEDC_TIMER_12_BIT,
        .timer_num        = LEDC_TIMER_0,
        .freq_hz          = 5000,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel = {
        .gpio_num   = LED_PIN,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .channel    = LEDC_CHANNEL_0,
        .timer_sel  = LEDC_TIMER_0,
        .duty       = 0,
        .hpoint     = 0
    };
    ledc_channel_config(&ledc_channel);

    while (1) {
        int ldr_value = adc1_get_raw(LDR_CHANNEL);
        ESP_LOGI(TAG, "LDR Value: %d", ldr_value);

        // แมพค่า LDR -> Duty Cycle (0-4095)
        int duty = (ldr_value * 4095) / 4095;

        // แสงน้อย -> หรี่ LED, แสงมาก -> สว่าง
        ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, duty);
        ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);

        if (ldr_value > THRESHOLD) {
            ESP_LOGI(TAG, "LED ON (Dark mode, dimmed)");
        } else {
            ESP_LOGI(TAG, "LED OFF (Bright mode, stronger)");
        }

        // ระบบเตือน
        if (ldr_value > ALERT_THRESHOLD) {
            ESP_LOGW(TAG, "⚠️ ALERT: Sensor over limit (%d)", ldr_value);
        }

        vTaskDelay(pdMS_TO_TICKS(1000)); // หน่วง 1 วินาที
    }
}
