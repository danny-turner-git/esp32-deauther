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
#include <esp_wifi_types.h>


char * TAG = "debug";
char * AP_TAG = "AP";

typedef struct{
    wifi_ap_record_t *ap_records;
    uint16_t num_ap;
} Scan;

Scan scanner()
{
    esp_netif_create_default_wifi_sta();
    esp_wifi_set_mode(WIFI_MODE_STA);
    ESP_ERROR_CHECK(esp_wifi_start());

    wifi_scan_config_t scan_cfg = {
      .ssid = NULL,
      .bssid = NULL,
      .channel = 0,
      .show_hidden = false
    };
    ESP_ERROR_CHECK(esp_wifi_scan_start(&scan_cfg, true));
    ESP_ERROR_CHECK(esp_wifi_scan_stop());

    uint16_t num_ap;
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&num_ap));
    ESP_LOGI(AP_TAG, "%u APs found", num_ap);

    wifi_ap_record_t *ap_records = calloc(num_ap, sizeof(*ap_records));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&num_ap, ap_records));

    Scan ap_scan;
    ap_scan.ap_records = ap_records;
    ap_scan.num_ap = num_ap;
    ESP_ERROR_CHECK(esp_wifi_stop()); //wifi stopped to allow for switching between wifi modes
    esp_wifi_set_mode(WIFI_MODE_NULL);
    return ap_scan;


};

// this always broadcasts with default host name
void evil_twin(wifi_ap_record_t clone_ap_record)
{
    esp_netif_t* nifx = esp_netif_create_default_wifi_ap();
    esp_wifi_set_mode(WIFI_MODE_AP);


    char * buffer = calloc(sizeof(clone_ap_record.ssid) +16, 1);
    sprintf(buffer, "%s esp", clone_ap_record.ssid);
    ESP_LOGI(AP_TAG, "Selected ssid %s", buffer);


    wifi_config_t wifi_ap_cfg = {};
    strcpy((char *)wifi_ap_cfg.sta.ssid, buffer);
    wifi_ap_cfg.ap.ssid_len = strlen((char *)wifi_ap_cfg.sta.ssid);
    wifi_ap_cfg.ap.channel = 1;
    wifi_ap_cfg.ap.authmode = WIFI_AUTH_OPEN;
    wifi_ap_cfg.ap.ssid_hidden = 0;
    wifi_ap_cfg.ap.max_connection = 10;
    wifi_ap_cfg.ap.beacon_interval = 100;
    tcpip_adapter_init();

    tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_AP, (char*)wifi_ap_cfg.sta.ssid);
    esp_netif_set_hostname(nifx, (char*)wifi_ap_cfg.sta.ssid);




    esp_wifi_set_config(WIFI_MODE_AP, &wifi_ap_cfg );
    ESP_ERROR_CHECK(esp_wifi_start());

    while (true){
        vTaskDelay(10000/ portTICK_PERIOD_MS);
        ESP_LOGI(AP_TAG, "Broadcasting...");
    };
    ESP_ERROR_CHECK(esp_wifi_stop()); //wifi stopped to allow for switching between wifi modes
    esp_wifi_set_mode(WIFI_MODE_NULL);


}

void app_main()
{
    nvs_flash_init(); // wifi configuration stored in nvs
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    //esp_netif_create_default_wifi_sta();
    const wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT(); // create default config here and alter config later
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    Scan ap_scan = scanner();
    int rec = 0;
    for (rec = 0; rec < ap_scan.num_ap; rec++){ // iterate through each record to output found APs
        ESP_LOGI(AP_TAG, "%s", ap_scan.ap_records[rec].ssid);
    };

    wifi_ap_record_t * ap_records_ptr = ap_scan.ap_records;
    evil_twin(*ap_records_ptr);

}
