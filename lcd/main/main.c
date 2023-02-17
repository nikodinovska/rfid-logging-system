#include <stdio.h>
#include "driver/gpio.h"
#include <rom/ets_sys.h>
#include <string.h>
#include "sdkconfig.h"
#include "esp_log.h"

#define PIN_LCD_RS  16
#define PIN_LCD_RW  17
#define PIN_LCD_E   25

#define PIN_LCD_D4  26
#define PIN_LCD_D5  27
#define PIN_LCD_D6  13
#define PIN_LCD_D7  5

#define GPIO_OUTPUT  0
#define GPIO_INPUT  1

// LCD commands
#define CLEAR_DISPLAY				0x01
#define RETURN_HOME				0x02
#define MODE					0x04
#define CURSOR_DIRECTION_RIGHT			0x02	// MODE command flag
#define CURSOR_DIRECTION_LEFT			0x00	// MODE command flag
#define DISPLAY_SHIFT_ON			0x01	// MODE command flag
#define DISPLAY_SHIFT_OFF			0x00	// MODE command flag
#define CONTROL					0x08
#define DISPLAY_ON				0x04	// CONTROL command flag
#define DISPLAY_OFF				0x00	// CONTROL command flag
#define CURSOR_ON				0x02	// CONTROL command flag
#define CURSOR_OFF				0x00	// CONTROL command flag
#define BLINK_ON				0x01	// CONTROL command flag
#define BLINK_OFF				0x00	// CONTROL command flag
#define SHIFT					0x10
#define DISPLAY					0x08	// SHIFT command flag
#define CURSOR					0x00	// SHIFT command flag
#define RIGHT					0x04	// SHIFT command flag
#define LEFT					0x00	// SHIFT command flag
#define SET					0x20
#define BITS_8					0x10	// SET command flag
#define BITS_4					0x00	// SET command flag
#define LINES_2					0x08	// SET command flag
#define LINE_1					0x00	// SET command flag
#define DOTS_5_10				0x04	// SET command flag
#define DOTS_5_8				0x00	// SET command flag
#define CGRAM					0x40
#define DDRAM					0x80

char lcd_buff[33] = "                                ";
char *lcd_string = lcd_buff;

void gpio_init(uint8_t pinNumber, bool input)
{
  gpio_reset_pin(pinNumber);
  uint8_t mode = ((input == 0) ? GPIO_MODE_OUTPUT : GPIO_MODE_INPUT);
  //gpio_set_direction(pinNumber, ((input == 1) ? GPIO_MODE_INPUT : GPIO_MODE_OUTPUT));
  gpio_set_direction(pinNumber, mode);
  char TAG[50] = " ";
  sprintf(TAG, "Pin configured as: %d", mode);
  ESP_LOGI(TAG, "2 means output");
}

void gpio_on(uint8_t pinNumber)
{
  gpio_set_level(pinNumber, 1);
}

void gpio_off(uint8_t pinNumber)
{
  gpio_set_level(pinNumber, 0);
}

uint8_t lcd_get_cursor(void)
{
  uint8_t position;

  // set pin direction
  gpio_init(PIN_LCD_D4, GPIO_INPUT);
  gpio_init(PIN_LCD_D5, GPIO_INPUT);
  gpio_init(PIN_LCD_D6, GPIO_INPUT);
  gpio_init(PIN_LCD_D7, GPIO_INPUT);

  gpio_init(PIN_LCD_RS, GPIO_OUTPUT);
  gpio_off(PIN_LCD_RS); // RS = 0 -> INSTRUCTION REGISTER
  gpio_init(PIN_LCD_RW, GPIO_OUTPUT);
  gpio_on(PIN_LCD_RW); // RW = 1 -> READ MODE
  ets_delay_us(1);
  gpio_init(PIN_LCD_E, 1);
  ets_delay_us(1);

  position = 0 | (gpio_get_level(PIN_LCD_D7) << 3) | 
    (gpio_get_level(PIN_LCD_D6) << 2) |
    (gpio_get_level(PIN_LCD_D5) << 1) |
    (gpio_get_level(PIN_LCD_D4) << 0);

  gpio_off(PIN_LCD_E);
  ets_delay_us(1);
  gpio_on(PIN_LCD_E);
  ets_delay_us(1);
  gpio_init(PIN_LCD_D4, GPIO_OUTPUT);
  gpio_init(PIN_LCD_D5, GPIO_OUTPUT);
  gpio_init(PIN_LCD_D6, GPIO_OUTPUT);
  gpio_init(PIN_LCD_D7, GPIO_OUTPUT);

  return position;
}

