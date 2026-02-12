#include <SuplaDevice.h>
#define www_username "admin"
#define www_password "pass"

//====================================================================================================
String ZABBIX_host = "192.168.1.115"; // Adres IP serwera Zabbix
int ZABBIX_Port = 10051;              // Port serwera Zabbix
String ZABBIX_DEVICE = "satel";       // nazwa hosta na serwerze Zabbix
//====================================================================================================

//====================================================================================================
#define debuguj_transmisje_ZABBIX 1
//====================================================================================================

byte ZABBIX_Liczba_RX;
byte ZABBIX_Liczba_TX;
#define ZABBIX_buffor 300
byte ZABBIX_DATA[ZABBIX_buffor];
byte ZABBIX_D_RX[ZABBIX_buffor];
byte ZABBIX_D_TX[ZABBIX_buffor];
word xcode, source;
char status[20];
char sdata[20];
char message[64];
//-------------------------------------------------------------------

//====================================================================================================
// GŁÓWNA PĘTLA PROGRAMU
//====================================================================================================
void loop() {
  
  SuplaDevice.iterate();

  httpServer.handleClient();
  unsigned long bierzacyCzas = millis();

  if (bierzacyCzas - poprzedniCzas_SATEL >= 1000) { // co ile ms zapytamy centrale o dane
    poprzedniCzas_SATEL = bierzacyCzas;
    SATEL_Przepytaj();
  }

  if (bierzacyCzas - poprzedniCzas >= 9999) { //Dodawaj co 10 sekund
    poprzedniCzas = bierzacyCzas;

    for (int i=1; i <= 10; i++){
      if(SATEL_Czas_minuty_linia[i] < 240){SATEL_Czas_sekundy_linia[i] = SATEL_Czas_sekundy_linia[i] + 10;if(++SATEL_Czas_sekundy_linia[i] >= 60){SATEL_Czas_sekundy_linia[i] = 0;++SATEL_Czas_minuty_linia[i];}}
    }
  }
}

//====================================================================================================
//====================================================================================================
// Odczyt informacji z centrali
//====================================================================================================
//====================================================================================================
void SATEL_Przepytaj() {

  char key[10];
  char key2[10];
  char key3[10];
  char key4[10];
  char key5[10];
  char key6[10];
  char host[10];
  char stime[8];
  char scode[6];
  char sreader[4];

  sprintf(host,"%s",ZABBIX_DEVICE);
  //sprintf(scode,"%s",xcode);
  sprintf(key, "suser");
  sprintf(key2, "sevent");
  sprintf(key3, "sdata");
  sprintf(key4, "stime");
  sprintf(key5, "scode");
  sprintf(key6, "sreader");
  int str_len = SATEL_pracownik.length() + 1; 
  char char_array[str_len];
  SATEL_pracownik.toCharArray(char_array, str_len);
  str_len = SATEL_opis_zdarzenia.length() + 1; 
  char char_array2[str_len];
  SATEL_opis_zdarzenia.toCharArray(char_array2, str_len);
//zabbix_send6(host, key, char_array, key2, char_array2, key3, sdata, key4, stime, key5, scode, key6, sreader);
  zabbix_send6(host, key, char_array, key3, sdata, key4, stime, key5, scode, key6, sreader);
}

