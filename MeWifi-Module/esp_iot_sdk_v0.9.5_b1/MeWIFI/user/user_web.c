#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "mem.h"
#include "user_interface.h"
#include <string.h>
#include "espconn.h"
#include "user_web.h"
#include "user_esp_platform.h"

LOCAL struct station_config *sta_conf;
LOCAL struct softap_config *ap_conf;

//LOCAL struct secrty_server_info *sec_server;
//LOCAL struct upgrade_server_info *server;
//struct lewei_login_info *login_info;

extern u16 scannum;
//extern at_stateType  at_state;
//extern BOOL specialAtState;
extern BOOL IPMODE;
extern BOOL at_ipMux;
//extern struct user_params myparam;
extern struct espconn *pUdpServer;


const static char web_temp[]="<!DOCTYPE html>"
"<html>"
"<script type=\"text/javascript\">"
"function frm_goto(obj){"
"    var iframe = document.getElementById(\"infrm\");"
"        switch(obj.id){"
"                case \"gMain\":"
"                        NavUrl = \"status.html\";"
"                        break;"
"                case \"gAP\":"
"                        NavUrl = \"serial.html\";"
"                        break;"
"                case \"gStation\":"
"                        NavUrl = \"join.html\";"
"                        break;"
"         } "
"        iframe.src = NavUrl; "
"        return false;"
"} "
"</script>"
"<body>"
"<table width=\"800\" border=\"0\">"
"<tr>"
"<td colspan=\"2\" style=\"background-color:#99bbbb;\">"
"<h1>Makeblock Wifi Serial</h1>"
"</td>"
"</tr>"
"<tr valign=\"top\">"
"<td style=\"background-color:#ffff99;width:100px;text-align:top;\">"
"<b>Menu</b><br />"
"<A id=\"gMain\" href=\"javascript:frm_goto();\" onClick=\"frm_goto(this);return false;\">Wifi Setup</A><br />"
"<A id=\"gAP\" href=\"javascript:frm_goto();\" onClick=\"frm_goto(this);return false;\">Serial Setup</A><br />"
"<A id=\"gStation\" href=\"javascript:frm_goto();\" onClick=\"frm_goto(this);return false;\">Join Setup</A><br />"
"<td style=\"background-color:#EEEEEE;height:600px;width:400px;text-align:top;\">"
"<iframe name=\"infrm\" id=\"infrm\" style=\"width:100%;height:100%;\" src=\"status.html\" frameborder=\"0\"></iframe>"
"</td>"
"</tr>"
"<tr>"
"<td colspan=\"2\" style=\"background-color:#99bbbb;text-align:center;\">"
"info@makeblock.cc</td>"
"</tr>"
"</table>"
"</body>"
"</html>";

const static char web_status[]="<html>"
"<body>"
"<center>"
"<table width=\"400\" border=\"0\">"
"<form name=\"input\" action=\"wifisetup\" method=\"get\">"
"<tr>"
"    <td width=\"100\">wifi mode:</td>"
"    <td><input name=\"wifimode\" type=\"radio\" value=\"1\" %s>Station&nbsp;&nbsp;<input name=\"wifimode\" type=\"radio\" value=\"2\" %s>softAp&nbsp;&nbsp;<input name=\"wifimode\" type=\"radio\" value=\"3\" %s>Both</td>"
"</tr>"
"<tr>"
"    <td width=\"100\">AP SSID:</td>"
"    <td><input type=\"text\" name=\"apname\" value=\"%s\"/></td>"
"</tr>"
"<tr>"
"    <td width=\"100\">PASSWORD:</td>"
"    <td><input type=\"text\" name=\"pass\" value=\"%s\"/></td>"
"</tr>"
"<tr>"
"    <td width=\"100\">CHANNEL:</td>"
"    <td><input type=\"text\" name=\"ch\" value=\"%d\"/></td>"
"</tr>"
"<tr>"
"    <td width=\"100\">AUTH MODE:</td>"
"    <td><input name=\"authmode\" type=\"radio\" value=\"0\" %s>OPEN&nbsp;&nbsp;<input name=\"authmode\" type=\"radio\" value=\"1\" %s>WEP&nbsp;&nbsp;<input name=\"authmode\" type=\"radio\" value=\"2\" %s>WPA2</td>"
"</tr>"
"<tr>"
"    <td width=\"100\">STA SSID:</td>"
"	 <td>%s:%d.%d.%d.%d\r\n</td>"
"</tr>"
"<tr>"
"    <td width=\"100\"><input type=\"submit\" value=\"Submit\" /></td>"
"</tr>"
"</form>"
"</table>"
"</center>"
"</body>"
"</html>";

