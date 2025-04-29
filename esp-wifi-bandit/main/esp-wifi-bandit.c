#include <stdio.h>
#include <string.h>
#include "esp_event.h"
#include "esp_err.h"
#include "freertos/task.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include <driver/gpio.h>
#include <driver/i2c_master.h>
#include <freertos/FreeRTOS.h>
#include "sdkconfig.h"
#include <u8g2.h>
#include "u8g2_esp32_hal.h"
#include "driver/gpio.h"
#include "freertos/queue.h"
#include "lib/deauth-sender.h"
#include "lib/evil-twin.h"
#include "lib/esp_ui.h"
#include "lib/pages.h"

const char * MAIN_TAG = "MAIN_PROCESS";

int ieee80211_raw_frame_sanity_check(int32_t arg, int32_t arg2, int32_t arg3){
  return 0;
}

void app_main(void)
{
	nvs_flash_init(); 
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());


    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT(); 
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));

	esp_netif_create_default_wifi_sta();
	esp_netif_create_default_wifi_ap();

	init_pages();
	esp_ui_init();

}
