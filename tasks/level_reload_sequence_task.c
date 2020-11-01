#include "level_reload_sequence_task.h"

#include "extra_task.h"
#include "fade_task.h"
#include "music.h"

GameLoopAction level_reload_sequence_task_main(ExtraTask *self) {
    LevelReloadSequenceTask *sub = &self->level_reload_sequence;

    switch (sub->state) {
        case LEVEL_RELOAD_SEQUENCE_DELAY:
            if (!(sub->delay_counter--)) {
                // 1. Fade out palette(s)
                ExtraTask *task = fade_task_init(FADE_OUT);
                FadeTask *fade = &task->fade;
                fade->palette_mask = sub->fade_palette_mask;
                fade->fade_delta = 2;
                sub->current_subtask_handle = task->handle;

                // 2. Fade out music, then play credits
                if (sub->fade_music) {
                    ExtraTask *music_task = music_fade_task_init();
                    sub->current_audio_subtask_handle = music_task->handle;
                }

                LevelReloadSequenceState new_state = sub->skip_hero_fade
                    ? LEVEL_RELOAD_SEQUENCE_FADE_REMAINDER : LEVEL_RELOAD_SEQUENCE_FADE_HERO;

                sub->state = new_state;
            }
            break;
        case LEVEL_RELOAD_SEQUENCE_FADE_HERO:
            if (!et_handle_live(sub->current_subtask_handle)) {
                ExtraTask *fade_task = fade_task_init(FADE_OUT);
                FadeTask *fade = &fade_task->fade;
                fade->palette_mask = ~0x0100;
                fade->fade_delta = 2;
                sub->current_subtask_handle = fade_task->handle;

                sub->state = LEVEL_RELOAD_SEQUENCE_FADE_REMAINDER;
            }
            break;
        case LEVEL_RELOAD_SEQUENCE_FADE_REMAINDER: {
            // 3. Wait until fade and music are finished
            bool tasks_complete = (
                !et_handle_live(sub->current_subtask_handle) &&
                !et_handle_live(sub->current_audio_subtask_handle)
            );

            if (tasks_complete) {
                // don't need to free self, the reset will do that
                return sub->final_action;
            }
        } break;
    }

    return GL_ACTION_NONE;
}

ExtraTask *level_reload_sequence_task_init(uint16_t delay, bool skip_hero_fade) {
    ExtraTask *task = et_alloc();
    task->main = level_reload_sequence_task_main;

    LevelReloadSequenceTask *sub = &task->level_reload_sequence;
    sub->delay_counter = delay;
    sub->fade_palette_mask = (skip_hero_fade ? 0xffff : 0x0100);
    sub->skip_hero_fade = skip_hero_fade;

    sub->state = LEVEL_RELOAD_SEQUENCE_DELAY;
    sub->final_action = GL_ACTION_RELOAD_LEVEL;
    sub->fade_music = false;
    sub->current_subtask_handle = ET_HANDLE_FREE;
    sub->current_audio_subtask_handle = ET_HANDLE_FREE;
    return task;
}
