#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

#include "sample_ble.h"
#include "led_strip_test.h"

static const char *TAG = "Main";


void app_main(void)
{
    ESP_LOGI(TAG, "Top of app_main");

    ble_init();
    led_strip_init();

    while (1)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        ESP_LOGI(TAG, "In main loop...");

    }
}
