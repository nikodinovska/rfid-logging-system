#include "application.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "freertos/semphr.h"

#include <inttypes.h>
#include <sys/time.h>

#include <hd44780.h>
#include <rc522.h>

char lcd_buffer[33] = "                                ";
SemaphoreHandle_t xSem = 0;

void lcd_task(void *arg)
{
  
    hd44780_t lcd = {
        .write_cb = NULL,
        .font = HD44780_FONT_5X8,
        .lines = 2,
        .pins = {
            .rs = GPIO_NUM_16,
            .e  = GPIO_NUM_25,
            .d4 = GPIO_NUM_26,
            .d5 = GPIO_NUM_27,
            .d6 = GPIO_NUM_13,
            .d7 = GPIO_NUM_5,
            .bl = HD44780_NOT_USED
        }
    };

    ESP_ERROR_CHECK(hd44780_init(&lcd));


    hd44780_gotoxy(&lcd, 0, 0);

    while (1)
    {
        xSemaphoreTake(xSem, portMAX_DELAY);
        hd44780_gotoxy(&lcd, 0, 0);

        hd44780_puts(&lcd, lcd_buffer);
        //xSemaphoreGive(xSem);

        //vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static uint32_t get_time_sec()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec;
}

void rfid_task(void *arg)
{
  xSem = xSemaphoreCreateBinary();
  while(1)
  {
    //xSemaphoreTake(xSem, portMAX_DELAY);
    if(get_time_sec() % 5 == 0)
    {
      snprintf(lcd_buffer, 7, "%" PRIu32 "  ", get_time_sec());
     xSemaphoreGive(xSem);      
    }
  }

}