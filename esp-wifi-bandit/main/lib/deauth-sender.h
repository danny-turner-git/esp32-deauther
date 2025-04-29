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


#ifndef DEAUTH_SENDER_H
#define DEAUTH_SENDER_H

extern uint8_t deauth_frame[];
extern bool stopDeauth;
void deauth_start();
void deauth_stop();

#endif
