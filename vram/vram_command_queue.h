#ifndef vram_command_queue_h
#define vram_command_queue_h

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint16_t address;
    uint16_t length;
    uint8_t increment;
    bool is_inline;

    union {
        const uint16_t *data;

        struct {
            uint16_t inline_data[2];
        };
    };
} VRAMUploadCommand;

void vcq_add(const VRAMUploadCommand *command);
void vcq_reset(void);
void vcq_run(void);

void vcq_add_16x16(uint16_t vram_base, uint16_t vram_tile, uint16_t source_tile, const uint16_t *data);
void vcq_add_2x2_map(uint16_t address_base, uint16_t tl, uint16_t tr, uint16_t bl, uint16_t br);

#endif /* vram_command_queue_h */
