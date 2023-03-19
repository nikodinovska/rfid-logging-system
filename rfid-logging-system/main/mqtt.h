#ifndef _RFID_LOG_SYS_MQTT_H
#define _RFID_LOG_SYS_MQTT_H

#define MQTT_MAX_BUF_LEN 100

typedef struct {
  char data[MQTT_MAX_BUF_LEN];
  char topic[MQTT_MAX_BUF_LEN];
} mqtt_data_t;

typedef enum
{
  Disconnected,
  Connected,
  Working,
  Error
} mqtt_status_t;

void wifi_init(void);
void wifi_connect(void);

void mqtt_start(void);
mqtt_status_t mqtt_get_connection_status(void);

void mqtt_publish_msg(const char* msg, const char* topic);
void mqtt_subscribe_topic(const char* topic);

int mqtt_get_data_status(mqtt_data_t* mqtt_data_ptr, int timeout_ms);

void mqtt_task(void* params);

#endif //_RFID_LOG_SYS_MQTT_H