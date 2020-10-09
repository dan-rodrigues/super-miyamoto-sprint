#include "music_fade_task.h"

#include "extra_task.h"
#include "palette_buffer.h"
#include "game_loop.h"
#include "music.h"

GameLoopAction music_fade_task_main(ExtraTask *self) {
    MusicFadeTask *sub = &self->music_fade;

    const uint8_t fade_speed = 3;
    sub->fade_step -= fade_speed;

    if (sub->fade_step < 0) {
        music_scale_volume(0);
        if (sub->next_track) {
            music_start(sub->next_track);
        }

        et_free(self);
        return sub->final_action;
    } else {
        music_scale_volume(sub->fade_step);
    }

    return GL_ACTION_NONE;
}

ExtraTask *music_fade_task_init() {
    ExtraTask *task = et_alloc();
    task->main = music_fade_task_main;

    MusicFadeTask *sub = &task->music_fade;
    sub->fade_step = 255;
    sub->next_track = NULL;
    sub->final_action = GL_ACTION_NONE;

    return task;
}
