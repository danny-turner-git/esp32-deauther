#include <driver/gpio.h>
#include <driver/i2c_master.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>
#include <string.h>
#include "sdkconfig.h"
#include <u8g2.h>
#include "u8g2_esp32_hal.h"
#include "driver/gpio.h"
#include "freertos/queue.h"
#include "deauth-sender.h"
#include "evil-twin.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_err.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include <esp_wifi_types.h>
#include "pages.h"

#define PIN_SDA 21
#define PIN_SCL 22
#define ORIGIN_X 40
#define ORIGIN_Y 15
#define OPTION_PADDING 15
#define OPTION_BACK_Y 120
#define OPTION_BACK_POS 8

const char* OLED_TAG = "sh1107";
const char* PAGE_TAG = "PAGE";
const char* BUTTON_TAG = "BUTTON";

const int UP_BUTTON_GPIO = 12;
const int DOWN_BUTTON_GPIO = 27;
const int ENTER_BUTTON_GPIO = 14;

int runningTasks = 0;
u8g2_t u8g2;
QueueHandle_t buttonQueue; 
bool display_idle = true;
Page *current_page;

void display_page(void* arg){
	display_idle = false;
	Page * page_to_display = (Page *)arg;	
	if (page_to_display == &home_page){ 
		home_processes_textline.text = (char*)malloc(50 * sizeof(char)); //use malloc to prevent null pointer dereference
		sprintf(home_processes_textline.text, "%d tasks running...", runningTasks );
		if (stopDeauth){
			home_deauth_textline.page_on_enter = &deauth_start_page;
		}else{
			home_deauth_textline.page_on_enter = &deauth_stop_page;
		}
		if (evilTwinOn){
			home_evil_twin_textline.page_on_enter = &evil_twin_stop_page;
		}else{
			home_evil_twin_textline.page_on_enter = &evil_twin_start_page;
		}
	}else if (page_to_display == &clone_ap_page && current_page != &clone_ap_page){ // fills out placeholder textlines in clone ap page
		
		int count = 0;
		TextLine *current_textline = page_to_display->head_textline->next_line;	
		while (count <= 5 && count < ap_scan.num_ap){
			if (strcmp((char*)ap_scan.ap_records[count].ssid, (char*)ap_scan.ap_records[count+1].ssid) == 0){ // we compare strings to prevent duplicate ssids from being displayed
				count++;
			}else {
				current_textline->text = (char*)malloc(50 *sizeof(char)); //use malloc to prevent null pointer dereference
				sprintf(current_textline->text, "AP %d: %s", current_textline->position, ap_scan.ap_records[count].ssid);
				current_textline = current_textline ->next_line;
				count++;
			}
	
			
		}
	}
	u8g2_ClearBuffer(&u8g2);	
	TextLine * current_line = page_to_display->head_textline;	
	while(current_line != NULL){	
		if (current_line->text != NULL){ // Check this because placeholder textlines in clone ap page can have no text
			if (current_line == page_to_display->selected_line){
				u8g2_DrawButtonUTF8(&u8g2, ORIGIN_X , ORIGIN_Y + (OPTION_PADDING * current_line->position), U8G2_BTN_BW2, 0,  2,  2, current_line->text);	

			}else{
				u8g2_DrawStr(&u8g2, ORIGIN_X , ORIGIN_Y + (OPTION_PADDING * current_line->position), current_line->text);	
			
			}
		}
		current_line = current_line->next_line;
		
	}
	u8g2_SendBuffer(&u8g2);
	
	ESP_LOGI(PAGE_TAG, "Page displayed: %s", page_to_display->name);

	current_page = page_to_display; 
	display_idle = true;
	vTaskDelete(NULL);
}

void IRAM_ATTR on_up_button_intr(){
	xQueueSendFromISR(buttonQueue, &UP_BUTTON_GPIO, NULL);
}

void IRAM_ATTR on_enter_button_intr(){
	xQueueSendFromISR(buttonQueue, &ENTER_BUTTON_GPIO, NULL);
}


void IRAM_ATTR on_down_button_intr(){
	xQueueSendFromISR(buttonQueue, &DOWN_BUTTON_GPIO, NULL);	
}

void StartProcess(){
	switch (current_page->selected_line->process_on_enter){
		case DEAUTH_START:
			xTaskCreatePinnedToCore(deauth_start, "deauth_start", 4096, NULL, 10, NULL, 1);
			break;
		case DEAUTH_STOP:
			xTaskCreatePinnedToCore(deauth_stop, "deauth_stop", 4096, NULL, 10, NULL, 1);
			break;
		case SCANNER:
			ESP_LOGI(BUTTON_TAG, "Starting Scanner Task");
			xTaskCreatePinnedToCore(scanner, "scanner", 4096, NULL, 10, NULL, 1);
			ESP_LOGI(BUTTON_TAG, "Scanner task started");
			vTaskDelay(3000/portTICK_PERIOD_MS);
			xTaskCreatePinnedToCore(display_page, "display page", 4096, &clone_ap_page, 10, NULL,1);
			break;
		case EVIL_TWIN_START:
			xTaskCreatePinnedToCore(evil_twin_start, "evil_twin_start", 4096, &ap_scan.ap_records[current_page->selected_line->position -1], 10, NULL, 1);
			//ensures record with correct ssid is passed into evil_twin_start
			break;
		case EVIL_TWIN_STOP:
			xTaskCreatePinnedToCore(evil_twin_stop, "evil_twin_stop", 4096, NULL, 10, NULL, 1);
			break;
		case NO_PROCESS:
			break;
		default:
			ESP_LOGI(PAGE_TAG, "Invalid process_on_enter in TextLine %s, Page %s", current_page->selected_line->text, current_page->name);
			break;
	}
}

