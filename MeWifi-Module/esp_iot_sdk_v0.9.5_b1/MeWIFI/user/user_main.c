/******************************************************************************
 * Copyright 2013-2014 Espressif Systems (Wuxi)
 *
 * FileName: user_main.c
 *
 * Description: entry file of user application
 *
 * Modification history:
 *     2014/1/1, v1.0 create this file.
*******************************************************************************/
#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "gpio.h"
#include "driver/uart.h"
#include "user_wifi.h"
#include "user_socket.h"
#include "user_web.h"
#include "user_esp_platform.h"
#include "user_interface.h"
#include "spi_flash.h"

USER_PARAM myParam;

void ICACHE_FLASH_ATTR
load_userParam(void)
{
	struct softap_config config;
    spi_flash_read((ESP_PARAM_START_SEC + 1) * SPI_FLASH_SEC_SIZE,
                   (uint32 *)&myParam, sizeof(USER_PARAM));

	// default setup
	if(myParam.sign!=4771215){
		myParam.sign = 4771215;
		myParam.baud_rate = BIT_RATE_115200;
		myParam.port = 1025;
		spi_flash_erase_sector(ESP_PARAM_START_SEC + 1);
        spi_flash_write((ESP_PARAM_START_SEC + 1) * SPI_FLASH_SEC_SIZE,
                        (uint32 *)&myParam, sizeof(USER_PARAM));
		wifi_set_opmode(STATIONAP_MODE);
		wifi_softap_get_config(&config);
		
		config.authmode = AUTH_OPEN;
        wifi_softap_set_config(&config);
		
	}
    spi_flash_read((ESP_PARAM_START_SEC + 1) * SPI_FLASH_SEC_SIZE,
                   (uint32 *)&myParam, sizeof(USER_PARAM));
}

void ICACHE_FLASH_ATTR
user_setup_all(int baud, int port)
{
   	myParam.baud_rate = baud;
	myParam.port = port;
	spi_flash_erase_sector(ESP_PARAM_START_SEC + 1);
	spi_flash_write((ESP_PARAM_START_SEC + 1) * SPI_FLASH_SEC_SIZE,
					(uint32 *)&myParam, sizeof(USER_PARAM));
}

void ICACHE_FLASH_ATTR
user_reset_all(void)
{
	spi_flash_erase_sector(ESP_PARAM_START_SEC + 1);
	
}


/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void user_init(void)
{
	load_userParam();
	uart_init(myParam.baud_rate, myParam.baud_rate);
    os_printf("SDK version:%s\n", system_get_sdk_version());
	uart0_sendStr("\r\nready\r\n");  
	user_wifi_init();
	user_socket_init(myParam.port);
	user_web_init();
	
	
}

