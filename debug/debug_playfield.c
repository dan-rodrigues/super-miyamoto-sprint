#include "debug_playfield.h"

#include "vdp.h"

#include "hero.h"
#include "camera.h"
#include "camera_init.h"
#include "level_loading.h"
#include "level_attributes.h"
#include "block_map_table.h"
#include "sprite_loading.h"
#include "music.h"
#include "hero_init.h"
#include "sprite_actor.h"
#include "game_loop.h"
#include "debug_print.h"
#include "game_loop.h"
#include "sprite_text.h"
#include "fade_task.h"
#include "palette_buffer.h"
#include "extra_task.h"

void dbg_init_playfield(GameContext *context) {
    const LevelAttributes *attributes = level_attributes(0);
    level_init(attributes, context);
}

void dbg_spawn_platform(const Hero *hero) {
    SpritePosition position = hero->position;
    position.x = hero->position.x;
    position.y = hero->position.y;
    position.y += 20;
    platform_sprite_init(&position, PLATFORM_MOTION_AUTO_X);
}

void dbg_print_sandbox_instructions() {
    static const char * const line_1 = "Start: Pause";
    static const char * const line_2 = "Select: Return to menu";

    const int16_t base_x = 320;
    const int16_t base_y = 10;

    st_write(line_1, base_x, base_y);
    st_write(line_2, base_x, base_y + 18);
}

void dbg_frame_action(GameContext *context) {
    const PadInputDecoded *p1_pad_edge = &context->players[0].hero->pad_edge;

    // Select: reset game world (fade out and reload)
    if (!gl_fading(context) && p1_pad_edge->select && !context->paused) {
        ExtraTask *reload_task = level_reload_sequence_task_init(0, true);
        reload_task->level_reload_sequence.final_action = GL_ACTION_SHOW_TITLE;
        context->current_fade_handle = reload_task->handle;
    }
}
