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

#define PIN_SDA 21
#define PIN_SCL 22
#define ORIGIN_X 40
#define ORIGIN_Y 15
#define OPTION_PADDING 15
#define OPTION_BACK_Y 120
/*
#define UP_BUTTON_GPIO 12
#define DOWN_BUTTON_GPIO 14
#define ENTER_BUTTON_GPIO 27
*/
#define MAX_DISPLAY_LINES 8

static const char* OLED_TAG = "sh1107";
static const char* PAGE_TAG = "PAGE";
static const char* BUTTON_TAG = "BUTTON";

static const int UP_BUTTON_GPIO = 12;
static const int DOWN_BUTTON_GPIO = 27;
static const int ENTER_BUTTON_GPIO = 14;

u8g2_t u8g2;
QueueHandle_t buttonQueue; 
TaskHandle_t pushButtonTaskHandle = NULL;
TaskHandle_t buttonTaskHandle = NULL;
bool display_idle = true;



typedef struct  {
	int position;
	char * text;
	bool option; 
	void * next_line;
	void * page_on_enter;
} TextLine;

typedef struct {
	char * name;
	TextLine * head_textline;
	TextLine * selected_line;
	int num_lines;
} Page;

Page *current_page;

void display_page(void* arg){
	display_idle = false;
	Page * page_to_display = (Page *)arg;	
	
	u8g2_ClearBuffer(&u8g2);

	TextLine * current_line = page_to_display->head_textline;	
	while(current_line->next_line != NULL){	
		if (current_line->option == true && current_line == page_to_display->selected_line){
			u8g2_DrawButtonUTF8(&u8g2, ORIGIN_X , ORIGIN_Y + (OPTION_PADDING * current_line->position), U8G2_BTN_BW2, 0,  2,  2, current_line->text);	

		}else if (current_line == page_to_display->selected_line && current_line->option == false){
			ESP_LOGI(PAGE_TAG, "Page %s: Configurations are invalid", page_to_display->name);

		}else{
			u8g2_DrawStr(&u8g2, ORIGIN_X , ORIGIN_Y + (OPTION_PADDING * current_line->position), current_line->text);	
			
		}
		current_line = current_line->next_line;
		u8g2_SendBuffer(&u8g2);

	}
	ESP_LOGI(PAGE_TAG, "Displaying final line :%s", current_line->text );
	if (current_line == page_to_display->selected_line){
		u8g2_DrawButtonUTF8(&u8g2, ORIGIN_X , ORIGIN_Y + (OPTION_PADDING * current_line->position), U8G2_BTN_BW2, 0,  2,  2, current_line->text);		

	}else if (current_line == page_to_display->selected_line && current_line->option == false){
			ESP_LOGI(PAGE_TAG, "Page %s: Configurations are invalid", page_to_display->name);
	
	}else{
		u8g2_DrawStr(&u8g2, ORIGIN_X , ORIGIN_Y + (OPTION_PADDING * current_line->position), current_line->text);	
	}
	u8g2_SendBuffer(&u8g2);
	ESP_LOGI(PAGE_TAG, "Page displayed: %s", page_to_display->name);
	current_page = page_to_display; 
	display_idle = true;
	vTaskDelete(NULL);


}

/*
void select_line(int new_line ){
	current_page->selected_line = new_line;
	display_page(current_page, false);
	
}
*/

static void IRAM_ATTR on_up_button_intr(){
	xQueueSendFromISR(buttonQueue, &UP_BUTTON_GPIO, NULL);
}

static void IRAM_ATTR on_enter_button_intr(){
	xQueueSendFromISR(buttonQueue, &ENTER_BUTTON_GPIO, NULL);
}


static void IRAM_ATTR on_down_button_intr(){
	xQueueSendFromISR(buttonQueue, &DOWN_BUTTON_GPIO, NULL);	
}


void ButtonTask(){
	ESP_LOGI(BUTTON_TAG, "Initializing button task");
	int gpio_received;
	while (1){
		xQueueReceive(buttonQueue, &gpio_received, portMAX_DELAY);
		if (display_idle){	
			switch (gpio_received){
				case DOWN_BUTTON_GPIO:
					TextLine * next_line = current_page->selected_line->next_line;
					if (next_line == NULL){
						break;
					}	
					current_page->selected_line = next_line;
					xTaskCreatePinnedToCore(display_page, "Display Page", 4096, current_page, 10, &pushButtonTaskHandle, 1);
					ESP_LOGI(BUTTON_TAG, "Down button pressed");
					break;
				case UP_BUTTON_GPIO:
					
					if (current_page->selected_line->position == 0){ 
						break;
					}

					TextLine *current_line = current_page->head_textline;
					while(current_line->next_line != current_page->selected_line){
						current_line = current_line->next_line;
					}
					current_page->selected_line = current_line;
					xTaskCreatePinnedToCore(display_page, "Display Page", 4096, current_page, 10, &pushButtonTaskHandle, 1);
					ESP_LOGI(BUTTON_TAG, "Up button pressed");	
					break;

				case ENTER_BUTTON_GPIO:
					if (current_page->selected_line->page_on_enter == NULL){
						break;
					}
					current_page = current_page->selected_line->page_on_enter;	
					xTaskCreatePinnedToCore(display_page, "Display Page", 4096, current_page, 10, &pushButtonTaskHandle, 1);
					break;
					
				default:
					ESP_LOGI(BUTTON_TAG, "GPIO_RECIEVED: %d", gpio_received);
					break;	
			}
		}
		
	}
}

void app_main(void) {
	u8g2_esp32_hal_t u8g2_esp32_hal = U8G2_ESP32_HAL_DEFAULT;
	u8g2_esp32_hal.bus.i2c.sda = PIN_SDA;
	u8g2_esp32_hal.bus.i2c.scl = PIN_SCL;
	u8g2_esp32_hal_init(u8g2_esp32_hal);

	  // this structure which will contain all the data for the display


    u8g2_Setup_sh1107_i2c_128x128_f(&u8g2, U8G2_R0,
      u8g2_esp32_i2c_byte_cb,
      u8g2_esp32_gpio_and_delay_cb);  // These are I2C callback function for mapping 

  
	u8x8_SetI2CAddress(&u8g2.u8x8, 0x78);  // The address is left-shifted 1 bit
  	ESP_LOGI(OLED_TAG, "u8g2_InitDisplay");
  	u8g2_InitDisplay(&u8g2);  // send init sequence to the display, display is in sleep mode after this,

  	ESP_LOGI(OLED_TAG, "u8g2_SetPowerSave");
  	u8g2_SetPowerSave(&u8g2, 0);  // wake up display
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
	
	TextLine deauthenticator_option = {
		.position = 0,
		.text = "Deauthenticator",
		.option = true,
	};
	TextLine evil_twin_option = {
		.position = 1,
		.text = "Evil-twin",
		.option = true,
		.next_line = NULL
	};
	deauthenticator_option.next_line = &evil_twin_option;

	Page home_page = {
		.name = "Home Page",
		.head_textline = &deauthenticator_option,
		.selected_line = &deauthenticator_option
	};
	
	//xTaskCreate()
	xTaskCreate(display_page, "Display Page", 4096, &home_page, 10, &buttonTaskHandle);
	//select_line(&home_page, 1);
	
	//xTaskCreatePinnedToCore(&ButtonTask, "ButtonTask", 4096, NULL, 9, &buttonTaskHandle, 1);
	ESP_LOGI(BUTTON_TAG, "about to call ButtonTask");
	ButtonTask();
	
}
