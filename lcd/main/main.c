#include <stdio.h>
#include "driver/gpio.h"

#define RS  PIN_1
#define RW  PIN_2

typedef struct lcd
{
  char buffer[32];
  uint8_t cursor = 0;
} lcd;

void gpio_init(uint8_t pinNumber)
{
  gpio_reset_pin(pinNumber);
  // Set the GPIO as a push/pull output
  gpio_set_direction(pinNumber, GPIO_MODE_OUTPUT);
}

void gpio_on(uint8_t pinNumber)
{
  gpio_set_level(pinNumber, 1);
}

void gpio_off(uint8_t pinNumber)
{
  gpio_set_level(pinNumber, 0);
}

void write_data(lcd *lcd)
{
  if (lcd->cursor == 0x0f)
  {
    lcd->cursor = 0x40;
  }
  else if (lcd->cursor == 0x4f)
  {
    lcd->cursor = 0x00;
  }
  // RS = 1, RW = 0 --- RS = 0 ova e adresa postavuvanje
  gpio_on(RS);
  gpio_off(RW);
  // DB7
  gpio_on(DB7);
  // DB6
  gpio_set_level(DB6, cursor & (1 << 6));
  // DB5
  gpio_set_level(DB5, cursor & (1 << 5));
  // DB4
  gpio_set_level(DB4, cursor & (1 << 4));
  // Enable
  gpio_set_level(E, 1);
  gpio_set_level(E, 0);
  // RS = 1, RW = 0
  gpio_on(RS);
  gpio_off(RW);
  // DB3
  gpio_set_level(DB3, cursor & (1 << 3));
  // DB2
  gpio_set_level(DB2, cursor & (1 << 2));
  // DB1
  gpio_set_level(DB1, cursor & (1 << 1));
  // DB0
  gpio_set_level(DB0, cursor & (1 << 0));
  // Enable
  gpio_set_level(E, 1);
  gpio_set_level(E, 0);
  lcd->cursor++;    
}

void app_main(void)
{

}
