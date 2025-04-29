#include <stdio.h>
#include "evil-twin.h"
#include "deauth-sender.h"
#include <u8g2.h>
#include "u8g2_esp32_hal.h"


#ifndef PAGES_HEADER
#define PAGES_HEADER

typedef enum  {
	DEAUTH_START,
	DEAUTH_STOP,
	SCANNER,
	EVIL_TWIN_START,
	EVIL_TWIN_STOP,
	NO_PROCESS
}Process;

typedef struct  {
	int position;
	char * text;
	bool option; 
	void * next_line;
	void * page_on_enter;
	Process process_on_enter;
}TextLine;

typedef struct {
	char * name;
	TextLine * head_textline;
	TextLine * selected_line;
	int num_lines;
}Page;



extern TextLine home_processes_textline;
extern TextLine home_evil_twin_textline;
extern TextLine home_deauth_textline;
extern Page home_page;

extern TextLine back_to_home_textline;
extern TextLine deauth_start_textline;
extern Page deauth_start_page;

extern TextLine deauth_stop_textline;
extern Page deauth_stop_page;

extern TextLine evil_twin_start_textline;
extern Page evil_twin_start_page;

extern TextLine evil_twin_stop_textline;
extern Page evil_twin_stop_page;

extern TextLine clone_ap_textline_7;
extern TextLine clone_ap_textline_6;
extern TextLine clone_ap_textline_5;
extern TextLine clone_ap_textline_4;
extern TextLine clone_ap_textline_3;
extern TextLine clone_ap_textline_2;
extern TextLine clone_ap_textline_1;
extern TextLine num_ap_scanned_textline;
extern Page clone_ap_page;

extern TextLine scanning_textline;
extern Page scanning_page;

void init_pages();

#endif