const static char serial_status[]="<html>"
"<body>"
"<center>"
"<table width=\"400\" border=\"0\">"
"<form name=\"input\" action=\"sersetup\" method=\"get\">"
"<tr>"
"    <td width=\"100\">Baudrate:</td>"
"    <td><input type=\"text\" name=\"baud\" value=\"%d\"/></td>"
"</tr>"
//"<tr>"
//"    <td width=\"100\">AT Status:</td>"
//"    <td>%d</td>"
//"</tr>"
"<tr>"
"<td width=\"100\">PORT:</td>"
"<td><input type=\"text\" name=\"port\" value=\"%d\"/></td>"
"</tr>"
"<tr>"
"	 <td width=\"100\">REMOTE:</td>"
"	 <td>%d.%d.%d.%d:%d\r\n</td>"
"</tr>"
"<tr>"
"    <td width=\"100\"><input type=\"submit\" value=\"Submit\" value=\"4\"/></td>"
"</tr>"
//"<tr>"
//"	 <td width=\"100\">Link List:</td>"
//"	 <td>%s</td>"
//"</tr>"
"</form>"
"</table>"
"</center>"
"</body>"
"</html>";

const static char join_status[]="<html>"
"<body>"
"<center>"
"<table width=\"400\" border=\"0\">"
"<form name=\"input\" action=\"joinap\" method=\"get\">"
"<tr>"
"    <td width=\"100\">AP:</td>"
"	 <td><input type=\"text\" name=\"ap\" /></td>"
"</tr>"
"<tr>"
"    <td width=\"100\">PASSWORD:</td>"
"	 <td><input type=\"text\" name=\"pass\" /></td>"
"</tr>"
"<tr>"
"    <td width=\"100\"><input type=\"submit\" value=\"Join\" value=\"4\"/></td>"
"</tr>"
"</form>"
"<tr>"
"	 <td width=\"100\">AP List:</td>"
"	 <td>%s</td>"
"</tr>"
"</table>"
"</center>"
"</body>"
"</html>";

static char scan_result[512];

/**** string replace ****/
// You must free the result if result is non-NULL.
char *str_replace(char *orig, char *rep, char *with) {
    char *result; // the return string
    char *ins;    // the next insert point
    char *tmp;    // varies
    int len_rep;  // length of rep
    int len_with; // length of with
    int len_front; // distance between rep and end of last rep
    int count;    // number of replacements

    if (!orig)
        return NULL;
    if (!rep)
        rep = "";
    len_rep = os_strlen(rep);
    if (!with)
        with = "";
    len_with = os_strlen(with);

    ins = (char*)orig;
    for (count = 0; tmp = (char*)os_strstr(ins, rep); ++count) {
        ins = tmp + len_rep;
    }

    // first time through the loop, all the variable are set correctly
    // from here on,
    //    tmp points to the end of the result string
    //    ins points to the next occurrence of rep in orig
    //    orig points to the remainder of orig after "end of rep"
    tmp = result = (char*)os_zalloc(os_strlen(orig) + (len_with - len_rep) * count + 1);

    if (!result)
        return NULL;

    while (count--) {
        ins = (char*)os_strstr(orig, rep);
        len_front = ins - orig;
        tmp = (char*)os_strncpy(tmp, orig, len_front) + len_front;
        tmp = (char*)os_strcpy(tmp, with) + len_with;
        orig += len_front + len_rep; // move to next "end of rep"
    }
    os_strcpy(tmp, orig);
    return result;
}


/**** get wifi status ****/
void web_getWifiStatue(char * pbuf)
{
	int len=0;	
	uint8_t at_wifiMode;
	struct softap_config apConfig;
	struct station_config stationConf;
	struct ip_info pTempIp;
  
	at_wifiMode = wifi_get_opmode();	
	wifi_softap_get_config(&apConfig);
	wifi_station_get_config(&stationConf);
	wifi_get_ip_info(0x00, &pTempIp);

	char * sta_check="";
	char * ap_check="";
	char * staap_check="";

	char * open_check="";
	char * wep_check="";
	char * wpa_check="";
	
	if(at_wifiMode==STATION_MODE){
		sta_check = "checked";
	}else if(at_wifiMode==SOFTAP_MODE){
		ap_check = "checked";
	}else if(at_wifiMode==STATIONAP_MODE){
		staap_check = "checked";
	}

	if(apConfig.authmode==AUTH_OPEN){
		open_check = "checked";
	}else if(apConfig.authmode==AUTH_WEP){
		wep_check = "checked";
	}else{
		wpa_check = "checked";
	}

	os_sprintf(pbuf,web_status,
				sta_check, ap_check, staap_check,
				apConfig.ssid,
				apConfig.password,
				apConfig.channel,
				open_check,wep_check,wpa_check,
				stationConf.ssid,IP2STR(&pTempIp.ip));
}


