#include "palette_lerp_task.h"

#include "extra_task.h"
#include "palette_buffer.h"
#include "game_loop.h"

GameLoopAction palette_lerp_task_main(ExtraTask *self) {
    PaletteLerpTask *sub = &self->palette_lerp;

    pb_lerp_palette_to_white(sub->palette_id, sub->color_id_mask, sub->fade_step++);

    if (sub->fade_step >= 16) {
        // Should have faded to black (or full brightness) by now
        et_free(self);
    }

    return GL_ACTION_NONE;
}

ExtraTask *palette_lerp_task_init(uint8_t palette_id, uint16_t mask) {
    ExtraTask *task = et_alloc();
    task->main = palette_lerp_task_main;

    PaletteLerpTask *sub = &task->palette_lerp;
    sub->palette_id = palette_id;
    sub->color_id_mask = mask;
    sub->fade_step = 0;

    return task;
}
