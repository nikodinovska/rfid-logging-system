#include <string.h>

#include <esp_log.h>

#include <rc522.h>

#include "rfid.h"

static const char *LOG_TAG = "RFID";
static rc522_tag_t new_tag;
static bool new_tag_found;

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
  rc522_create(&config, &scanner);
  rc522_register_events(scanner, RC522_EVENT_ANY, rc522_handler, NULL);
  rc522_start(scanner);
  ESP_LOGI(LOG_TAG, "RFID init successful");
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
    memcpy(&new_tag, tag, sizeof(rc522_tag_t));
  }
  break;
  }
}