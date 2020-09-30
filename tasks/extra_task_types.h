#ifndef extra_task_types_h
#define extra_task_types_h

#include "game_loop.h"

typedef struct ExtraTask ExtraTask;
typedef GameLoopAction (*ExtraTaskMain)(ExtraTask *self);

#endif /* extra_task_types_h */