void ButtonTask(){
	ESP_LOGI(BUTTON_TAG, "Initializing button task");
	int gpio_received;
	while (1){
		xQueueReceive(buttonQueue, &gpio_received, portMAX_DELAY);
		if (display_idle){ // interrupts are ignored if page is being displayed	
			switch (gpio_received){
				case DOWN_BUTTON_GPIO:
					if (current_page->selected_line->next_line == NULL){
						break;
					}

					TextLine * down_line = current_page->selected_line->next_line;					
					current_page->selected_line = down_line;
					xTaskCreatePinnedToCore(display_page, "display_page", 4096, current_page,10, NULL, 0);
					ESP_LOGI(BUTTON_TAG, "Down button pressed");
					break;
				case UP_BUTTON_GPIO:
					
					if (current_page->selected_line->position == 0){ 
						break;
					}

					
					TextLine *current_line = current_page->head_textline;	
					while(current_line->next_line != current_page->selected_line){ // finds the previous textline
						current_line = current_line->next_line;
					}
					
					current_page->selected_line = current_line;
					xTaskCreatePinnedToCore(display_page, "display_page", 4096, current_page,10, NULL, 0);	
					ESP_LOGI(BUTTON_TAG, "Up button pressed");	
					break;

				case ENTER_BUTTON_GPIO:
					if (current_page->selected_line->page_on_enter == NULL){ //valid option will always link to another page
						break;
					}
					
					Page *saved_page = current_page;
					
					xTaskCreatePinnedToCore(display_page, "display_page", 4096, current_page->selected_line->page_on_enter,10, NULL, 0);
					ESP_LOGI(BUTTON_TAG, "Delaying and displaying scanning page");
				
					if (saved_page->selected_line->process_on_enter != NO_PROCESS){
						StartProcess();
					}
					break;
					
				default:
					ESP_LOGI(BUTTON_TAG, "GPIO_RECIEVED: %d", gpio_received);
					break;	
			}
		}
		
	}
}

void esp_ui_init() {
	u8g2_esp32_hal_t u8g2_esp32_hal = U8G2_ESP32_HAL_DEFAULT;
	u8g2_esp32_hal.bus.i2c.sda = PIN_SDA;
	u8g2_esp32_hal.bus.i2c.scl = PIN_SCL;
	u8g2_esp32_hal_init(u8g2_esp32_hal);


    u8g2_Setup_sh1107_i2c_128x128_f(&u8g2, U8G2_R0,
      u8g2_esp32_i2c_byte_cb,
      u8g2_esp32_gpio_and_delay_cb);   

  
	u8x8_SetI2CAddress(&u8g2.u8x8, 0x78);  // The address is left-shifted 1 bit
  	ESP_LOGI(OLED_TAG, "u8g2_InitDisplay");
  	u8g2_InitDisplay(&u8g2); 

  	ESP_LOGI(OLED_TAG, "u8g2_SetPowerSave");
  	u8g2_SetPowerSave(&u8g2, 0); 
  	ESP_LOGI(OLED_TAG, "u8g2_ClearBuffer");
  	u8g2_ClearBuffer(&u8g2);
	u8g2_ClearDisplay(&u8g2);
 	u8g2_SetFont(&u8g2 ,u8g2_font_helvR08_tr);

	esp_rom_gpio_pad_select_gpio(UP_BUTTON_GPIO);
	esp_rom_gpio_pad_select_gpio(ENTER_BUTTON_GPIO);
	esp_rom_gpio_pad_select_gpio(DOWN_BUTTON_GPIO);

	gpio_set_direction(UP_BUTTON_GPIO, GPIO_MODE_INPUT);
	gpio_set_direction(ENTER_BUTTON_GPIO, GPIO_MODE_INPUT);
	gpio_set_direction(DOWN_BUTTON_GPIO, GPIO_MODE_INPUT);
	
	gpio_pulldown_en(UP_BUTTON_GPIO);
	gpio_pulldown_en(ENTER_BUTTON_GPIO);
	gpio_pulldown_en(DOWN_BUTTON_GPIO);

	gpio_pullup_dis(UP_BUTTON_GPIO);
	gpio_pullup_dis(ENTER_BUTTON_GPIO);
	gpio_pullup_dis(DOWN_BUTTON_GPIO);

	gpio_set_intr_type(UP_BUTTON_GPIO, GPIO_INTR_POSEDGE);
	gpio_set_intr_type(ENTER_BUTTON_GPIO, GPIO_INTR_POSEDGE);
	gpio_set_intr_type(DOWN_BUTTON_GPIO, GPIO_INTR_POSEDGE);

	gpio_install_isr_service(0);
	gpio_isr_handler_add(UP_BUTTON_GPIO, on_up_button_intr, NULL);
	gpio_isr_handler_add(DOWN_BUTTON_GPIO, on_down_button_intr, NULL);
	gpio_isr_handler_add(ENTER_BUTTON_GPIO, on_enter_button_intr, NULL);

	gpio_intr_enable(UP_BUTTON_GPIO);
	gpio_intr_enable(ENTER_BUTTON_GPIO);
	gpio_intr_enable(DOWN_BUTTON_GPIO);

	buttonQueue = xQueueCreate(10, sizeof(int));
	
	current_page = &home_page;
	xTaskCreatePinnedToCore(display_page, "display_page", 4096, current_page,10, NULL, 0);

	ESP_LOGI(BUTTON_TAG, "about to call ButtonTask");
	ButtonTask();
	
}
