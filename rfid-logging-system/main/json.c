#include <cJSON.h>

#include "json.h"

cJSON* create_json(uint64_t timestamp, uint64_t id)
{
  cJSON* root = cJSON_CreateObject();
  cJSON_AddNumberToObject(root, "timestamp", timestamp);
  cJSON_AddNumberToObject(root, "id", id);
  return root;
}