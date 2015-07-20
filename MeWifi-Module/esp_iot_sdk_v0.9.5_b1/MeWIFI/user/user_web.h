#ifndef __USER_WEBSERVER_H__
#define __USER_WEBSERVER_H__

#define SERVER_PORT 80

#define URLSize 10

typedef enum Result_Resp {
    RespFail = 0,
    RespSuc,
} Result_Resp;

typedef enum ProtocolType {
    GET = 0,
    POST,
} ProtocolType;

typedef enum _ParmType {
    SWITCH_STATUS = 0,
    INFOMATION,
    WIFI,
    SCAN,
	REBOOT,
    DEEP_SLEEP,
    LIGHT_STATUS,
    CONNECT_STATUS
} ParmType;

typedef struct URL_Frame {
    enum ProtocolType Type;
    char pCommand[URLSize*20];
} URL_Frame;

typedef struct _rst_parm {
    ParmType parmtype;
    struct espconn *pespconn;
} rst_parm;

void user_web_init(void);

#endif

