#include <cJSON.h>

#include "json.h"

void create_json(cJSON *root, uint64_t timestamp, uint64_t id)
{
  root = cJSON_CreateObject();
  cJSON_AddNumberToObject(root, "timestamp", timestamp);
  cJSON_AddNumberToObject(root, "id", id);
}