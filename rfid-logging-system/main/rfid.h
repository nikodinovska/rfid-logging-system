#ifndef _RFID_LOG_SYS_RFID_H
#define _RFID_LOG_SYS_RFID_H

#include <rc522.h>

void rfid_init(void);
void rfid_start(void);
rc522_tag_t rfid_get_tag(void);

#endif // _RFID_LOG_SYS_RFID_H