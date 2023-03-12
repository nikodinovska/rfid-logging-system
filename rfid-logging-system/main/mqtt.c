#include <nvs_flash.h>
#include <esp_netif.h>
#include <protocol_examples_common.h>
#include <esp_log.h>
#include <esp_wifi.h>
#include <esp_system.h>

#include <esp_event.h>
#include <esp_netif.h>

#include <lwip/sockets.h>
#include <lwip/dns.h>
#include <lwip/netdb.h>
#include <mqtt_client.h>

#include "mqtt.h"
#include "lcd.h"

static const char *LOG_TAG = "MQTT";
static const char *APP_TAG = "APP"; //< @todo move to other source

enum mqtt_status
{
  Idle,
  Done,
  Error
};

static esp_mqtt_client_handle_t mqtt_client;
static enum mqtt_status mqtt_stat;
#define MQTT_MAX_BUF_LEN 100
static bool mqtt_new_data_received;
static char mqtt_new_data[MQTT_MAX_BUF_LEN];
static char mqtt_new_data_topic[MQTT_MAX_BUF_LEN];

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);
static void log_error_if_nonzero(const char *message, int error_code);

void wifi_init(void)
{
  ESP_ERROR_CHECK(nvs_flash_init());
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());
}

void wifi_connect(void)
{
  ESP_ERROR_CHECK(example_connect());
}

void mqtt_start(void)
{
  ///@todo remove mqtt broker URI from code, put in config file
  esp_mqtt_client_config_t mqtt_cfg = {
      .broker.address.uri = "mqtt://192.168.2.100",
  };
  mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
  /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
  esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
  esp_mqtt_client_start(mqtt_client);
}

void mqtt_publish_msg(const char* msg, const char* topic)
{
  msg_id = esp_mqtt_client_publish(mqtt_client, topic, msg, 0, 1, 0);
  ESP_LOGI(LOG_TAG, "MQTT sent publish, msg_id=%d", msg_id);
}

void mqtt_subscribe_topic(const char* topic)
{
  int msg_id = esp_mqtt_client_subscribe(mqtt_client, topic, 0);
  ESP_LOGI(APP_TAG, "MQTT sent subscribe, msg_id=%d", msg_id);
}

void mqtt_task(void* params)
{
  const int wait_ms = 5;
  const TickType_t wait_ticks = pdMS_TO_TICKS(wait_ms);
  // wait for mqtt connection
  ESP_LOGI(APP_TAG, "Wait for MQTT connect");
  while(mqtt_stat == Idle)
  {
    vTaskDelay(wait_ticks);
  }
  if(mqtt_stat == Error)
  {
    ESP_LOGE(APP_TAG, "MQTT error occured");
    vTaskDelete(NULL);
  }
  mqtt_stat = Idle;
  // subscribe to topic
  int msg_id = esp_mqtt_client_subscribe(mqtt_client, "/topic/qos0", 0);
  ESP_LOGI(APP_TAG, "MQTT sent subscribe, msg_id=%d", msg_id);
  // wait for subscription
  while(mqtt_stat == Idle)
  {
    vTaskDelay(wait_ticks);
  }
  if(mqtt_stat == Error)
  {
    ESP_LOGE(APP_TAG, "MQTT error occured");
    vTaskDelete(NULL);
  }
  mqtt_stat = Idle;
  // publish to topic
  msg_id = esp_mqtt_client_publish(mqtt_client, "/topic/qos1", "bump", 0, 1, 0);
  ESP_LOGI(APP_TAG, "sent publish, msg_id=%d", msg_id);
  // wait for publish to finish
  while(mqtt_stat == Idle)
  {
    vTaskDelay(wait_ticks);
  }
  if(mqtt_stat == Error)
  {
    ESP_LOGE(APP_TAG, "MQTT error occured");
    vTaskDelete(NULL);
  }
  while(true)
  {
    if(mqtt_new_data_received)
    {
      mqtt_new_data_received = false;
      ESP_LOGI(APP_TAG, "Received MQTT data topic: %s", mqtt_new_data_topic);
      ESP_LOGI(APP_TAG, "Received MQTT data: %s", mqtt_new_data);
      lcd_print(mqtt_new_data, 0);
    }
    else
    {
      vTaskDelay(wait_ticks);
    }
  }
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
  ESP_LOGD(LOG_TAG, "Event dispatched from event loop base=%s, event_id=%ld", base, event_id);
  esp_mqtt_event_handle_t event = event_data;
  switch ((esp_mqtt_event_id_t)event_id)
  {
  case MQTT_EVENT_CONNECTED:
    ESP_LOGI(LOG_TAG, "MQTT_EVENT_CONNECTED");
    mqtt_stat = Done;
    break;
  case MQTT_EVENT_DISCONNECTED:
    ESP_LOGI(LOG_TAG, "MQTT_EVENT_DISCONNECTED");
    break;
  case MQTT_EVENT_SUBSCRIBED:
    ESP_LOGI(LOG_TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
    mqtt_stat = Done;
    break;
  case MQTT_EVENT_UNSUBSCRIBED:
    ESP_LOGI(LOG_TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
    break;
  case MQTT_EVENT_PUBLISHED:
    ESP_LOGI(LOG_TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
    mqtt_stat = Done;
    break;
  case MQTT_EVENT_DATA:
    ESP_LOGI(LOG_TAG, "MQTT_EVENT_DATA");
    ESP_LOGI(LOG_TAG, "MQTT topic: %s", event->topic);
    ESP_LOGI(LOG_TAG, "MQTT data: %s", event->data);
    strncpy(mqtt_new_data_topic, event->topic, event->topic_len);
    mqtt_new_data_topic[event->topic_len] = '\0';
    strncpy(mqtt_new_data, event->data, event->data_len);
    mqtt_new_data[event->data_len] = '\0';
    mqtt_new_data_received = true;
    break;
  case MQTT_EVENT_ERROR:
    ESP_LOGI(LOG_TAG, "MQTT_EVENT_ERROR");
    if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT)
    {
      log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
      log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
      log_error_if_nonzero("captured as transport's socket errno", event->error_handle->esp_transport_sock_errno);
      ESP_LOGI(LOG_TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
    }
    mqtt_stat = Error;
    break;
  default:
    ESP_LOGI(LOG_TAG, "Other event id:%d", event->event_id);
    break;
  }
}

static void log_error_if_nonzero(const char *message, int error_code)
{
  if (error_code != 0)
  {
    ESP_LOGE(LOG_TAG, "Last error %s: 0x%x", message, error_code);
  }
}