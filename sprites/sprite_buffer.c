#include "sprite_buffer.h"

#include "vdp.h"
#include "assert.h"

static uint16_t sprite_buffer[256 * 3];
static uint16_t * const buffer_end = sprite_buffer + sizeof(sprite_buffer) / sizeof(uint16_t);

static uint16_t *buffer_pointer = sprite_buffer;
static uint16_t *priority_buffer_pointer = buffer_end;

void sb_reset() {
    buffer_pointer = sprite_buffer;
    priority_buffer_pointer = buffer_end;
}

void sb_write(uint32_t x, uint32_t y, uint32_t g) {
    uint32_t pointer;
    __asm("lw %0, %1\n"
          "sh %2, 0(%0)\n"
          "sh %3, 2(%0)\n"
          "sh %4, 4(%0)\n"
          "addi %0, %0, 6\n"
          "sw %0, %1"
          : "=&r" (pointer), "+m" (buffer_pointer)
          : "r" (x), "r" (y), "r" (g)
          : "memory");

    // *buffer_pointer++ = x;
    // *buffer_pointer++ = y;
    // *buffer_pointer++ = g;
}

void sb_write_priority(uint32_t x, uint32_t y, uint32_t g) {
    priority_buffer_pointer -= 3;
    priority_buffer_pointer[0] = x;
    priority_buffer_pointer[1] = y;
    priority_buffer_pointer[2] = g;
}

void sb_upload() {
    assert(buffer_pointer < buffer_end);
    assert(priority_buffer_pointer > buffer_pointer);

    vdp_seek_sprite(0);

    const uint16_t *unused_buffer = buffer_pointer;

    // Write onscreen low-priority sprites..

    const uint16_t *buffer_read_poiner = sprite_buffer;

    // This if-do-while generates a faster / smaller loop than the obvious while-loop
    // The while-loop variant has an inner of "jump -> branch -> branch -> loop body"
    if (buffer_read_poiner < buffer_pointer) {
        do {
            vdp_write_sprite_meta(buffer_read_poiner[0],
                                  buffer_read_poiner[1],
                                  buffer_read_poiner[2]);
            buffer_read_poiner += 3;
        } while (buffer_read_poiner < buffer_pointer);
    }

    // ..then high-priority..
    
    const uint16_t *priority_buffer_read_poiner = buffer_end;
    while (priority_buffer_pointer < priority_buffer_read_poiner) {
        priority_buffer_read_poiner -= 3;
        vdp_write_sprite_meta(priority_buffer_read_poiner[0],
                              priority_buffer_read_poiner[1],
                              priority_buffer_read_poiner[2]);
        unused_buffer += 3;
    }

    // ..then remove all remaining offscreen sprites

    // Same if-do-while pattern as above for better asm
    if (unused_buffer < buffer_end) {
        do {
            vdp_write_sprite_meta(0, 480, 0);
            unused_buffer += 3;
        } while (unused_buffer < buffer_end);
    }

    sb_reset();
}
