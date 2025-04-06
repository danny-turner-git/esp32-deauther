#include <string.h>
#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_event.h"
#include "esp_err.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_log.h"


char * TAG = "debug";
char * AP_TAG = "AP";

typedef struct{
    wifi_ap_record_t *ap_records;
    uint16_t num_ap;
} Scan;

Scan scanner()
{
    wifi_scan_config_t scan_cfg = {
      .ssid = NULL,
      .bssid = NULL,
      .channel = 0,
      .show_hidden = false
    };
    esp_err_t ret = esp_wifi_scan_start(&scan_cfg, true);
    ESP_ERROR_CHECK(esp_wifi_scan_stop());

    uint16_t num_ap;
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&num_ap));
    ESP_LOGI(AP_TAG, "%u APs found", num_ap);

    wifi_ap_record_t *ap_records = calloc(num_ap, sizeof(*ap_records));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&num_ap, ap_records));

    Scan ap_scan;
    ap_scan.ap_records = ap_records;
    ap_scan.num_ap = num_ap;
    return ap_scan;


};

void app_main()
{
    nvs_flash_init(); // wifi configuration stored in nvs
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();


    const wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT(); // create default config here and alter config later
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    esp_wifi_set_mode(WIFI_MODE_APSTA); // STA needed for scanning and AP needed for evil twin
    ESP_ERROR_CHECK(esp_wifi_start());

    Scan ap_scan = scanner();

    int rec = 0;
    for (rec = 0; rec < ap_scan.num_ap; rec++){ // iterate through each record to output found APs
        ESP_LOGI(AP_TAG, "%s", ap_scan.ap_records[rec].ssid);
    };

}
