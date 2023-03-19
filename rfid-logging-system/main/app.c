#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <esp_log.h>

#include "app.h"

#include "config.h"
#include "mqtt.h"
#include "lcd.h"
#include "rfid.h"
#include "utils.h"

#define MQTT_WAIT_MS 5
#define MQTT_WAIT_TICKS pdMS_TO_TICKS(MQTT_WAIT_MS)

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
	char buf[100]; //temp
	while(1)
	{
		// wait for RFID tag
		rc522_tag_t tag = rfid_get_tag();

		// send tag info via MQTT
		
		sprintf(buf, "%" PRIu64, tag.serial_number);
		mqtt_publish_msg(buf, MQTT_PUBLISH_TOPIC);

		// send JSON info via MQTT
		/*
		cJSON root;
		mqtt_publish_json_msg(&root, get_timestamp_ms(), tag.serial_number, MQTT_PUBLISH_TOPIC);
		*/
		// wait for publish to end
		while(mqtt_get_connection_status() != Connected)
		{
			ESP_LOGI(LOG_TAG, "Waiting for MQTT publishing...");
			vTaskDelay(MQTT_WAIT_TICKS);
		}

		mqtt_data_t response_mqtt;
		// wait for MQTT response
		int response_received = mqtt_get_data_status(&response_mqtt, -5);
		printf("response_received: %d", response_received);

		// print something on LCD
		//lcd_print(response_mqtt.data, 0);
		lcd_print("HELLO", 0);
	}
}