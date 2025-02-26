#include <stdio.h>
#include <string.h>
#include "esp_event.h"
#include "esp_err.h"
#include "esp_check.h"
#include "freertos/task.h"
#include "esp_event.h"
#include "driver/i2c_master.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include <esp_wifi_types_generic.h>
#include "esp_log.h"
#include "esp_system.h" // esp_mac.h no longer included in the system api header file
#include "esp_mac.h"


static const char * TAG = "tag";

void app_main(void)
{
    uint8_t mac[6] = {0};
    esp_read_mac(mac, ESP_MAC_WIFI_SOFTAP);
    ESP_LOGI(TAG, "Mac address is: %02x:%02x:%02x:%02x:%02x:%02x", mac[0],mac[1], mac[2], mac[3], mac[4], mac[5]);

}
