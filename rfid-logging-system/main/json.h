#ifndef _RFID_LOG_SYS_JSON_H
#define _RFID_LOG_SYS_JSON_H

#include <stdint.h>

#include <cJSON.h>

void create_json(cJSON *root, uint64_t timestamp, uint64_t id);

#endif // _RFID_LOG_SYS_JSON_H