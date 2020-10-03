#include "fade_task.h"

#include "math_util.h"

#include "extra_task.h"
#include "palette_buffer.h"
#include "game_loop.h"

GameLoopAction fade_task_main(ExtraTask *self) {
    FadeTask *sub = &self->fade;

    sub->fade_step += sub->fade_delta;

    switch (sub->type) {
        case FADE_OUT:
            pb_alpha_mask_all(MAX(0, 15 - sub->fade_step), false);
            break;
        case FADE_IN:
            pb_alpha_mask_all(MIN(15, sub->fade_step), false);
            break;
    }

    if (sub->fade_step >= 16) {
        // Should have faded to black (or full brightness) by now
        et_free(self);
    }

    return GL_ACTION_NONE;
}

ExtraTask *fade_task_init(FadeTaskType type) {
    ExtraTask *task = et_alloc();
    task->main = fade_task_main;

    FadeTask *sub = &task->fade;
    sub->fade_step = 0;
    sub->type = type;
    sub->fade_delta = 1;

    return task;
}
