#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define LDR_CHANNEL      ADC1_CHANNEL_6  // GPIO34
#define LED_STATUS_PIN   19               // LED แสดงสถานะ LDR
#define BUZZER_PIN       21               // Buzzer
#define ALERT_THRESHOLD  1000             // ค่า LDR ต่ำสุดที่เตือน
#define BLINK_DELAY_MS   500              // หน่วง LED กระพริบ

static const char *TAG = "LDR_ALERT";

void app_main(void)
{
    // ตั้งค่า ADC
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(LDR_CHANNEL, ADC_ATTEN_DB_11);

    // ตั้งค่า LED Status และ Buzzer เป็น Output
    gpio_reset_pin(LED_STATUS_PIN);
    gpio_set_direction(LED_STATUS_PIN, GPIO_MODE_OUTPUT);
    gpio_reset_pin(BUZZER_PIN);
    gpio_set_direction(BUZZER_PIN, GPIO_MODE_OUTPUT);

    while (1) {
        int ldr_value = adc1_get_raw(LDR_CHANNEL);
        ESP_LOGI(TAG, "LDR Value: %d", ldr_value);

        if (ldr_value < ALERT_THRESHOLD) {
            // แสงต่ำ: กระพริบ LED และ Buzzer ON
            ESP_LOGW(TAG, "⚠️ ALERT: LDR below threshold (%d)", ldr_value);
            gpio_set_level(BUZZER_PIN, 1);      // Buzzer ON
            ESP_LOGI(TAG, "BUZZER: ON");

            // กระพริบ LED_STATUS
            gpio_set_level(LED_STATUS_PIN, 1);
            ESP_LOGI(TAG, "LED_STATUS: ON (Blink)");
            vTaskDelay(pdMS_TO_TICKS(BLINK_DELAY_MS));
            gpio_set_level(LED_STATUS_PIN, 0);
            ESP_LOGI(TAG, "LED_STATUS: OFF (Blink)");
        } else {
            // แสงเพียงพอ: LED_STATUS ON, Buzzer OFF
            gpio_set_level(LED_STATUS_PIN, 1); // LED สว่างค้าง
            ESP_LOGI(TAG, "LED_STATUS: ON");
            gpio_set_level(BUZZER_PIN, 0);      // Buzzer OFF
            ESP_LOGI(TAG, "BUZZER: OFF");

            vTaskDelay(pdMS_TO_TICKS(1000));   // หน่วง 1 วินาที
        }
    }
}
