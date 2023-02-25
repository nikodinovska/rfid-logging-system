#include <inttypes.h>
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>


#include <hd44780.h>
#include <rc522.h>
#include <application.h>


void app_main()
{
    xTaskCreate(lcd_task, "lcd_task", configMINIMAL_STACK_SIZE * 3, NULL, 5, NULL);
    xTaskCreate(rfid_task, "rfid_task", configMINIMAL_STACK_SIZE * 3, NULL, 4, NULL);
}
