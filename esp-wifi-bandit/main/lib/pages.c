#include <stdio.h>
#include "deauth-sender.h"
#include "esp_ui.h"
#include "evil-twin.h"
#include <u8g2.h>
#include "u8g2_esp32_hal.h"

// home page


TextLine home_processes_textline = {
	.position = 7,
	.option = false,
	.next_line = NULL,
	.process_on_enter = NO_PROCESS,
	.page_on_enter = NULL
};

TextLine home_evil_twin_textline = {
	.position = 1,
	.text = "Evil-twin Attack",
	.option = true,
	.next_line = &home_processes_textline,
	.process_on_enter = NO_PROCESS,
	.page_on_enter = NULL
};

TextLine home_deauth_textline = {
	.position = 0,
	.text = "Deauth Attack",
	.option = true,
	.next_line = &home_evil_twin_textline,
	.process_on_enter = NO_PROCESS,
	.page_on_enter = NULL
};

Page home_page = {
	.name = "Home Page",
	.head_textline = &home_deauth_textline,
	.selected_line = &home_deauth_textline
};


// deauth start page

TextLine back_to_home_textline = {
	.position = 7,
	.text = "Back",
	.option = true,
	.next_line = NULL,
	.process_on_enter = NO_PROCESS,
	.page_on_enter = &home_page
};

TextLine deauth_start_textline = {
	.position = 0,
	.text = "Start Deauther",
	.option = true,
	.next_line = &back_to_home_textline,
	.process_on_enter = DEAUTH_START,
	.page_on_enter = NULL
};

Page deauth_start_page = {
	.name = "Death Start Page",
	.head_textline = &deauth_start_textline,
	.selected_line = &deauth_start_textline
};

// deauth_stop_page

TextLine deauth_stop_textline = {
	.position = 0,
	.text = "Stop Deauther",
	.option = true,
	.next_line = &back_to_home_textline,
	.process_on_enter = DEAUTH_STOP,
	.page_on_enter = &deauth_start_page
};

Page deauth_stop_page = {
	.name = "deauth_stop_page",
	.head_textline = &deauth_stop_textline,
	.selected_line = &deauth_stop_textline
};


// evil twin start page

TextLine evil_twin_start_textline = {
	.position = 0,
	.text = "Start Evil-Twin",
	.option = true,
	.next_line = &back_to_home_textline,
	.process_on_enter = SCANNER,
	.page_on_enter = NULL,
};

Page evil_twin_start_page = {
	.name = "evil_twin_start_page",
	.head_textline = &evil_twin_start_textline,
	.selected_line = &evil_twin_start_textline,
};

// evil twin stop page

TextLine evil_twin_stop_textline = {
	.position = 0,
	.text = "Stop Evil-Twin",
	.option = true,
	.next_line = &back_to_home_textline,
	.process_on_enter = EVIL_TWIN_STOP,
	.page_on_enter = &evil_twin_start_page
};

Page evil_twin_stop_page = {
	.name = "evil_twin_stop_page",
	.head_textline = &evil_twin_stop_textline,
	.selected_line = &evil_twin_stop_textline
};



// clone ap page

TextLine clone_ap_textline_7 = {
	.position = 7,
	.option = true,
	.next_line = NULL,
	.page_on_enter = &evil_twin_stop_page,
	.process_on_enter = EVIL_TWIN_START
};

TextLine clone_ap_textline_6 = {
	.position = 6,
	.option = true,
	.next_line = &clone_ap_textline_7,
	.page_on_enter = &evil_twin_stop_page,
	.process_on_enter = EVIL_TWIN_START
};

TextLine clone_ap_textline_5 = {
	.position = 5,
	.option = true,
	.next_line = &clone_ap_textline_6,
	.page_on_enter = &evil_twin_stop_page,
	.process_on_enter = EVIL_TWIN_START
};

TextLine clone_ap_textline_4 = {
	.position = 4,
	.option = true,
	.next_line = &clone_ap_textline_5,
	.page_on_enter = &evil_twin_stop_page,
	.process_on_enter = EVIL_TWIN_START
};

TextLine clone_ap_textline_3 = {
	.position = 3,
	.option = true,
	.next_line = &clone_ap_textline_4,
	.page_on_enter = &evil_twin_stop_page,
	.process_on_enter = EVIL_TWIN_START
};

TextLine clone_ap_textline_2 = {
	.position = 2,
	.option = true,
	.next_line = &clone_ap_textline_3,
	.page_on_enter = &evil_twin_stop_page,
	.process_on_enter = EVIL_TWIN_START
};

TextLine clone_ap_textline_1 = {
	.position = 1,
	.option = true,
	.next_line = &clone_ap_textline_2,
	.page_on_enter = &evil_twin_stop_page,
	.process_on_enter = EVIL_TWIN_START
};

TextLine num_ap_scanned_textline = {
	.position = 0,
	.option = false,
	.text = "Choose an AP:",
	.next_line = &clone_ap_textline_1,
	.page_on_enter = NULL,
	.process_on_enter = NO_PROCESS
};

Page clone_ap_page = {
	.name = "clone_ap_page",
	.head_textline = &num_ap_scanned_textline,
	.selected_line = &num_ap_scanned_textline
};

// scanning page (loading screen for clone ap)

TextLine scanning_textline = {
	.position = 0,
	.option = true,
	.next_line = NULL,
	.page_on_enter = NULL,
	.process_on_enter = NO_PROCESS
};

Page scanning_page = {
	.name = "scanning_page",
	.head_textline = &scanning_textline,
	.selected_line = NULL
};

void init_pages (){
	
	deauth_start_textline.page_on_enter = &deauth_stop_page;
	evil_twin_start_textline.page_on_enter = &scanning_page;	
	current_page = &home_page;
};

