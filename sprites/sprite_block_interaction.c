#include "sprite_block_interaction.h"

#include "block.h"

static bool horizontal_block_check(const SpritePosition *position, const SpriteBoundingBox *box, SpriteBlockInteractionResult *result, bool test_left);

bool sa_point_inside_solid_block(const SpritePosition *position) {
    uint16_t block = block_lookup_pixel(position->x, position->y);
    BlockInteractionAttributes attributes = block_get_attributes(block);
    return !!(attributes & BLOCK_SOLID_SIDE);
}

bool sa_inside_block_horizontal(const SpritePosition *position,
                                const SpriteBoundingBox *box,
                                SpriteBlockInteractionResult *left_result,
                                SpriteBlockInteractionResult *right_result)
{
    // Sprite on left, block on right
    horizontal_block_check(position, box, left_result, true);

    // Sprite on right, block on left
    horizontal_block_check(position, box, right_result, false);

    return (left_result->solid || right_result->solid);
}

// Returns true is sprite is grounded on a level block

bool sa_block_ground_test(const SpritePosition *position, const SpriteVelocity *velocity, const SpriteBoundingBox *box, SpriteBlockInteractionResult *result) {
    bool rising = velocity->y < 0;
    bool point_in_first_4_rows = (position->y & 0x08) == 0;

    uint32_t lookup_left = position->x + box->offset.x;
    uint16_t block_left = block_lookup_pixel(lookup_left, position->y);
    BlockInteractionAttributes left_attributes = block_get_attributes(block_left);
    bool grounded = left_attributes & BLOCK_CAN_STAND;

    // Don't bother with alternate lookup if it's in same block
    uint32_t lookup_right = lookup_left + box->size.width;

    uint16_t block_right = block_left;
    BlockInteractionAttributes right_attributes = left_attributes;

    if ((lookup_left & 0xfff0) != (lookup_right & 0xfff0)) {
        block_right = block_lookup_pixel(lookup_right, position->y);
        right_attributes = block_get_attributes(block_right);
        grounded |= right_attributes & BLOCK_CAN_STAND;
    }

    bool solid = grounded && !rising && point_in_first_4_rows;

    if (result) {
        result->solid = solid;
        result->level_bound_touched = false;

        result->interactions[0].x = lookup_left;
        result->interactions[0].y = position->y;
        result->interactions[0].attributes = left_attributes;

        result->interactions[1].x = lookup_right;
        result->interactions[1].y = position->y;
        result->interactions[1].attributes = right_attributes;
    }

    return solid;
}

bool sa_inside_block_bottom(const SpritePosition *position, const SpriteBoundingBox *box, SpriteBlockInteractionResult *result) {
    uint32_t lookup_x_left = position->x + box->offset.x;
    uint32_t lookup_x_right = lookup_x_left + box->size.width;

    uint32_t lookup_y = position->y + box->offset.y;
    bool point_in_bottom_half = (lookup_y % 16 >= 8);

    uint16_t block_left = block_lookup_pixel(lookup_x_left, lookup_y);
    BlockInteractionAttributes left_attributes = block_get_attributes(block_left);
    bool inside = left_attributes & BLOCK_SOLID_BOTTOM;

    uint16_t block_right = block_lookup_pixel(lookup_x_right, lookup_y);
    BlockInteractionAttributes right_attributes = block_get_attributes(block_right);
    inside |= right_attributes & BLOCK_SOLID_BOTTOM;

    bool solid = inside && point_in_bottom_half;

    if (result) {
        result->overlap = 16 - lookup_y % 16;
        result->level_bound_touched = false;

        result->interactions[0].x = lookup_x_left;
        result->interactions[0].y = lookup_y;
        result->interactions[0].attributes = left_attributes;

        result->interactions[1].x = lookup_x_right;
        result->interactions[1].y = lookup_y;
        result->interactions[1].attributes = right_attributes;

        result->solid = solid;
    }

    return solid;
}

static bool horizontal_block_check(const SpritePosition *position, const SpriteBoundingBox *box, SpriteBlockInteractionResult *result, bool test_left) {
    uint32_t lookup_y_top = position->y + box->offset.y;
    uint32_t lookup_y_bottom = lookup_y_top + box->size.height;

    uint32_t lookup_x = position->x + box->offset.x + (test_left ? box->size.width : 0);

    uint16_t block_top = block_lookup_pixel(lookup_x, lookup_y_top);
    BlockInteractionAttributes top_attributes = block_get_attributes(block_top);
    bool side_solid = top_attributes & BLOCK_SOLID_SIDE;

    uint16_t block_bottom = block_top;
    BlockInteractionAttributes bottom_attributes = top_attributes;

    // Don't bother with alternate lookup if it's in same block
    if ((lookup_y_top & 0xfff0) != (lookup_y_bottom & 0xfff0)) {
        block_bottom = block_lookup_pixel(lookup_x, lookup_y_bottom);
        bottom_attributes = block_get_attributes(block_bottom);
        side_solid |= bottom_attributes & BLOCK_SOLID_SIDE;
    }

    if (result) {
        result->overlap = (test_left ? lookup_x % 16 : (16 - lookup_x % 16));
        result->level_bound_touched = (lookup_x >= 0x8000);

        result->interactions[0].x = lookup_x;
        result->interactions[0].y = lookup_y_top;
        result->interactions[0].attributes = top_attributes;

        result->interactions[1].x = lookup_x;
        result->interactions[1].y = lookup_y_bottom;
        result->interactions[1].attributes = bottom_attributes;

        result->solid = side_solid;
    }

    return side_solid;
}
