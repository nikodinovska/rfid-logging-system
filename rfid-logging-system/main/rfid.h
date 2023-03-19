#ifndef _RFID_LOG_SYS_RFID_H
#define _RFID_LOG_SYS_RFID_H

#include <rc522.h>

void rfid_init(void);
void rfid_start(void);
int rfid_get_tag(rc522_tag_t* rfid_tag, int timeout_ms);

#endif // _RFID_LOG_SYS_RFID_H