#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

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

#include "config.h"
#include "json.h"
#include "mqtt.h"
#include "utils.h"

static const char *LOG_TAG = "MQTT";

static esp_mqtt_client_handle_t mqtt_client;
static mqtt_status_t mqtt_status = Disconnected;
static mqtt_data_t mqtt_data;
SemaphoreHandle_t mqtt_sem;

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
      .broker.address.uri = MQTT_BROKER_URI,
  };
  mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
  esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
  mqtt_status = Disconnected;
  mqtt_sem = xSemaphoreCreateBinary();
  esp_mqtt_client_start(mqtt_client);
}

mqtt_status_t mqtt_get_connection_status(void)
{
  return mqtt_status;
}

void mqtt_publish_msg(const char* msg, const char* topic)
{
  int msg_id = esp_mqtt_client_publish(mqtt_client, topic, msg, 0, 1, 0);
  ESP_LOGI(LOG_TAG, "MQTT sent publish, msg_id=%d", msg_id);
  mqtt_status = Working;
}

void mqtt_subscribe_topic(const char* topic)
{
  int msg_id = esp_mqtt_client_subscribe(mqtt_client, topic, 0);
  ESP_LOGI(LOG_TAG, "MQTT sent subscribe, msg_id=%d", msg_id);
  mqtt_status = Working;
}
/*
const mqtt_data_t* mqtt_get_data(int timeout)
{
  xSemaphoreTake(mqtt_sem, timeout);

  return &mqtt_data;
}
*/
int mqtt_get_data_status(mqtt_data_t* mqtt_data_ptr, int timeout_ms)
{
  timeout_ms = ((timeout_ms < 0) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms));
  if (xSemaphoreTake(mqtt_sem, timeout_ms) == pdFALSE)
  {
    ESP_LOGI(LOG_TAG, "Semaphore not taken...");
    return UNSUCCESSFUL;
  }
  strcpy(mqtt_data_ptr->topic, mqtt_data.topic);
  strcpy(mqtt_data_ptr->data, mqtt_data.data);
  return SUCCESSFUL;
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
  ESP_LOGD(LOG_TAG, "Event dispatched from event loop base=%s, event_id=%ld", base, event_id);
  esp_mqtt_event_handle_t event = event_data;
  switch ((esp_mqtt_event_id_t)event_id)
  {
  case MQTT_EVENT_CONNECTED:
    ESP_LOGI(LOG_TAG, "MQTT_EVENT_CONNECTED");
    mqtt_status = Connected;
    break;
  case MQTT_EVENT_DISCONNECTED:
    ESP_LOGI(LOG_TAG, "MQTT_EVENT_DISCONNECTED");
    mqtt_status = Disconnected;
    break;
  case MQTT_EVENT_SUBSCRIBED:
    ESP_LOGI(LOG_TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
    mqtt_status = Connected;
    break;
  case MQTT_EVENT_UNSUBSCRIBED:
    ESP_LOGI(LOG_TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
    mqtt_status = Connected;
    break;
  case MQTT_EVENT_PUBLISHED:
    ESP_LOGI(LOG_TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
    mqtt_status = Connected;
    break;
  case MQTT_EVENT_DATA:
    ESP_LOGI(LOG_TAG, "MQTT_EVENT_DATA");
    ESP_LOGI(LOG_TAG, "MQTT topic: %s", event->topic);
    ESP_LOGI(LOG_TAG, "MQTT data: %s", event->data);
    strncpy(mqtt_data.topic, event->topic, event->topic_len);
    mqtt_data.topic[event->topic_len] = '\0';
    strncpy(mqtt_data.data, event->data, event->data_len);
    mqtt_data.data[event->data_len] = '\0';
    mqtt_status = Connected;
    xSemaphoreGive(mqtt_sem);
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
    mqtt_status = Error;
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