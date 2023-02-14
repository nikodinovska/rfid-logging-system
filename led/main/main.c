#include <stdio.h>
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define BLINK_GPIO  2

void led_on(uint8_t pinNumber)
{
  gpio_set_level(pinNumber, 1);
}

void led_off(uint8_t pinNumber)
{
  gpio_set_level(pinNumber, 0);
}

void led_init(uint8_t pinNumber)
{
  gpio_reset_pin(pinNumber);
  // Set the GPIO as a push/pull output
  gpio_set_direction(pinNumber, GPIO_MODE_OUTPUT);
}

void app_main(void)
{
  while(1)
  {
    led_init(BLINK_GPIO);
    led_on(BLINK_GPIO);
    vTaskDelay(pdMS_TO_TICKS(500));
    led_off(BLINK_GPIO);
    vTaskDelay(pdMS_TO_TICKS(500));
  }  
}
