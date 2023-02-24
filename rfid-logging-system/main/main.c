#include <inttypes.h>
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <sys/time.h>

#include <hd44780.h>
#include <rc522.h>

static uint32_t get_time_sec()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec;
}

void lcd_test(void *pvParameters)
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
    hd44780_puts(&lcd, "Hello world!");

    char time[16];

    while (1)
    {
        hd44780_gotoxy(&lcd, 2, 1);
 
        snprintf(time, 7, "%" PRIu32 "  ", get_time_sec());
        time[sizeof(time) - 1] = 0;

        hd44780_puts(&lcd, time);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void app_main()
{
    xTaskCreate(lcd_test, "lcd_test", configMINIMAL_STACK_SIZE * 3, NULL, 5, NULL);
}
