#include "sample_ble.h"

#include <string.h>

#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/ble_gap.h"
#include "esp_log.h"

static const char *TAG = "SampleBLE";

static struct ble_gap_event_listener ble_gap_listener;

// Bluetooth device name
#define DEVICE_NAME "LED_BLE_TEST"

// Advertisement parameters
static const struct ble_gap_adv_params adv_params = {
    .conn_mode = BLE_GAP_CONN_MODE_UND,  // Undirected connectable advertising
    .disc_mode = BLE_GAP_DISC_MODE_GEN,  // General discoverable mode
    .itvl_min = 0,  // Use default interval
    .itvl_max = 0,  // Use default interval
    .channel_map = BLE_GAP_ADV_DFLT_CHANNEL_MAP,  // Default advertising channels
    .filter_policy = BLE_HCI_ADV_FILT_NONE,  // No filter policy (accept all)
    .high_duty_cycle = 0,  // Not high-duty cycle advertising
};
static void ble_advertise(void)
{
    struct ble_hs_adv_fields adv_fields;
    memset(&adv_fields, 0, sizeof(adv_fields));

    adv_fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;
    adv_fields.name = (uint8_t *)DEVICE_NAME;
    adv_fields.name_len = strlen(DEVICE_NAME);
    adv_fields.name_is_complete =  (adv_fields.name_len < 31) ? 1 : 0;

    int rc = ble_gap_adv_set_fields(&adv_fields);
    if (rc != 0)
    {
        ESP_LOGE(TAG, "Failed to set advertisement data: %d", rc);
        return;
    }

    rc = ble_gap_adv_start(BLE_OWN_ADDR_PUBLIC, NULL, BLE_HS_FOREVER, &adv_params, NULL, NULL);
    if (rc == 0)
    {
        ESP_LOGI(TAG, "BLE advertising started.");
    }
    else
    {
        ESP_LOGE(TAG, "Failed to start advertising: %d", rc);
    }
}

// GAP event handler
static int ble_gap_event(struct ble_gap_event *event, void *arg)
{
    switch (event->type)
    {
    case BLE_GAP_EVENT_CONNECT:
        if (event->connect.status == 0)
        {
            ESP_LOGI(TAG, "Device connected.");
        }
        else
        {
            ESP_LOGE(TAG, "Connection failed; restarting advertising.");
            ble_advertise();
        }
        break;

    case BLE_GAP_EVENT_DISCONNECT:
        ESP_LOGI(TAG, "Device disconnected; restarting advertising.");
        ble_advertise();
        break;

    default:
        break;
    }
    return 0;
}

// Bluetooth host task
void ble_host_task(void *param)
{
    nimble_port_run();
}

void ble_on_sync(void) {
    ESP_LOGI("BLE", "NimBLE stack is ready, starting advertisement...");
    ble_advertise();  // Start advertising only after BLE stack is initialized
}

void ble_init(void) {
    ESP_LOGI(TAG, "Initializing NimBLE...");

    ESP_ERROR_CHECK(nimble_port_init());  // Initialize NimBLE stack

    // Register the GAP event handler using the new method
    int rc = ble_gap_event_listener_register(&ble_gap_listener, ble_gap_event, NULL);
    if (rc != 0)
    {
        ESP_LOGE(TAG, "Failed to register GAP event listener: %d", rc);
        return;
    } else {
        ESP_LOGI(TAG, "GAP event listener registered.");
    }

    // Set security configurations (if needed)
    ble_hs_cfg.sm_sc = 1;  // Enable Secure Connections (LE Secure Connections)


    // Register a callback for when the BLE host is ready
    ble_hs_cfg.sync_cb = ble_on_sync;

    // Start the NimBLE host task
    nimble_port_freertos_init(ble_host_task);
}