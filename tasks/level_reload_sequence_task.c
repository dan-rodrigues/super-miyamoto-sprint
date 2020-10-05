#include "level_reload_sequence_task.h"

#include "extra_task.h"
#include "fade_task.h"

GameLoopAction level_reload_sequence_task_main(ExtraTask *self) {
    LevelReloadSequenceTask *sub = &self->level_reload_sequence;

    switch (sub->state) {
        case LEVEL_RELOAD_SEQUENCE_DELAY:
            if (!(sub->delay_counter--)) {
                ExtraTask *task = fade_task_init(FADE_OUT);
                FadeTask *fade = &task->fade;
                fade->fade_delta = 2;
                fade->palette_mask = sub->fade_palette_mask;
                sub->current_subtask = task;

                sub->state = LEVEL_RELOAD_SEQUENCE_FADE;
            }
            break;
        case LEVEL_RELOAD_SEQUENCE_FADE:
            if (!et_live(sub->current_subtask)) {
                sub->state = LEVEL_RELOAD_SEQUENCE_LOAD;
            }
            break;
        case LEVEL_RELOAD_SEQUENCE_LOAD:
            return sub->final_action;
    }

    return GL_ACTION_NONE;
}

ExtraTask *level_reload_sequence_task_init(uint16_t delay) {
    ExtraTask *task = et_alloc();
    task->main = level_reload_sequence_task_main;

    LevelReloadSequenceTask *sub = &task->level_reload_sequence;
    sub->state = LEVEL_RELOAD_SEQUENCE_DELAY;
    sub->delay_counter = delay;
    sub->final_action = GL_ACTION_RELOAD_LEVEL;
    sub->fade_palette_mask = 0xffff;

    return task;
}
