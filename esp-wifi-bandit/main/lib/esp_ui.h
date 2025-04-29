#include <u8g2.h>
#include "u8g2_esp32_hal.h"
#include "pages.h"

#ifndef ESP_UI_HEADER
#define ESP_UI_HEADER

#define PIN_SDA 21
#define PIN_SCL 22
#define ORIGIN_X 40
#define ORIGIN_Y 15
#define OPTION_PADDING 15
#define OPTION_BACK_Y 120

extern const char* OLED_TAG;
extern const char* PAGE_TAG;
extern const char* BUTTON_TAG;

extern const int UP_BUTTON_GPIO;
extern const int DOWN_BUTTON_GPIO;
extern const int ENTER_BUTTON_GPIO;
extern int runningTasks;

extern u8g2_t u8g2;
extern QueueHandle_t buttonQueue; 
extern bool display_idle;
extern Page *current_page;

void display_page(void* arg);
void ButtonTask();
void esp_ui_init();

#endif
