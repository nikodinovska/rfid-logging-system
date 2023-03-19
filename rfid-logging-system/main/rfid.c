#include <string.h>

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#include <esp_log.h>

#include "rfid.h"

static const char *LOG_TAG = "RFID";
static rc522_handle_t rfid_scanner;
static rc522_tag_t rfid_last_tag;
SemaphoreHandle_t rfid_sem;

static void rc522_handler(void *arg, esp_event_base_t base, int32_t event_id, void *event_data);

void rfid_init(void)
{
  rc522_config_t config = {
      .spi.host = VSPI_HOST,
      .spi.miso_gpio = GPIO_NUM_19,
      .spi.mosi_gpio = GPIO_NUM_23,
      .spi.sck_gpio = GPIO_NUM_18,
      .spi.sda_gpio = GPIO_NUM_21,
  };
  rc522_create(&config, &rfid_scanner);
  rc522_register_events(rfid_scanner, RC522_EVENT_ANY, rc522_handler, NULL);
  rfid_sem = xSemaphoreCreateBinary();
  ESP_LOGI(LOG_TAG, "RFID init successful");
}

void rfid_start(void)
{
  rc522_start(rfid_scanner);
}

rc522_tag_t rfid_get_tag(void)
{
  xSemaphoreTake(rfid_sem, portMAX_DELAY);
  return rfid_last_tag;
}

static void rc522_handler(void *arg, esp_event_base_t base, int32_t event_id, void *event_data)
{
  rc522_event_data_t *data = (rc522_event_data_t *)event_data;
  switch (event_id)
  {
  case RC522_EVENT_TAG_SCANNED:
  {
    rc522_tag_t *tag = (rc522_tag_t *)data->ptr;
    ESP_LOGI(LOG_TAG, "Tag scanned (sn: %" PRIu64 ")", tag->serial_number);
    memcpy(&rfid_last_tag, tag, sizeof(rc522_tag_t));
    xSemaphoreGive(rfid_sem);
  }
  break;
  }
}