/**** get serial status *****/
extern struct espconn ptresp_serial;
extern USER_PARAM myParam;
void web_getSerialStatus(char * pbuf)
{
	int state;
	os_sprintf(pbuf,serial_status,myParam.baud_rate,
					myParam.port,IP2STR(ptresp_serial.proto.udp->remote_ip),ptresp_serial.proto.udp->remote_port);
}

/**** parse serial setup ****/
extern void user_setup_all(int baud, int port);
int web_parseSerialCommand(char * cmd)
{
	int len;
	char dbg[64];
	char * tmp;
	char * str;
	char * eq;
	
	strtok_r(cmd, "?", &tmp);
	// baud
	str = strtok_r(0, "&", &tmp);
	eq = strstr(str,"=");
	int baud = atoi(eq+1);
	//os_sprintf(dbg,"baud=%d\n",baud);	uart0_sendStr(dbg);
	// port
	eq = strstr(tmp,"=");
	int port = atoi(eq+1);
	//os_sprintf(dbg,"port=%d\n",port);	uart0_sendStr(dbg);
	user_setup_all(baud,port);
	return 0;
}

/**** parse wifi setup command ****/
int web_parseWifiCommand(char * cmd)
{
	int len;
	char dbg[64];
	char * tmp;
	char * str;
	char * eq;
	struct softap_config apConfig;
	wifi_softap_get_config(&apConfig);
	
	strtok_r(cmd, "?", &tmp);
	// wifi mode
	str = strtok_r(0, "&", &tmp);
	eq = strstr(str,"=");
	int wifimode = atoi(eq+1);
	//os_sprintf(dbg,"wifi mode=%d\n",wifimode);	uart0_sendStr(dbg);
    ETS_UART_INTR_DISABLE();
    wifi_set_opmode(wifimode);
    ETS_UART_INTR_ENABLE();
	// ap name
	str = strtok_r(0, "&", &tmp);
	eq = strstr(str,"=");
	char * apname = eq+1;
	//os_sprintf(dbg,"apname=%s\n",apname);	uart0_sendStr(dbg);
	len = os_strlen(apname);
	if(len==0 || len>32) return -1;
	os_memset(apConfig.ssid,0,32);
	os_memcpy(apConfig.ssid, apname, len);
	// pass word
	str = strtok_r(0, "&", &tmp);
	eq = strstr(str,"=");
	char * appass = eq+1;
	//os_sprintf(dbg,"appass=%s\n",appass);	uart0_sendStr(dbg);
	len = os_strlen(appass);
	if(len>64) return -1;	
	os_memset(apConfig.password,0,64);
	os_memcpy(apConfig.password, appass, len);
	// channel
	str = strtok_r(0, "&", &tmp);
	eq = strstr(str,"=");
	int channel = atoi(eq+1);
	//os_sprintf(dbg,"channel=%d\n",channel);	uart0_sendStr(dbg);
	apConfig.channel = channel;
	// auth mode
	eq = strstr(tmp,"=");
	int authmode = atoi(eq+1);
	//os_sprintf(dbg,"authmode=%d\n",authmode);	uart0_sendStr(dbg);
	apConfig.authmode= authmode;

	ETS_UART_INTR_DISABLE();
	wifi_softap_set_config(&apConfig);
	ETS_UART_INTR_ENABLE();
	
	return 0;
}


void web_getJoinStatus(char * pbuf)
{
	os_sprintf(pbuf,join_status,scan_result);
}


