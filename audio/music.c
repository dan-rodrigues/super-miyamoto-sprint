#include "music.h"

#include "audio.h"

#include "audio_command_queue.h"

#ifdef MUSIC_INCLUDED
#include "track1_left.h"
#include "track1_right.h"
#endif

void music_start() {
#ifdef MUSIC_INCLUDED
    const int8_t volume = 0x0c;
    const uint8_t left_channel = 0;
    const uint8_t right_channel = 1;
    const uint16_t pitch = 0x800;

    AudioAlignedAddresses left_addresses;
    audio_aligned_addresses(track1_left, track1_left_length, &left_addresses);

    AudioCommand left_cmd = {
        .channel = left_channel,
        .addresses = left_addresses,
        .volumes = {.left = volume, .right = 0},
        .pitch = pitch,
        .flags = AUDIO_FLAG_LOOP
    };
    acq_add(&left_cmd);

    AudioAlignedAddresses right_addresses;
    audio_aligned_addresses(track1_right, track1_right_length, &right_addresses);

    AudioCommand right_cmd = {
        .channel = right_channel,
        .addresses = right_addresses,
        .volumes = {.left = 0, .right = volume},
        .pitch = pitch,
        .flags = AUDIO_FLAG_LOOP
    };
    acq_add(&right_cmd);
#endif

}
