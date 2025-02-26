#include "led_strip_test.h"

#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "led_strip.h"

// ESP_LOG Tag
static const char *TAG = "LED_Strip";

static TaskHandle_t xHandle = NULL;


static led_strip_handle_t led_strip = NULL;

static void updateLED(int Red, int Green, int Blue)
{
    if (led_strip == NULL)
    {
        ESP_LOGE(TAG, "LED strip not initialized!");
        return;
    }

    ESP_LOGD(TAG, "Setting to %02x %02x %02x", Red, Green, Blue);

    if (Red == 0 && Green == 0 && Blue == 0)
    {
        ESP_LOGI(TAG, "Setting clear");
        ESP_ERROR_CHECK(led_strip_clear(led_strip));
    }
    else
    {
        for (int i = 0; i < LED_STRIP_LED_COUNT; i++)
        {
            ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, i, Red, Green, Blue));
        }
        ESP_LOGI(TAG, "calling led_strip_refresh");
        ESP_ERROR_CHECK(led_strip_refresh(led_strip));
    }
}

static void controllerTask(void * pvParameters)
{
    ESP_LOGD(TAG, "controllerTask() called");
    static LEDStatus last_status = OFF;

    while (1)
    {

        switch (last_status)
        {
        case OFF:
            updateLED(0, 0, 0);
            break;
        case BLUE:
            updateLED(0, 0, 255);
            break;
        case RED:
            updateLED(255, 0, 0);
            break;
        case GREEN:
            updateLED(0, 255, 0);
            break;
        case WHITE:
            updateLED(255, 255, 255);
            break;
        }

        last_status = (LEDStatus)((((int)last_status) + 1) % (1 + (int)WHITE));

        vTaskDelay(pdMS_TO_TICKS(50)); // Delay for 100 ms
    }
}


esp_err_t led_strip_init(void)
{

    ESP_LOGI(TAG, "Initializing LED strip");

    // LED strip general initialization, according to your led board design
    led_strip_config_t strip_config = {
        .strip_gpio_num = LED_STRIP_GPIO_PIN,                        // The GPIO that connected to the LED strip's data line
        .max_leds = LED_STRIP_LED_COUNT,                             // The number of LEDs in the strip,
        .led_model = LED_MODEL_SK6812,                               // LED strip model
        .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB, // The color order of the strip: GRB
        .flags = {
            .invert_out = false, // don't invert the output signal
        }};

    // LED strip backend configuration: RMT
    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_APB,            // different clock source can lead to different power consumption
        .resolution_hz = LED_STRIP_RMT_RES_HZ, // RMT counter clock frequency
        .mem_block_symbols = 64,               // the memory size of each RMT channel, in words (4 bytes)
        .flags = {
            .with_dma = false, // DMA feature is available on chips like ESP32-S3/P4
        }};

    esp_log_level_set("rmt", ESP_LOG_DEBUG);

    // LED Strip object handle
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    ESP_LOGI(TAG, "Created LED strip object with RMT backend");

    BaseType_t xReturned;
    xReturned = xTaskCreate(controllerTask,
                            "LED Controller Task",
                            3072,
                            NULL,
                            5,
                            &xHandle);

    if (xReturned == pdPASS)
    {
        ESP_LOGD(TAG, "Successfully Created Force Update Task");
    }
    else
    {
        ESP_LOGE(TAG, "Failed to Create Force Update Task");
        return ESP_FAIL;
    }

    return ESP_OK;
}

