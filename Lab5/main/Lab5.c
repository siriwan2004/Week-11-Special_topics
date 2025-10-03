#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include "driver/ledc.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define LDR_CHANNEL      ADC1_CHANNEL_6  // GPIO34
#define LED_MAIN_PIN     18               // LED หลัก
#define LED_STATUS_PIN   19               // LED แสดงสถานะ
#define BUZZER_PIN       21               // Buzzer
#define ALERT_THRESHOLD  1000             // ค่า LDR ต่ำสุดที่เตือน

static const char *TAG = "LDR_ALERT";

void app_main(void)
{
    // ตั้งค่า ADC
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(LDR_CHANNEL, ADC_ATTEN_DB_11);

    // ตั้งค่า PWM สำหรับ LED หลัก
    ledc_timer_config_t ledc_timer = {
        .speed_mode      = LEDC_HIGH_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_12_BIT,
        .timer_num       = LEDC_TIMER_0,
        .freq_hz         = 5000,
        .clk_cfg         = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel = {
        .gpio_num   = LED_MAIN_PIN,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .channel    = LEDC_CHANNEL_0,
        .timer_sel  = LEDC_TIMER_0,
        .duty       = 0,
        .hpoint     = 0
    };
    ledc_channel_config(&ledc_channel);

    // ตั้งค่า LED Status และ Buzzer เป็น Output
    gpio_reset_pin(LED_STATUS_PIN);
    gpio_set_direction(LED_STATUS_PIN, GPIO_MODE_OUTPUT);
    gpio_reset_pin(BUZZER_PIN);
    gpio_set_direction(BUZZER_PIN, GPIO_MODE_OUTPUT);

    while (1) {
        int ldr_value = adc1_get_raw(LDR_CHANNEL);
        ESP_LOGI(TAG, "LDR Value: %d", ldr_value);

        // ควบคุม LED หลักตามค่า LDR (0-4095)
        int duty = (ldr_value * 4095) / 4095;
        ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, duty);
        ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);

        // ระบบเตือน: แสงน้อยเกินไป
        if (ldr_value < ALERT_THRESHOLD) {
            ESP_LOGW(TAG, "⚠️ ALERT: LDR below threshold (%d)", ldr_value);
            gpio_set_level(LED_STATUS_PIN, 1);  // LED Status ON
            gpio_set_level(BUZZER_PIN, 1);      // Buzzer ดัง
        } else {
            gpio_set_level(LED_STATUS_PIN, 0);  // LED Status OFF
            gpio_set_level(BUZZER_PIN, 0);      // Buzzer ปิด
        }

        vTaskDelay(pdMS_TO_TICKS(1000)); // หน่วง 1 วินาที
    }
}
