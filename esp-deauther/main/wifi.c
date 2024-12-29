#include <stdio.h>
#include <stdint.h>
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_err.h"

#define SSID "example_ssid"
#define PASSWORD "example_password"

esp_err_t esp_deauther_wifi_config(uint8_t channel){
    ESP_ERROR_CHECK(esp_netif_init());

}
