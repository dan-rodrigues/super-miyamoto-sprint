#ifndef extra_task_handle_h
#define extra_task_handle_h

#include <stdint.h>

typedef struct {
    uint8_t index;
    uint8_t generation;
} ExtraTaskHandle;

const uint8_t ET_UNDEFINED;
extern const ExtraTaskHandle ET_HANDLE_FREE;

#endif /* extra_task_handle_h */
