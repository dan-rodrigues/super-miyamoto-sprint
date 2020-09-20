#ifndef sprite_block_interaction_h
#define sprite_block_interaction_h

#include <stdint.h>
#include <stdbool.h>

#include "sprite_block_interaction_types.h"
#include "sprite_actor.h"
#include "block.h"

struct SpriteBlockPositionedInteraction {
    uint32_t x, y;
    BlockInteractionAttributes attributes;
};

struct SpriteBlockInteractionResult {
    int32_t overlap;
    bool solid;
    bool level_bound_touched;
    SpriteBlockPositionedInteraction interactions[2];
};

bool sa_inside_block_bottom(const SpritePosition *position, const SpriteBoundingBox *box, SpriteBlockInteractionResult *result);

bool sa_inside_block_horizontal(const SpritePosition *position,
                                const SpriteBoundingBox *bounding_box,
                                SpriteBlockInteractionResult *left_result,
                                SpriteBlockInteractionResult *right_result);

bool sa_block_ground_test(const SpritePosition *position, const SpriteVelocity *velocity, const SpriteBoundingBox *box, SpriteBlockInteractionResult *result);

#endif /* sprite_block_interaction_h */
