#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "gpio.h"
#include "driver/uart.h"
#include "mem.h"
#include "user_interface.h"


#define LINK_LED_IO_MUX     PERIPHS_IO_MUX_MTDI_U
#define LINK_LED_IO_NUM     12
#define LINK_LED_IO_FUNC    FUNC_GPIO12

#define LINK_RESTORE_IO_MUX     PERIPHS_IO_MUX_GPIO0_U
#define LINK_RESTORE_IO_NUM     0
#define LINK_RESTORE_IO_FUNC    FUNC_GPIO0


extern unsigned int remoteOnline;

LOCAL os_timer_t link_led_timer;
LOCAL uint8 link_led_level = 0;
LOCAL uint32 link_start_time;

int restore_cnt=0;
char dbg[64];

LOCAL void user_link_led_init(void)
{
    PIN_FUNC_SELECT(LINK_LED_IO_MUX, LINK_LED_IO_FUNC);	
    PIN_FUNC_SELECT(LINK_RESTORE_IO_MUX, LINK_RESTORE_IO_FUNC);	
}

LOCAL void user_link_led_timer_cb(void)
{
	int gin;	
	if(remoteOnline==0){
    	link_led_level = (~link_led_level) & 0x01;
	}else{
		link_led_level = 1;
		remoteOnline--;
	}
	GPIO_OUTPUT_SET(GPIO_ID_PIN(LINK_LED_IO_NUM), link_led_level);
	gin = GPIO_INPUT_GET(GPIO_ID_PIN(LINK_RESTORE_IO_NUM));
	//os_sprintf(dbg,"io0=%d\r\n",gin);
	//uart0_tx_buffer(dbg,32);
	if(gin==0){
		restore_cnt++;
		if(restore_cnt==4){
			user_reset_all();
			os_timer_disarm(&link_led_timer);
		    os_timer_setfn(&link_led_timer, (os_timer_func_t *)user_link_led_timer_cb, NULL);
		    os_timer_arm(&link_led_timer, 100, 1);
		}
	}else{
		restore_cnt = 0;
	}
	
}

LOCAL void user_link_led_timer_init(void)
{
    link_start_time = system_get_time();

    os_timer_disarm(&link_led_timer);
    os_timer_setfn(&link_led_timer, (os_timer_func_t *)user_link_led_timer_cb, NULL);
    os_timer_arm(&link_led_timer, 1000, 1);
    link_led_level = 0;
    GPIO_OUTPUT_SET(GPIO_ID_PIN(LINK_LED_IO_NUM), link_led_level);
}

LOCAL void user_link_led_timer_done(void)
{
    os_timer_disarm(&link_led_timer);
    GPIO_OUTPUT_SET(GPIO_ID_PIN(LINK_LED_IO_NUM), 0);
}

LOCAL void user_wifi_setup(void)
{
	struct softap_config config;
	char password[33];
	char macaddr[6];
	
	wifi_softap_get_config(&config);
	wifi_get_macaddr(SOFTAP_IF, macaddr);
	/*
	os_memset(config.password, 0, sizeof(config.password));
	os_sprintf(password, "12345678");
	os_memcpy(config.password, password, os_strlen(password));
	config.authmode = AUTH_OPEN;
	
	wifi_softap_set_config(&config);
	
	wifi_set_opmode(STATIONAP_MODE);
	*/
}

void user_wifi_init(void)
{
	user_link_led_init();
	user_link_led_timer_init();
	user_wifi_setup();
}










