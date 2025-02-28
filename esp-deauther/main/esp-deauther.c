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

const uint8_t deauth_frame[] = {
    // frame control
}

void app_main(void)
{
    nvs_flash_init(); // wifi configuration stored in nvs
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();


    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT(); // create default config here and alter config later
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    esp_wifi_set_mode(WIFI_MODE_AP); // set mode in order to use alter AP configs

    ESP_ERROR_CHECK(esp_wifi_start());

    esp_wifi_80211_tx(WIFI_IF_STA, &deauth_frame, sizeof(deauth_frame), true);

}
