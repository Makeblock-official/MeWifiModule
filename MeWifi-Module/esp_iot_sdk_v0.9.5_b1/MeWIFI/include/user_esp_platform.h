#ifndef __USER_DEVICE_H__
#define __USER_DEVICE_H__

/* NOTICE---this is for 512KB spi flash.
 * you can change to other sector if you use other size spi flash. */
#define ESP_PARAM_START_SEC		0x3C

#define ESP_PARAM_SAVE_0    1
#define ESP_PARAM_SAVE_1    2
#define ESP_PARAM_FLAG      3

typedef struct {
    uint32 sign;
    uint32 baud_rate;
	uint32 port;
}USER_PARAM;

#endif
