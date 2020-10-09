#ifndef level_reload_sequence_task_h
#define level_reload_sequence_task_h

#include <stdint.h>

#include "extra_task_types.h"

typedef enum {
    LEVEL_RELOAD_SEQUENCE_DELAY,
    LEVEL_RELOAD_SEQUENCE_FADE_HERO,
    LEVEL_RELOAD_SEQUENCE_FADE_REMAINDER
} LevelReloadSequenceState;

typedef struct {
    LevelReloadSequenceState state;
    ExtraTaskHandle current_subtask_handle;
    ExtraTaskHandle current_audio_subtask_handle;
    uint8_t delay_counter;
    uint16_t fade_palette_mask;
    bool play_credits_music;
    bool skip_hero_fade;
    GameLoopAction final_action;
} LevelReloadSequenceTask;

ExtraTask *level_reload_sequence_task_init(uint16_t delay, bool skip_hero_fade);
GameLoopAction level_reload_sequence_task_main(ExtraTask *self);

#endif /* level_reload_sequence_task_h */
