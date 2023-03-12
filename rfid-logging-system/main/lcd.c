#include <string.h>

#include <freertos/FreeRTOS.h>
#include <freertos/timers.h>
#include <freertos/semphr.h>

#include <esp_log.h>
#include <hd44780.h>

#include "lcd.h"

static const char *LOG_TAG = "LCD";

static void lcd_clear(TimerHandle_t xTimer);

#define LCD_BUF_LEN 32
#define LCD_CLEAR_PERIOD_MS 5000
static char lcd_buffer[LCD_BUF_LEN] = "                                ";
static size_t msg_len;
static hd44780_t lcd = {
  .write_cb = NULL,
  .font = HD44780_FONT_5X8,
  .lines = 2,
  .pins = {
    .rs = GPIO_NUM_16,
    .e = GPIO_NUM_25,
    .d4 = GPIO_NUM_26,
    .d5 = GPIO_NUM_27,
    .d6 = GPIO_NUM_13,
    .d7 = GPIO_NUM_5,
    .bl = HD44780_NOT_USED
  }
};
static SemaphoreHandle_t lcd_sem;

void lcd_init(void)
{
  ESP_ERROR_CHECK(hd44780_init(&lcd));
  lcd_clear(NULL);
  lcd_sem = xSemaphoreCreateBinary();

  ESP_LOGI(LOG_TAG, "LCD init successfull");
}

void lcd_print(const char* msg, uint8_t pos)
{
  if(pos >= LCD_BUF_LEN) return;
  msg_len = strlen(msg);
  if(pos + msg_len >= LCD_BUF_LEN)
  {
    strncpy(lcd_buffer + pos, msg, LCD_BUF_LEN - pos);
  }
  else
  {
    strncpy(lcd_buffer + pos, msg, msg_len);
  }
  ESP_LOGI(LOG_TAG, "LCD sending print signal");
  // give sem
  xSemaphoreGive(lcd_sem);
}

void lcd_task(void *arg)
{
  const int ticks = pdMS_TO_TICKS(LCD_CLEAR_PERIOD_MS);
  TimerHandle_t clear_timer = xTimerCreate("lcd_clear", ticks, pdFALSE, NULL, lcd_clear);
  while (1)
  {
    xSemaphoreTake(lcd_sem, portMAX_DELAY);
    xTimerStop(clear_timer, portMAX_DELAY);
    char line[LCD_BUF_LEN/2 + 1] = { 0 };
    strncpy(line, lcd_buffer, LCD_BUF_LEN/2);
    hd44780_gotoxy(&lcd, 0, 0);
    hd44780_puts(&lcd, line);
    strncpy(line, lcd_buffer + LCD_BUF_LEN/2, LCD_BUF_LEN/2);
    hd44780_gotoxy(&lcd, 0, 1);
    hd44780_puts(&lcd, line);
    ESP_LOGI(LOG_TAG, "LCD updated");
    xTimerReset(clear_timer, portMAX_DELAY);
    xTimerStart(clear_timer, portMAX_DELAY);
  }
}

static void lcd_clear(TimerHandle_t xTimer)
{
  char line[17] = "                ";
  hd44780_gotoxy(&lcd, 0, 0);
  hd44780_puts(&lcd, line);
  hd44780_gotoxy(&lcd, 0, 1);
  hd44780_puts(&lcd, line);
  ESP_LOGI(LOG_TAG, "LCD cleared");
}