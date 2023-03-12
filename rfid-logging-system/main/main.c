#include <inttypes.h>
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_log.h"

#include "mqtt.h"
#include "lcd.h"
#include "rfid.h"

#include <app.h>

void app_main()
{
    // init hardware
    lcd_init();
    // rfid_init();
    // ESP_LOGI(TAG, "RFID initialized");

    // configure log subsystem
    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("mqtt_client", ESP_LOG_VERBOSE);
    // esp_log_level_set(TAG, ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_BASE", ESP_LOG_VERBOSE);
    esp_log_level_set("esp-tls", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
    esp_log_level_set("outbox", ESP_LOG_VERBOSE);

    // init for wifi
    wifi_init();

    // connect to wifi
    wifi_connect();

    // connect to mqtt
    mqtt_start();

    xTaskCreate(lcd_task, "lcd_task", configMINIMAL_STACK_SIZE * 3, NULL, 3, NULL);
    // xTaskCreate(rfid_task, "rfid_task", configMINIMAL_STACK_SIZE * 3, NULL, 1, NULL);
    xTaskCreate(mqtt_task, "mqtt_task", configMINIMAL_STACK_SIZE * 5, NULL, 1, NULL);  // @todo check priorities
}