void busy(void)
{
  uint32_t busy_flag;
  gpio_init(PIN_LCD_D4, GPIO_INPUT);
  gpio_init(PIN_LCD_D5, GPIO_INPUT);
  gpio_init(PIN_LCD_D6, GPIO_INPUT);
  gpio_init(PIN_LCD_D7, GPIO_INPUT);

  gpio_init(PIN_LCD_RS, GPIO_OUTPUT);
  gpio_init(PIN_LCD_RW, GPIO_OUTPUT);

  gpio_init(PIN_LCD_E, GPIO_OUTPUT);

  gpio_off(PIN_LCD_RS);
  gpio_on(PIN_LCD_RW);

  do
  {
    ets_delay_us(1);
    gpio_on(PIN_LCD_E);
    ets_delay_us(1);
    busy_flag = gpio_get_level(PIN_LCD_D7);
    gpio_off(PIN_LCD_E);
    ets_delay_us(1);
    gpio_on(PIN_LCD_E);
    ets_delay_us(1);
    gpio_off(PIN_LCD_E);
  } while (busy_flag);
  gpio_init(PIN_LCD_D4, GPIO_OUTPUT);
  gpio_init(PIN_LCD_D5, GPIO_OUTPUT);
  gpio_init(PIN_LCD_D6, GPIO_OUTPUT);
  gpio_init(PIN_LCD_D7, GPIO_OUTPUT);
}

// LCD write high four bits
// byte {data or command byte}
void lcd_write_high(uint8_t byte)
{
  // WRITE MODE
  gpio_off(PIN_LCD_RW);
  ets_delay_us(1);

  gpio_set_level(PIN_LCD_D7, (byte & (1<<7)));
  gpio_set_level(PIN_LCD_D6, (byte & (1<<6)));
  gpio_set_level(PIN_LCD_D5, (byte & (1<<5)));
  gpio_set_level(PIN_LCD_D4, (byte & (1<<4)));

  gpio_on(PIN_LCD_E);
  ets_delay_us(1);
  gpio_off(PIN_LCD_E);
}

// LCD write low four bits
// byte {data or command byte}
void lcd_write_low(uint8_t byte)
{
  // WRITE MODE
  //gpio_off(PIN_LCD_RW);
  ets_delay_us(1);

  gpio_set_level(PIN_LCD_D7, (byte & (1<<3)));
  gpio_set_level(PIN_LCD_D6, (byte & (1<<2)));
  gpio_set_level(PIN_LCD_D5, (byte & (1<<1)));
  gpio_set_level(PIN_LCD_D4, (byte & (1<<0)));

  gpio_on(PIN_LCD_E);
  ets_delay_us(1);
  gpio_off(PIN_LCD_E);
  busy();
}

// LCD write half command
void lcd_write_half_comm(uint8_t com)
{
  // RS = 0 -> Instruction
  gpio_off(PIN_LCD_RS);
  lcd_write_high(com);
}

// LCD write command
void lcd_write_comm(uint8_t com)
{
  lcd_write_half_comm(com);
  lcd_write_low(com);
}

// LCD write data {character}
void lcd_write_data(uint8_t data)
{
  // RS = 1 -> Data register
  gpio_on(PIN_LCD_RS);
  lcd_write_high(data);
  lcd_write_low(data);
}


void lcd_init(void)
{
  printf("GPIO INIT");
  gpio_init(PIN_LCD_D4, GPIO_OUTPUT);
  gpio_init(PIN_LCD_D5, GPIO_OUTPUT);
  gpio_init(PIN_LCD_D6, GPIO_OUTPUT);
  gpio_init(PIN_LCD_D7, GPIO_OUTPUT);
  printf("ssss");
  gpio_init(PIN_LCD_RS, GPIO_OUTPUT);
  gpio_init(PIN_LCD_RW, GPIO_OUTPUT);
  gpio_init(PIN_LCD_E, GPIO_OUTPUT);
  printf("GPIO OUTPUTS");
  ets_delay_us(15000);

  lcd_write_half_comm(SET | BITS_8);
  ets_delay_us(4100);
  printf("STOPPPPP 2");
  lcd_write_half_comm(SET | BITS_8);
  ets_delay_us(100);
  printf("STOP 3");
  lcd_write_half_comm(SET | BITS_8);
  printf("STOP 4");
  //busy();
  printf("STOP 5");
  lcd_write_half_comm(SET | BITS_4);
  //ets_delay_us(100);
  //busy();
  printf("stop 1");
  lcd_write_comm(SET | BITS_4 | LINES_2 | DOTS_5_8);
  printf(" STOP 6");
  lcd_write_comm(CONTROL | DISPLAY_OFF | CURSOR_OFF | BLINK_OFF);
  printf("STOP 7");
  lcd_write_comm(CLEAR_DISPLAY);
  printf("STOP 8");
  lcd_write_comm(MODE | CURSOR_DIRECTION_RIGHT | DISPLAY_SHIFT_OFF);
  printf("STOP 9");
  lcd_write_comm(CONTROL | DISPLAY_ON | CURSOR_OFF | BLINK_OFF);
  printf("LCD END");
}

void lcd_driver(void)
{
  int i;
  lcd_write_comm(DDRAM | 0x00);
  for(i = 0; i < 16; i++)
  {
    lcd_write_data(lcd_string[i]);
  }
  lcd_write_comm(DDRAM | 0x40);
  for(; i < 32; i++)
  {
    lcd_write_data(lcd_string[i]);
  }
}

void app_main(void)
{
    printf("Main");
    lcd_init();
    printf("Pred WHILE");
    while(1)
    {
      printf("AAAAAAAAAAAAAAAAAAAAAAAAA");
      sprintf(lcd_string, "Hello world");
      //strcpy(lcd_string, "Najmiwo");
      lcd_driver();
    }
    
}
