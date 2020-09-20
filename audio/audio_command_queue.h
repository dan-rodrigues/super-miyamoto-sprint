#ifndef audio_command_queue_h
#define audio_command_queue_h

#include <stdint.h>

#include "audio.h"

typedef struct {
    uint8_t channel;
    AudioAlignedAddresses addresses;
    AudioVolumes volumes;
    uint16_t pitch;
    uint8_t flags;
} AudioCommand;

void acq_reset(void);

void acq_add(const AudioCommand *cmd);
void acq_run(void);

#endif /* audio_command_queue_h */
