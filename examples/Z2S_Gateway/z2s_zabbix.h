#ifndef Z2S_ZABBIX
#define Z2S_ZABBIX

void zabbix_send(const char *xhostname, const char *item_key, const char *value_key);
void zabbix_send2(const char *xhostname, const char *item_key1, const char *value_key1, const char *item_key2, const char *value_key2);
void zabbix_send4(const char *xhostname, const char *item_key1, const char *value_key1, const char *item_key2, const char *value_key2, const char *item_key3, const char *value_key3, const char *item_key4, const char *value_key4);
void zabbix_send6(char *xhostname, char *item_key1, char *value_key1, char *item_key2, char *value_key2, char *item_key3, char *value_key3, char *item_key4, char *value_key4, char *item_key5, char *value_key5, char *item_key6, char *value_key6);

#endif // Z2S_ZABBIX