int web_parseJoinAp(char * cmd)
{
	int len;
	char dbg[64];
	char * tmp;
	char * str;
	char * eq;
	struct station_config stationConf;	
	wifi_station_get_config(&stationConf);
	strtok_r(cmd, "?", &tmp);
	// ap name
	str = strtok_r(0, "&", &tmp);
	eq = strstr(str,"=");
	char * apname = eq+1;
	len = os_strlen(apname);
	//os_sprintf(dbg,"apname=%s %d\n",apname,len);	uart0_sendStr(dbg);
	if(len==0 || len>32) return -1;
	os_memcpy(stationConf.ssid,apname,len);
	stationConf.ssid[len] = '\0';
	// pass word
	eq = strstr(tmp,"=");
	char * pass = eq+1;
	//os_sprintf(dbg,"pass=%s\n",pass);	uart0_sendStr(dbg);
	len = os_strlen(apname);
	if(len==0 || len>64) return -1;
	os_memcpy(stationConf.password,pass,len);
	
    ETS_UART_INTR_DISABLE();
    wifi_station_set_config(&stationConf);
    ETS_UART_INTR_ENABLE();
    wifi_station_connect();
	return 0;
}

/******************************************************************************
 * FunctionName : data_send
 * Description  : processing the data as http format and send to the client or server
 * Parameters   : arg -- argument to set for client or server
 *                responseOK -- true or false
 *                psend -- The send data
 * Returns      :
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
data_send(void *arg, bool responseOK, char *psend)
{
    uint16 length = 0;
    char *pbuf = NULL;
    char httphead[256];
    struct espconn *ptrespconn = arg;
    os_memset(httphead, 0, 256);

    if (responseOK) {
        os_sprintf(httphead,
                   "HTTP/1.0 200 OK\r\nContent-Length: %d\r\nServer: lwIP/1.4.0\r\n",
                   psend ? os_strlen(psend) : 0);

        if (psend) {
            os_sprintf(httphead + os_strlen(httphead),
					   "Pragma: no-cache\r\n\r\n");
                       //"Content-type: application/json\r\nExpires: Fri, 10 Apr 2008 14:00:00 GMT\r\nPragma: no-cache\r\n\r\n");
            length = os_strlen(httphead) + os_strlen(psend);
            pbuf = (char *)os_zalloc(length + 1);
            os_memcpy(pbuf, httphead, os_strlen(httphead));
            os_memcpy(pbuf + os_strlen(httphead), psend, os_strlen(psend));
        } else {
            os_sprintf(httphead + os_strlen(httphead), "\n");
            length = os_strlen(httphead);
        }
    } else {
        os_sprintf(httphead, "HTTP/1.0 400 BadRequest\r\n\
Content-Length: 0\r\nServer: lwIP/1.4.0\r\n\n");
        length = os_strlen(httphead);
    }

    if (psend) {
#ifdef SERVER_SSL_ENABLE
        espconn_secure_sent(ptrespconn, pbuf, length);
#else
        espconn_sent(ptrespconn, pbuf, length);
#endif
    } else {
#ifdef SERVER_SSL_ENABLE
        espconn_secure_sent(ptrespconn, httphead, length);
#else
        espconn_sent(ptrespconn, httphead, length);
#endif
    }

    if (pbuf) {
        os_free(pbuf);
        pbuf = NULL;
    }
}


/******************************************************************************
 * FunctionName : parse_url
 * Description  : parse the received data from the server
 * Parameters   : precv -- the received data
 *                purl_frame -- the result of parsing the url
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
parse_url(char *precv, URL_Frame *purl_frame)
{
    char *str = NULL;
    uint8 length = 0;
    char *pbuffer = NULL;
    char *pbufer = NULL;

    if (purl_frame == NULL || precv == NULL) {
        return;
    }

    pbuffer = (char *)os_strstr(precv, "Host:");

    if (pbuffer != NULL) {
        length = pbuffer - precv;
        pbufer = (char *)os_zalloc(length + 1);
        pbuffer = pbufer;
        os_memcpy(pbuffer, precv, length);
        os_memset(purl_frame->pCommand, 0, URLSize*10);

        if (os_strncmp(pbuffer, "GET ", 4) == 0) {
            purl_frame->Type = GET;
            pbuffer += 4;			
			pbuffer ++; // skit the '/'
			str = (char *)os_strstr(pbuffer, " HTTP");
			if (str != NULL) {
				length = str - pbuffer;
				os_memcpy(purl_frame->pCommand, pbuffer, length);
			}
        } else if (os_strncmp(pbuffer, "POST ", 5) == 0) {
            purl_frame->Type = POST;
            pbuffer += 5;
        }

        os_free(pbufer);
    } else {
        return;
    }
}


LOCAL char *precvbuffer;
static uint32 dat_sumlength = 0;
LOCAL bool save_data(char *precv, uint16 length)
{
    bool flag = false;
    char length_buf[10] = {0};
    char *ptemp = NULL;
    char *pdata = NULL;
    uint16 headlength = 0;
    static uint32 totallength = 0;

    ptemp = (char *)os_strstr(precv, "\r\n\r\n");

    if (ptemp != NULL) {
        length -= ptemp - precv;
        length -= 4;
        totallength += length;
        headlength = ptemp - precv + 4;
        pdata = (char *)os_strstr(precv, "Content-Length: ");

        if (pdata != NULL) {
            pdata += 16;
            precvbuffer = (char *)os_strstr(pdata, "\r\n");

            if (precvbuffer != NULL) {
                os_memcpy(length_buf, pdata, precvbuffer - pdata);
                dat_sumlength = atoi(length_buf);
            }
        } else {
        	if (totallength != 0x00){
        		totallength = 0;
        		dat_sumlength = 0;
        		return false;
        	}
        }

        precvbuffer = (char *)os_zalloc(dat_sumlength + headlength + 1);
        os_memcpy(precvbuffer, precv, os_strlen(precv));
    } else {
        if (precvbuffer != NULL) {
            totallength += length;
            os_memcpy(precvbuffer + os_strlen(precvbuffer), precv, length);
        } else {
            totallength = 0;
            dat_sumlength = 0;
            return false;
        }
    }

    if (totallength == dat_sumlength) {
        totallength = 0;
        dat_sumlength = 0;
        return true;
    } else {
        return false;
    }
}


/******************************************************************************
 * FunctionName : response_send
 * Description  : processing the send result
 * Parameters   : arg -- argument to set for client or server
 *                responseOK --  true or false
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
response_send(void *arg, bool responseOK)
{
    struct espconn *ptrespconn = arg;

    data_send(ptrespconn, responseOK, NULL);
}



/**** scan all ap *********/
bool scan_fin = true;
static void ICACHE_FLASH_ATTR
web_scan_done(void *arg, STATUS status)
{
  uint8 ssid[33];
  char temp[64];
  int len=0;
  	//uart0_sendStr("#0\n");
  	os_memset(scan_result,0,512);
	if (status == OK)
	  {
		struct bss_info *bss_link = (struct bss_info *)arg;
		//os_sprintf(temp,"#1 %x\r\n",bss_link);
		//uart0_sendStr(temp);
		while (bss_link != NULL)
		{
		  //uart0_sendStr("#2\n");
		  os_memset(ssid, 0, 33);
		  if (os_strlen(bss_link->ssid) <= 32)
		  {
			os_memcpy(ssid, bss_link->ssid, os_strlen(bss_link->ssid));
		  }
		  else
		  {
			os_memcpy(ssid, bss_link->ssid, 32);
		  }
		  //uart0_sendStr("#3\n");
		  if(bss_link->authmode!=0){
		  os_sprintf(temp,"%d,\"%s\",%d<br/>",
					 bss_link->authmode, ssid, bss_link->rssi);
		  //uart0_sendStr(temp);
		  os_strcat(scan_result, temp);
		  if(len++>16) return;
		  }
		  bss_link = bss_link->next.stqe_next;
		}
	  }
	  //specialAtState = TRUE;
	  //at_state = at_statIdle;
	  scan_fin = true;	
}



