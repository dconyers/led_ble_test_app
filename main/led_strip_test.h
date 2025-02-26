#ifndef LED_STRIP_TEST_H
#define LED_STRIP_TEST_H

#include "esp_err.h"

typedef enum   {
    OFF,
    BLUE,
    RED,
    GREEN,
    WHITE
} LEDStatus;

#define LED_STRIP_GPIO_PIN 2                    // GPIO assignment
#define LED_STRIP_LED_COUNT 5                   // Numbers of the LED in the strip
#define LED_STRIP_RMT_RES_HZ (80 * 1000 * 1000) // 10MHz resolution, 1 tick = 0.1us (led strip needs a high resolution)

esp_err_t led_strip_init(void);

#endif // LED_STRIP_TEST_H