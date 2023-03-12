#ifndef _RFID_LOG_SYS_MQTT_H
#define _RFID_LOG_SYS_MQTT_H

void wifi_init(void);
void wifi_connect(void);

void mqtt_start(void);
void mqtt_publish_msg(const char* msg, const char* topic);
void mqtt_subscribe_topic(const char* topic);
const char* mqtt_get_msg();

void mqtt_task(void* params);

#endif //_RFID_LOG_SYS_MQTT_H