/******************************************************************************
 * FunctionName : webserver_recv
 * Description  : Processing the received data from the server
 * Parameters   : arg -- Additional argument to pass to the callback function
 *                pusrdata -- The received data (or NULL when the connection has been closed!)
 *                length -- The length of received data
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
webserver_recv(void *arg, char *pusrdata, unsigned short length)
{
	char temp[64];
    URL_Frame *pURL_Frame = NULL;
    char *pParseBuffer = NULL;
    bool parse_flag = false;	
    char *pbuf = NULL;
	pbuf = (char *)os_zalloc(4*1024);
	
    struct espconn *ptrespconn = arg;
    parse_flag = save_data(pusrdata, length);

    do {
        if (parse_flag == false) {
        	response_send(ptrespconn, false);
        	if (dat_sumlength == 0){
        		if (precvbuffer != NULL){
        			os_free(precvbuffer);
        			precvbuffer = NULL;
        		}
        	}
            break;
        }

//        os_printf(precvbuffer);
        pURL_Frame = (URL_Frame *)os_zalloc(sizeof(URL_Frame));
        parse_url(precvbuffer, pURL_Frame);
		//uart0_sendStr(pURL_Frame->pCommand);
        switch (pURL_Frame->Type) {
            case GET:
                os_printf("We have a GET request.\n");
				if(os_strncmp(pURL_Frame->pCommand, "status.html", 11) == 0){
					//os_sprintf(pbuf,"%s\r\n",pURL_Frame->pCommand);
					web_getWifiStatue(pbuf);
					data_send(ptrespconn, true, pbuf);
				}else if(os_strncmp(pURL_Frame->pCommand, "serial.html", 11) == 0){
					web_getSerialStatus(pbuf);
					data_send(ptrespconn, true, pbuf);
				}else if(os_strncmp(pURL_Frame->pCommand, "join.html", 9) == 0){
					web_getJoinStatus(pbuf);
					data_send(ptrespconn, true, pbuf);
				}else if(os_strncmp(pURL_Frame->pCommand, "wifisetup", 9) == 0){
					int ret = web_parseWifiCommand(pURL_Frame->pCommand);
					if(ret==0){
						data_send(ptrespconn, true, "config accepted");
					}else{
						data_send(ptrespconn, true, "config error");
					}
				}else if(os_strncmp(pURL_Frame->pCommand, "sersetup", 8) == 0){
					int ret = web_parseSerialCommand(pURL_Frame->pCommand);
					data_send(ptrespconn, true, "config accepted");
				}else if(os_strncmp(pURL_Frame->pCommand, "joinap", 6) == 0){
					int ret = web_parseJoinAp(pURL_Frame->pCommand);
					data_send(ptrespconn, true, "config accepted");
				}else if(os_strlen(pURL_Frame->pCommand)<3){
					os_strcpy(pbuf, web_temp);
					data_send(ptrespconn, true, pbuf);
					if(scan_fin==true){
						uint8 ap_node = wifi_get_opmode();
						scan_fin = false;
						if(ap_node!=SOFTAP_MODE){
							wifi_station_scan(NULL, web_scan_done);						
							//specialAtState = FALSE;
						}
					}
				}else{
					data_send(ptrespconn, true, "unknow");
				}
                break;
        }

        if (precvbuffer != NULL){
        	os_free(precvbuffer);
        	precvbuffer = NULL;
        }
        os_free(pURL_Frame);
        pURL_Frame = NULL;
	    

    } while (0);
	os_free(pbuf);
	pbuf = NULL;

}



LOCAL ICACHE_FLASH_ATTR
void webserver_recon(void *arg, sint8 err)
{
    struct espconn *pesp_conn = arg;

    os_printf("webserver's %d.%d.%d.%d:%d err %d reconnect\n", pesp_conn->proto.tcp->remote_ip[0],
    		pesp_conn->proto.tcp->remote_ip[1],pesp_conn->proto.tcp->remote_ip[2],
    		pesp_conn->proto.tcp->remote_ip[3],pesp_conn->proto.tcp->remote_port, err);
}

LOCAL ICACHE_FLASH_ATTR
void webserver_discon(void *arg)
{
    struct espconn *pesp_conn = arg;

    os_printf("webserver's %d.%d.%d.%d:%d disconnect\n", pesp_conn->proto.tcp->remote_ip[0],
        		pesp_conn->proto.tcp->remote_ip[1],pesp_conn->proto.tcp->remote_ip[2],
        		pesp_conn->proto.tcp->remote_ip[3],pesp_conn->proto.tcp->remote_port);
}


LOCAL void ICACHE_FLASH_ATTR
webserver_listen(void *arg)
{
    struct espconn *pesp_conn = arg;

    espconn_regist_recvcb(pesp_conn, webserver_recv);
    espconn_regist_reconcb(pesp_conn, webserver_recon);
    espconn_regist_disconcb(pesp_conn, webserver_discon);
}

void user_web_init(void)
{
	LOCAL struct espconn esp_conn;
    LOCAL esp_tcp esptcp;

    esp_conn.type = ESPCONN_TCP;
    esp_conn.state = ESPCONN_NONE;
    esp_conn.proto.tcp = &esptcp;
    esp_conn.proto.tcp->local_port = 80;
    espconn_regist_connectcb(&esp_conn, webserver_listen);

    espconn_accept(&esp_conn);
}










