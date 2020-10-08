#include "fade_task.h"

#include "math_util.h"

#include "extra_task.h"
#include "palette_buffer.h"
#include "game_loop.h"

GameLoopAction fade_task_main(ExtraTask *self) {
    FadeTask *sub = &self->fade;

    switch (sub->type) {
        case FADE_OUT: {
            uint8_t alpha = MAX(0, 15 - sub->fade_step);
            pb_alpha_mask_multiple(sub->palette_mask, alpha, false);
        } break;
        case FADE_IN: {
            uint8_t alpha = MIN(15, sub->fade_step);
            pb_alpha_mask_multiple(sub->palette_mask, alpha, false);
        } break;
    }

    if (sub->fade_step >= 16) {
        // Should have faded to black (or full brightness) by now
        et_free(self);
        return sub->final_action;
    } else {
        sub->fade_step += sub->fade_delta;
        return GL_ACTION_NONE;
    }
}

ExtraTask *fade_task_init(FadeTaskType type) {
    ExtraTask *task = et_alloc();
    task->main = fade_task_main;

    FadeTask *sub = &task->fade;
    sub->fade_step = 0;
    sub->type = type;
    sub->fade_delta = 1;
    sub->palette_mask = 0xffff;
    sub->final_action = GL_ACTION_NONE;

    return task;
}
