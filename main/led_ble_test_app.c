#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

#include "sample_ble.h"
#include "led_strip_test.h"

static const char *TAG = "Main";

void print_task_info()
{
    TaskStatus_t task_array[20];
    UBaseType_t task_count, i;

    task_count = uxTaskGetNumberOfTasks();

    uxTaskGetSystemState(task_array, task_count, NULL);
    ESP_LOGI("TaskMonitor", "Task count: %d", task_count);
    for (i = 0; i < task_count; i++)
    {
        ESP_LOGI("TaskMonitor", "Task: %s | Core: %d | Priority: %d",
                 task_array[i].pcTaskName, task_array[i].uxCoreAffinityMask, task_array[i].uxBasePriority);
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "Top of app_main");

    ble_init();
    led_strip_init();

    while (1)
    {
        vTaskDelay(5000 / portTICK_PERIOD_MS);

        print_task_info();
    }
}