//====================================================================================================
//====================================================================================================
// Wysłanie danych i odczyt odpowiedzi z serwera ZABBIX
//====================================================================================================
//====================================================================================================
void ZABBIX_Sender(void *buf, int count) {

  WiFiClient client;
  byte header[400];
  int r;
        
  if(!client.connect(ZABBIX_host.c_str(), ZABBIX_Port)){
    Serial.println("Error connecting to server ZABBIX");
    delay(5000);
  }
  else
  {
    word timeout = 0;
    Serial.println("Connection to ZABBIX server successfully");
    ZABBIX_Liczba_RX = 0;

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

    if(debuguj_transmisje_ZABBIX == 1) {
      //Serial.println(count);
      Serial.print("Zabbix Tx: ");
      //----------------------------------
      //for (int i=0; i < count; i++) {
      //  Serial.print("0x");
      //  Serial.print(header[i],HEX);
      //  if (i<count) Serial.print(",");
      //}
      //Serial.println(" ");
      //----------------------------------
      for (int i=0; i < count; i++) {
        Serial.print((char)header[i]);
      }
      Serial.println(" ");
    }   
  
    client.write(&header[0],count); 

    while(timeout < 50000){    // Przerywamy polaczenie jesli timeout wynosi 50000 = 10 sekund
      ZABBIX_Liczba_RX = client.available();
      for (int i=1; i <= ZABBIX_Liczba_RX; i++) {
        ZABBIX_D_RX[i] = client.read();
      }
      if(ZABBIX_Liczba_RX > 0) {
        if(debuguj_transmisje_ZABBIX == 1) {
          Serial.print("Zabbix Rx: ");
          for (int i=0; i <= ZABBIX_Liczba_RX; i++){
            Serial.print((char)ZABBIX_D_RX[i]);
          }
          Serial.println(" ");
        }
        break;
      }
      delayMicroseconds(200);
      ++timeout;
    }

    bool flaga_timeout = 0;
    if(timeout >= 50000){ // = 10 sekund
      Serial.print("");
      Serial.println(" TIMEOUT");
      flaga_timeout = 1;
    }

    if (ZABBIX_D_RX[27] == 0x73 && ZABBIX_D_RX[28] == 0x75 && ZABBIX_D_RX[29] == 0x63 && ZABBIX_D_RX[30] == 0x63 && ZABBIX_D_RX[31] == 0x65 && ZABBIX_D_RX[32] == 0x73 && ZABBIX_D_RX[33] == 0x73) { 
      if(debuguj_transmisje_ZABBIX == 1) {
        Serial.println("Zabbix response: Success");
        Serial.println(" ");
      }
    }
    client.stop();
  }
}

//====================================================================================================
// Wysłanie informacji do serwera ZABBIX (1 parametr)
//====================================================================================================
void zabbix_send(char *xhostname, char *item_key, char *value_key) {

  char buff[160];
  memset(buff, 0, sizeof(buff));
  sprintf(buff, "{\"request\":\"sender data\",\"data\":[{\"host\":\"%s\",\"key\":\"%s\",\"value\":\"%s\"}]}",xhostname,item_key,value_key);
  ZABBIX_Sender(buff, strlen(buff));
}

//====================================================================================================
// Wysłanie informacji do serwera ZABBIX (2 parametry)
//====================================================================================================
void zabbix_send2(char *xhostname, char *item_key1, char *value_key1, char *item_key2, char *value_key2) {

  char buff[160];
  memset(buff, 0, sizeof(buff));
  sprintf(buff, "{\"request\":\"sender data\",\"data\":[" \
     "{\"host\":\"%s\",\"key\":\"%s\",\"value\":\"%s\"}," \
     "{\"host\":\"%s\",\"key\":\"%s\",\"value\":\"%s\"}]}", \
     xhostname, item_key1, value_key1, \
     xhostname, item_key2, value_key2);
  ZABBIX_Sender(buff, strlen(buff));
}

//====================================================================================================
// Wysłanie informacji do serwera ZABBIX (6 parametrów)
//====================================================================================================
void zabbix_send6(char *xhostname, char *item_key1, char *value_key1, char *item_key3, char *value_key3, char *item_key4, char *value_key4, char *item_key5, char *value_key5, char *item_key6, char *value_key6) {

  char buff[360];
  memset(buff, 0, sizeof(buff));
  sprintf(buff, "{\"request\":\"sender data\",\"data\":[" \
     "{\"host\":\"%s\",\"key\":\"%s\",\"value\":\"%s\"}," \
     "{\"host\":\"%s\",\"key\":\"%s\",\"value\":\"%s\"}," \
     "{\"host\":\"%s\",\"key\":\"%s\",\"value\":\"%s\"}," \
     "{\"host\":\"%s\",\"key\":\"%s\",\"value\":\"%s\"}," \
     "{\"host\":\"%s\",\"key\":\"%s\",\"value\":\"%s\"}]}", \
     xhostname, item_key1, value_key1, \
     xhostname, item_key3, value_key3, \
     xhostname, item_key4, value_key4, \
     xhostname, item_key5, value_key5, \
     xhostname, item_key6, value_key6);
  ZABBIX_Sender(buff, strlen(buff));
}

