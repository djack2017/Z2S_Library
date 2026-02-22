#include <Arduino.h>
#include <SuplaDevice.h>
#include <supla/network/esp_wifi.h>
#include "z2s_zabbix.h"
#include <supla/storage/storage.h>

//====================================================================================================
int ZABBIX_Port = 10051;              	 // Zabbix Port
//====================================================================================================

//====================================================================================================
#define debug_ZABBIX 0
//====================================================================================================

byte ZABBIX_Liczba_RX;
byte ZABBIX_D_RX[600];

//====================================================================================================
// Zabbix Sender
//====================================================================================================
void ZABBIX_Sender(void *buf, int count) {

	WiFiClient client;
	byte header[400];
	int r;
    char zabbix[64] = {0};
	
	auto cfg = Supla::Storage::ConfigInstance();
	if (cfg->getString(PSTR("zabbix_server"), zabbix, sizeof(zabbix))) {
		if (strlen(zabbix) > 0) {
			//printf("Nazwa Zabbix: %s\n", zabbix);
		} else {
			printf("Nazwa Serwera Zabbix jest pusta\n");
			return;
		}
	} else {
		printf("Brak klucza zabbix_server w konfiguracji\n");
		return;
	}

//	if(!client.connect(zabbix_host.c_str(), ZABBIX_Port)){
	if(!client.connect(zabbix, ZABBIX_Port)){
		printf("Error connecting to server ZABBIX\n");
		delay(5000);
	} else {
		word timeout = 0;
		printf("Connection to ZABBIX server successfully\n");
		ZABBIX_Liczba_RX = 0;
		//printf("Sender0 count:%d\n",count);
		if (count + 13 > sizeof(header)) {
			printf("ZABBIX ERROR: header buffer too small\n");
			return;
		}	

		header[0]  = 'Z';
		header[1]  = 'B';
		header[2]  = 'X';
		header[3]  = 'D';
		header[4]  = '\1';
		header[5]  = (unsigned char)(count & 0xFF);
		header[6]  = (unsigned char)((count >> 8) & 0xFF);
		header[7]  = '\0';
		header[8]  = '\0';
		header[9]  = '\0';
		header[10] = '\0';
		header[11] = '\0';
		header[12] = '\0';
		for (r = 0; r<count; r ++) header[13+r] = ((byte*)buf)[r];
		count = count+13;

		if(debug_ZABBIX == 1) {
		//printf("Zabbix Tx: ");
		//----------------------------------
		//for (int i=0; i < count; i++) {
		//  printf("0x");
		//  printf(header[i],HEX);
		//  if (i<count) printf(",");
		//}
		//printf(" \n");
		//----------------------------------
		//for (int i=0; i < count; i++) {
		//  printf((char)header[i]);
		//}
		//printf("\n");
		}   

		client.write(&header[0],count); 

	/*    while(timeout < 50000){    // Przerywamy polaczenie jesli timeout wynosi 50000 = 10 sekund
      ZABBIX_Liczba_RX = client.available();
      for (int i=1; i <= ZABBIX_Liczba_RX; i++) {
        ZABBIX_D_RX[i] = client.read();
      }
      if(ZABBIX_Liczba_RX > 0) {
        if(debug_ZABBIX == 1) {
          printf("Zabbix Rx: ");
          for (int i=0; i <= ZABBIX_Liczba_RX; i++){
            printf("%c", ZABBIX_D_RX[i]);
          }
          printf("\n");
        }
        break;
      }
      delayMicroseconds(200);
      ++timeout;
    }*/

		bool flaga_timeout = 0;
		if(timeout >= 50000){ // = 10 sekund
		printf("");
		printf(" TIMEOUT\n");
		flaga_timeout = 1;
		}

		if (ZABBIX_Liczba_RX >= 34) {
			if (memcmp(&ZABBIX_D_RX[27], "success", 7) == 0) {
				printf("Zabbix response: Success\n");
			}
		}
	
//    if (ZABBIX_D_RX[27] == 0x73 && ZABBIX_D_RX[28] == 0x75 && ZABBIX_D_RX[29] == 0x63 && ZABBIX_D_RX[30] == 0x63 && ZABBIX_D_RX[31] == 0x65 && ZABBIX_D_RX[32] == 0x73 && ZABBIX_D_RX[33] == 0x73) { 
//      if(debug_ZABBIX == 1) {
//        printf("Zabbix response: Success\n");
//        printf(" \n");
//      }
//    }
		client.stop();
	}
}

