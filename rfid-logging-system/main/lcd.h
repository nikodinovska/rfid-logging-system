#ifndef _RFID_LOG_SYS_LCD_H
#define _RFID_LOG_SYS_LCD_H

#include <stdint.h>

void lcd_init(void);
void lcd_start(void);
void lcd_print(const char* msg, uint8_t pos);
void lcd_task(void *arg);

#endif  //_RFID_LOG_SYS_LCD_H