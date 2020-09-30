#include "vram_command_queue.h"

#include "vdp.h"
#include "assert.h"

#define COMMAND_QUEUE_MAX 24

static uint32_t queue_index = 0;
static VRAMUploadCommand queue[COMMAND_QUEUE_MAX];

void vcq_add(const VRAMUploadCommand *cmd) {
    assert(queue_index < COMMAND_QUEUE_MAX);

    VRAMUploadCommand *target = &queue[queue_index];
    target->address = cmd->address;
    target->length = cmd->length;
    target->increment = cmd->increment;
    target->is_inline = cmd->is_inline;
    target->data = cmd->data;

    queue_index++;
}

void vcq_reset() {
    queue_index = 0;
}

void vcq_run() {
    for (uint32_t i = 0; i < queue_index; i++) {
        const VRAMUploadCommand *cmd = &queue[i];
        vdp_seek_vram(cmd->address);
        vdp_set_vram_increment(cmd->increment);
        const uint16_t *source = (cmd->is_inline ? cmd->inline_data : cmd->data);
        uint16_t size = (cmd->is_inline ? 2 : cmd->length);
        vdp_write_vram_block(source, size);
    }

    vcq_reset();
}

void vcq_add_16x16(uint16_t vram_base, uint16_t vram_tile, uint16_t source_tile, const uint16_t *data) {
    const uint16_t *source_tiles = data + source_tile * (0x20 / sizeof(uint16_t));
    const uint16_t frame_row_word_size = 0x10 * 2;

    VRAMUploadCommand top_row_cmd = {
        .address = vram_base + vram_tile * 0x10,
        .length = frame_row_word_size,
        .increment = 1,
        .is_inline = false,
        .data = source_tiles
    };
    vcq_add(&top_row_cmd);

    const uint16_t sprite_row_word_stride = 0x20 * 0x10 / 2;

    VRAMUploadCommand bottom_row_cmd = {
        .address = vram_base + (0x10 + vram_tile) * 0x10,
        .length = frame_row_word_size,
        .increment = 1,
        .is_inline = false,
        .data = source_tiles + sprite_row_word_stride
    };
    vcq_add(&bottom_row_cmd);
}

void vcq_add_2x2_map(uint16_t address_base, uint16_t tl, uint16_t tr, uint16_t bl, uint16_t br) {
    VRAMUploadCommand cmd = {
        .address = address_base,
        .increment = 1,
        .is_inline = true,
        .inline_data = {tl, tr}
    };
    vcq_add(&cmd);

    VRAMUploadCommand bottom_cmd = {
        .address = address_base + 0x40,
        .increment = 1,
        .is_inline = true,
        .inline_data = {bl, br}
    };
    vcq_add(&bottom_cmd);
}
