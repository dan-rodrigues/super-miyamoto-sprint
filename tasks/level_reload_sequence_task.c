#include "level_reload_sequence_task.h"

#include "extra_task.h"
#include "fade_task.h"

GameLoopAction level_reload_sequence_task_main(ExtraTask *self) {
    LevelReloadSequenceTask *sub = &self->level_reload_sequence;

    switch (sub->state) {
        case LEVEL_RELOAD_SEQUENCE_INITIAL:
            sub->state = LEVEL_RELOAD_SEQUENCE_DELAY;

            const uint8_t prefade_delay = 30;
            sub->delay_counter = prefade_delay;
            break;
        case LEVEL_RELOAD_SEQUENCE_DELAY:
            if (!(--sub->delay_counter)) {
                sub->current_subtask = fade_task_init(FADE_OUT);
                sub->state = LEVEL_RELOAD_SEQUENCE_FADE;
            }
            break;
        case LEVEL_RELOAD_SEQUENCE_FADE:
            if (!et_live(sub->current_subtask)) {
                sub->state = LEVEL_RELOAD_SEQUENCE_LOAD;
            }
            break;
        case LEVEL_RELOAD_SEQUENCE_LOAD:
            return GL_ACTION_RESET_WORLD;
    }

    return GL_ACTION_NONE;
}

ExtraTask *level_reload_sequence_task_init() {
    ExtraTask *task = et_alloc();
    task->main = level_reload_sequence_task_main;

    LevelReloadSequenceTask *sub = &task->level_reload_sequence;
    sub->state = LEVEL_RELOAD_SEQUENCE_INITIAL;

    return task;
}
