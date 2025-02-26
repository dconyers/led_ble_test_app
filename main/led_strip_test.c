#include "led_strip_test.h"

#include <string.h>

#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/rmt_tx.h"
#include "led_strip_encoder.h"

// ESP_LOG Tag
static const char *TAG = "LED_Strip";

static TaskHandle_t xHandle = NULL;

static uint8_t led_strip_pixels[LED_STRIP_LED_COUNT * 3];
static rmt_channel_handle_t led_chan;
static rmt_encoder_handle_t led_encoder;

static void updateLED(int Red, int Green, int Blue)
{

    ESP_LOGD(TAG, "Setting to %02x %02x %02x", Red, Green, Blue);

    {
        for (int i = 0; i < LED_STRIP_LED_COUNT; i++)
        {
            // ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, i, Red, Green, Blue));
            led_strip_pixels[i * 3 + 0] = Green;
            led_strip_pixels[i * 3 + 1] = Red;
            led_strip_pixels[i * 3 + 2] = Blue;
        }
    }
}

static void controllerTask(void *pvParameters)
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

        rmt_transmit_config_t tx_config = {
            .loop_count = 0, // no transfer loop
        };

        // Flush RGB values to LEDs
        ESP_ERROR_CHECK(rmt_transmit(led_chan, led_encoder, led_strip_pixels, sizeof(led_strip_pixels), &tx_config));
        ESP_ERROR_CHECK(rmt_tx_wait_all_done(led_chan, portMAX_DELAY));

        last_status = (LEDStatus)((((int)last_status) + 1) % (1 + (int)WHITE));

        vTaskDelay(pdMS_TO_TICKS(50)); // Delay for 100 ms
    }
}
esp_err_t led_strip_init(void)
{

    ESP_LOGI(TAG, "Initializing LED strip");

    esp_log_level_set("rmt", ESP_LOG_DEBUG);

    // LED Strip object handle
    ESP_LOGI(TAG, "Create RMT TX channel");
    led_chan = NULL;
    rmt_tx_channel_config_t tx_chan_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT, // select source clock
        .gpio_num = LED_STRIP_GPIO_PIN,
        .mem_block_symbols = 64, // increase the block size can make the LED less flickering
        .resolution_hz = LED_STRIP_RMT_RES_HZ,
        .trans_queue_depth = 4, // set the number of transactions that can be pending in the background
    };
    ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_chan_config, &led_chan));

    ESP_LOGI(TAG, "Install led strip encoder");
    led_encoder = NULL;
    led_strip_encoder_config_t encoder_config = {
        .resolution = LED_STRIP_RMT_RES_HZ,
    };
    ESP_ERROR_CHECK(rmt_new_led_strip_encoder(&encoder_config, &led_encoder));

    ESP_LOGI(TAG, "Enable RMT TX channel");
    ESP_ERROR_CHECK(rmt_enable(led_chan));

    BaseType_t xReturned;
    xReturned = xTaskCreatePinnedToCore(controllerTask,
                                        "LED Controller Task",
                                        3072,
                                        NULL,
                                        5,
                                        &xHandle,
                                        1);

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
