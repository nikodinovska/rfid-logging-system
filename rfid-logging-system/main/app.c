#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <esp_log.h>

#include "app.h"

#include "config.h"
#include "mqtt.h"
#include "lcd.h"
#include "rfid.h"
#include "utils.h"
#include "json.h"

#define MQTT_WAIT_MS 5
#define MQTT_WAIT_TICKS pdMS_TO_TICKS(MQTT_WAIT_MS)

#define MQTT_SEM_TAKE_MS 5000

#define RFID_SEM_TAKE_MS 500

#define JSON_BUF_SIZE 1024

static const char* LOG_TAG = "APP";

static void app_main_loop(void);

void app_init(void)
{
	// configure log subsystem
	esp_log_level_set("*", ESP_LOG_INFO);
	esp_log_level_set("mqtt_client", ESP_LOG_VERBOSE);
	esp_log_level_set("TRANSPORT_BASE", ESP_LOG_VERBOSE);
	esp_log_level_set("esp-tls", ESP_LOG_VERBOSE);
	esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
	esp_log_level_set("outbox", ESP_LOG_VERBOSE);

	// init hardware
  lcd_init();
  rfid_init();

	// init wifi
	wifi_init();
}

void app_start()
{
	// start hw
	rfid_start();
	lcd_start();

	// connect to wifi
	wifi_connect();

	// start mqtt client
	mqtt_start();

	// wait for MQTT connection to be established
	while(mqtt_get_connection_status() != Connected)
	{
		ESP_LOGI(LOG_TAG, "Waiting for MQTT connection...");
		vTaskDelay(MQTT_WAIT_TICKS);
	}

	// susbscribe to MQTT topic(s)
	mqtt_subscribe_topic(MQTT_SUBSCRIBE_TOPIC);

	// wait for subscription to end
	while(mqtt_get_connection_status() != Connected)
	{
		ESP_LOGI(LOG_TAG, "Waiting for MQTT subscription...");
		vTaskDelay(MQTT_WAIT_TICKS);
	}

	// run main loop
	app_main_loop();
}

static void app_main_loop()
{
	while(1)
	{
		// wait for RFID tag
		rc522_tag_t tag;
		int status_rfid = rfid_get_tag(&tag, RFID_SEM_TAKE_MS);
		if(status_rfid == UNSUCCESSFUL)
		{
			continue;
		}

		// send tag info via MQTT
		/*
		sprintf(buf, "%" PRIu64, tag.serial_number);
		mqtt_publish_msg(buf, MQTT_PUBLISH_TOPIC);
		*/
		// send JSON info via MQTT
		
		cJSON* root = create_json(get_timestamp_ms(), tag.serial_number);;
		static char json_buf[JSON_BUF_SIZE];
		cJSON_PrintPreallocated(root, json_buf, JSON_BUF_SIZE, true);
		ESP_LOGI(LOG_TAG, "Formatted json: %s", json_buf);
		mqtt_publish_msg(json_buf, MQTT_PUBLISH_TOPIC);
		
		// wait for publish to end
		while(mqtt_get_connection_status() != Connected)
		{
			ESP_LOGI(LOG_TAG, "Waiting for MQTT publishing...");
			vTaskDelay(MQTT_WAIT_TICKS);
		}
		cJSON_Delete(root);

		mqtt_data_t response_mqtt;
		// wait for MQTT response
		int response_received = mqtt_get_data_status(&response_mqtt, MQTT_SEM_TAKE_MS);
	
		if(response_received == UNSUCCESSFUL)
		{
			continue;
		}
		
		// MQTT Parsing
		cJSON *mqtt_response_json = cJSON_Parse(response_mqtt.data);
		cJSON *status = cJSON_GetObjectItemCaseSensitive(mqtt_response_json, "status");
		cJSON *name = cJSON_GetObjectItemCaseSensitive(mqtt_response_json, "name");
		// print something on LCD
		lcd_print(status->valuestring, 0);
		// lcd_print("HELLO", 0);
		cJSON_Delete(mqtt_response_json);
	}
}