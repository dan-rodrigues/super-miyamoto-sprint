#include "extra_task_handle.h"

const uint8_t ET_UNDEFINED = UINT8_MAX;
const ExtraTaskHandle ET_HANDLE_FREE = {.index = ET_UNDEFINED, .generation = 0};
