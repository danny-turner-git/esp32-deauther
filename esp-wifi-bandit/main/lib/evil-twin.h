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


#ifndef EVIL_TWIN_HEADER
#define EVIL_TWIN_HEADER

typedef struct{
    wifi_ap_record_t *ap_records;
    uint16_t num_ap;
} Scan;

void scanner();
void evil_twin_start(void* void_clone_ap_record);
void evil_twin_stop();
extern Scan ap_scan;
extern bool evilTwinOn;

#endif
