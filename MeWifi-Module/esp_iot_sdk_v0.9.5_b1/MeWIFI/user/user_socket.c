
#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "mem.h"
#include "user_interface.h"
#include "driver/uart.h"
#include "espconn.h"

LOCAL struct espconn ptresp_probe;
struct espconn ptresp_serial;

LOCAL os_timer_t tx_timer;

int rxIndex=0;
char rxBuf[512];
unsigned int remoteOnline=0;

LOCAL void ICACHE_FLASH_ATTR
rx_timer_cb(void)
{
	if(remoteOnline){
		espconn_sent(&ptresp_serial, rxBuf, rxIndex);		
		remoteOnline = 30;
	}
	//uart0_tx_buffer(rxBuf, rxIndex);
	rxIndex = 0;
}

void rxPushChar(char c)
{
	if(rxIndex==0) os_timer_arm(&tx_timer, 20, 0);
	rxBuf[rxIndex++] = c;
}

LOCAL void ICACHE_FLASH_ATTR
user_socket_serial_rx(void *arg, char *pusrdata, unsigned short length)
{
    char DeviceBuffer[40] = {0};
    char Device_mac_buffer[60] = {0};
    char hwaddr[6];

    if (pusrdata == NULL) {
        return;
    }
	remoteOnline = 30;
	uart0_tx_buffer(pusrdata,length);
	//espconn_sent(&ptresp_serial, pusrdata, length);
}

LOCAL void ICACHE_FLASH_ATTR
user_socket_probe_rx(void *arg, char *pusrdata, unsigned short length)
{
	struct softap_config apConfig;
    char DeviceBuffer[40] = {0};
    char Device_mac_buffer[60] = {0};
    char hwaddr[6];

    if (pusrdata == NULL) {
        return;
    }	
	wifi_softap_get_config(&apConfig);
	espconn_sent(&ptresp_probe, apConfig.ssid, os_strlen(apConfig.ssid));
}


void user_socket_init(int rxPort)
{
	ptresp_probe.type = ESPCONN_UDP;
	ptresp_probe.proto.udp = (esp_udp *)os_zalloc(sizeof(esp_udp));
	ptresp_probe.proto.udp->local_port = 333;
	espconn_regist_recvcb(&ptresp_probe, user_socket_probe_rx);
	espconn_create(&ptresp_probe);

	ptresp_serial.type = ESPCONN_UDP;
	ptresp_serial.proto.udp = (esp_udp *)os_zalloc(sizeof(esp_udp));
	ptresp_serial.proto.udp->local_port = rxPort;
	espconn_regist_recvcb(&ptresp_serial, user_socket_serial_rx);
	espconn_create(&ptresp_serial);

	os_timer_disarm(&tx_timer);
	os_timer_setfn(&tx_timer, (os_timer_func_t *)rx_timer_cb, NULL);
	//os_timer_arm(&link_led_timer, 1000, 1);	
}





