#include "audio_command_queue.h"

#include "math_util.h"
#include "assert.h"

#define COMMAND_QUEUE_MAX 6

static AudioCommand queue[COMMAND_QUEUE_MAX];
static uint8_t queue_index;

void acq_reset() {
    queue_index = 0;
}

void acq_add(const AudioCommand *cmd) {
    queue[queue_index] = *cmd;

    // If the queue is already full, bump the last-added command
    queue_index = MIN(queue_index + 1, COMMAND_QUEUE_MAX - 1);
}

void acq_run() {
    assert(queue_index < COMMAND_QUEUE_MAX);

    // There is enough time between frames but check just incase
    if (AUDIO_STATUS_BUSY) {
        return;
    }

    uint8_t channel_start_mask = 0;

    for (uint32_t i = 0; i < queue_index; i++) {
        const AudioCommand *cmd = &queue[i];

        volatile AudioChannel *ch = &AUDIO->channels[cmd->channel];

        ch->volumes.left = 0;
        ch->volumes.right = 0;

        ch->sample_start_address = cmd->addresses.start;
        ch->sample_end_address = cmd->addresses.end;
        ch->sample_loop_address = cmd->addresses.loop;
        ch->volumes = cmd->volumes;
        ch->pitch = cmd->pitch;
        ch->flags = cmd->flags;

        channel_start_mask |= (1 << cmd->channel);
    }

    if (channel_start_mask) {
        AUDIO_GB_PLAY = channel_start_mask;
    }

    acq_reset();
}