//====================================================================================================
// Wysłanie informacji do serwera ZABBIX (1 parametr)
//====================================================================================================
void zabbix_send(const char *xhostname, const char *item_key, const char *value_key)
{
    if (!xhostname || !item_key || !value_key) {
        printf("Error: NULL argument in zabbix_send\n");
        return;
    }
    char buff[256];
    memset(buff, 0, sizeof(buff));
    int written = snprintf(
        buff, sizeof(buff),
        "{\"request\":\"sender data\",\"data\":[{\"host\":\"%s\",\"key\":\"%s\",\"value\":\"%s\"}]}",
        xhostname, item_key, value_key
		);
    if (written < 0 || written >= sizeof(buff)) {
        printf("Error: JSON buffer too small in zabbix_send\n");
        return;
    }
    ZABBIX_Sender(buff, strlen(buff));
}

//====================================================================================================
// Wysłanie informacji do serwera ZABBIX (2 parametry)
//====================================================================================================
void zabbix_send2(const char *xhostname, const char *item_key1, const char *value_key1, const char *item_key2, const char *value_key2) {

	if (!xhostname || !item_key1 || !value_key1 || !item_key2 || !value_key2) {
        printf("Error: NULL argument in zabbix_send\n");
        return;
	}
	char buff[160];
	memset(buff, 0, sizeof(buff));
    int written = snprintf(
		buff, sizeof(buff),
		"{\"request\":\"sender data\",\"data\":[" \
		"{\"host\":\"%s\",\"key\":\"%s\",\"value\":\"%s\"}," \
		"{\"host\":\"%s\",\"key\":\"%s\",\"value\":\"%s\"}]}", \
		xhostname, item_key1, value_key1, \
		xhostname, item_key2, value_key2);
    if (written < 0 || written >= sizeof(buff)) {
        printf("Error: JSON buffer too small in zabbix_send\n");
        return;
    }
	ZABBIX_Sender(buff, strlen(buff));
}

//====================================================================================================
// Wysłanie informacji do serwera ZABBIX (4 parametry)
//====================================================================================================
void zabbix_send4(const char *xhostname, const char *item_key1, const char *value_key1, const char *item_key2, const char *value_key2, const char *item_key3, const char *value_key3, const char *item_key4, const char *value_key4) {

    if (!xhostname || !item_key1 || !value_key1 || !item_key2 || !value_key2 || !item_key3 || !value_key3 || !item_key4 || !value_key4) {
        printf("Error: NULL argument in zabbix_send\n");
        return;
    }
	char buff[400];
	memset(buff, 0, sizeof(buff));
    int written = snprintf(
		buff, sizeof(buff),
		"{\"request\":\"sender data\",\"data\":[" \
		"{\"host\":\"%s\",\"key\":\"%s\",\"value\":\"%s\"}," \
		"{\"host\":\"%s\",\"key\":\"%s\",\"value\":\"%s\"}," \
		"{\"host\":\"%s\",\"key\":\"%s\",\"value\":\"%s\"}," \
		"{\"host\":\"%s\",\"key\":\"%s\",\"value\":\"%s\"}]}", \
		xhostname, item_key1, value_key1, \
		xhostname, item_key2, value_key2, \
		xhostname, item_key3, value_key3, \
		xhostname, item_key4, value_key4);
    if (written < 0 || written >= sizeof(buff)) {
        printf("Error: JSON buffer too small in zabbix_send\n");
        return;
    }
	ZABBIX_Sender(buff, strlen(buff));
}

//====================================================================================================
// Wysłanie informacji do serwera ZABBIX (6 parametrów)
//====================================================================================================
void zabbix_send6(char *xhostname, char *item_key1, char *value_key1, char *item_key2, char *value_key2, char *item_key3, char *value_key3, char *item_key4, char *value_key4, char *item_key5, char *value_key5, char *item_key6, char *value_key6) {

  char buff[360];
  memset(buff, 0, sizeof(buff));
  sprintf(buff, "{\"request\":\"sender data\",\"data\":[" \
     "{\"host\":\"%s\",\"key\":\"%s\",\"value\":\"%s\"}," \
     "{\"host\":\"%s\",\"key\":\"%s\",\"value\":\"%s\"}," \
     "{\"host\":\"%s\",\"key\":\"%s\",\"value\":\"%s\"}," \
     "{\"host\":\"%s\",\"key\":\"%s\",\"value\":\"%s\"}," \
     "{\"host\":\"%s\",\"key\":\"%s\",\"value\":\"%s\"}," \
     "{\"host\":\"%s\",\"key\":\"%s\",\"value\":\"%s\"}]}", \
     xhostname, item_key1, value_key1, \
     xhostname, item_key2, value_key2, \
     xhostname, item_key3, value_key3, \
     xhostname, item_key4, value_key4, \
     xhostname, item_key5, value_key5, \
     xhostname, item_key6, value_key6);
  ZABBIX_Sender(buff, strlen(buff));
}
