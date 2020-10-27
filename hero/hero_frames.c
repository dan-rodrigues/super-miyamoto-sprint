#include "hero_frames.h"

#include "hero.h"
#include "gcc_lib.h"

static const HeroTileOffset sprint_frame_offsets[] = {
    { .x = 0, .y = 0 },
    { .x = 0, .y = -16 },
    { .x = 16, .y = 0 }
};

static const HeroTileOffset kick_frame_offsets[] = {
    { .x = 0, .y = 0 },
    { .x = 0, .y = -16 },
    { .x = -16, .y = 0 }
};

static const HeroTileOffset standard_offsets[] = {
    { .x = 0, .y = 0 },
    { .x = 0, .y = -16 }
};

static const HeroTileOffset driving_offsets[] = {
    { .x = 0, .y = -2 },
    { .x = 0, .y = -18 }
};

static const HeroTileOffset carry_offsets[] = {
    { .x = 1, .y = 0 },
    { .x = 0, .y = -16 }
};

static const HeroFrameLayout hero_frames[] = {
    { 2, 0, { 0x000, 0x0e0, -1 }, standard_offsets, { 0, -1 } },
    { 2, 0, { 0x002, 0x0e0, -1 }, standard_offsets, { 0, 0 } },
    { 2, 0, { 0x004, 0x0e0, -1 }, standard_offsets, { 0, 0 } },

    { 2, 0, { 0x028, 0x0e0, 0x2c2 }, sprint_frame_offsets, { 0, -1} },
    { 2, 0, { 0x026, 0x0e0, 0x2c2 }, sprint_frame_offsets, { 0, 0 } },
    { 2, 0, { 0x024, 0x0e0, 0x2c2 }, sprint_frame_offsets, { 0, 0 } },

    { 2, 0, { 0x04c, 0x0e0, -1 }, carry_offsets, { 0, -1 } },
    { 2, 0, { 0x04e, 0x0e0, -1 }, carry_offsets, { 0, 0 } },
    { 2, 0, { 0x060, 0x0e0, -1 }, carry_offsets, { 0, 0 } },

    { 0, 0, { 0x06c, 0x08c, -1 }, standard_offsets, { 0, 0 } },
    { 0, 1, { 0x06c, 0x08c, -1 }, standard_offsets, { 0, 0 } },

    { 2, 0, { 0x062, 0x0e0, 0x2c6 }, kick_frame_offsets, { 0, 0 } },
    { 2, 0, { 0x004, 0x0e0, -1 }, driving_offsets, { 0, 1 } },

    { 2, 0, { 0x008, 0x2c4, -1 }, standard_offsets, { 0, 0 } },

    { 2, 0, { 0x006, 0x0e8, -1 }, standard_offsets, { 0, 0 } },
    { 2, 0, { 0x1a4, 0x18e, -1 }, standard_offsets, { 0, 0 } },
    { 2, 0, { 0x02a, 0x18e, 0x2c0 }, sprint_frame_offsets, { 0, 0 } },

    { 0, 0, { 0x004, 0x140, -1 }, standard_offsets, { 0, 0 } },

    { 0, 0, { 0x00e, 0x108, -1 }, standard_offsets, { 0, 0 } },
    { 0, 0, { 0x00e, 0x0e0, -1 }, standard_offsets, { 0, 0 } },
     {0, 1, { 0x00e, 0x0e0, -1 }, standard_offsets, { 0, 0 } },

    { 0, 0, { 0x16e, 0x166, -1 }, standard_offsets, { 0, 0 } },

    { 5, 0, { 0x00c, -1 }, standard_offsets, { 0, 0 } },

    { 0, 0, { -1 }, standard_offsets, { 0, 0 } }
};

void hero_frame_layout(HeroFrame frame, HeroFrameLayout *layout) {
    memcpy(layout, &hero_frames[frame], sizeof(HeroFrameLayout));
}
