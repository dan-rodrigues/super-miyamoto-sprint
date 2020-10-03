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

void dbg_init_playfield(GameContext *context) {
    // Disable the display while preparing the level / display
    vdp_set_alpha_over_layers(0);
    vdp_enable_layers(0);
    vdp_set_single_palette_color(0, 0x0000);

    const PlayerContext *p1 = &context->players[0];

    // Initialization:

    const LevelAttributes *attributes = level_attributes(0);
    level_load(attributes);

    hero_level_init(p1->hero, attributes);
    camera_init(p1->camera, p1->hero, block_map_table);
    sprite_level_data_perform_initial_load(p1->camera, p1->hero);

    dbg_print_init();

    context->paused = false;

    const bool skip_fade_in = false;

    if (!skip_fade_in) {
        fade_task_init(FADE_IN);
    } else {
        pb_alpha_mask_all(0xf);
    }
}

void dbg_reset_sprites(Hero *hero) {
    hero->life = 3;
    sa_reset();

    // TEST: Spawn a controllable enemy generator
    SpritePosition position = hero->position;
    position.x -= 64;
    position.y -= 128;
    enemy_generator_sprite_init(&position);

    position.x += 128;
    enemy_generator_sprite_init(&position);
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
    static const char * const line_2 = "Select: Reset game world";

    const int16_t base_x = 320;
    const int16_t base_y = 10;

    st_write(line_1, base_x, base_y);
    st_write(line_2, base_x, base_y + 18);
}

GameLoopAction dbg_frame_action(GameContext *context) {
    const PadInputDecoded *p1_pad_edge = &context->players[0].hero->pad_edge;

    // Select: reset game world
    if (p1_pad_edge->select) {
        return GL_ACTION_RESET_WORLD;
    }

    // X button: spawn sprites
//    if (p1_pad_edge->x) {
//        return GL_ACTION_SPAWN_SPRITES;
//    }

    return GL_ACTION_NONE;
}
