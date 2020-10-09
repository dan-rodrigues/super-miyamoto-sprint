#include "music.h"

#include "audio.h"
#include "assert.h"
#include "math_util.h"

#include "audio_command_queue.h"

#ifdef MUSIC_INCLUDED

#include "track1_left.h"
#include "track1_right.h"
#include "track2_left.h"
#include "track2_right.h"

struct Track {
    const int16_t *left;
    const int16_t *right;
    const size_t *left_length;
    const size_t *right_length;
};

static const Track TRACK_LEVEL_1_DEF = {
    .left = track1_left,
    .right = track1_right,
    .left_length = &track1_left_length,
    .right_length = &track1_right_length
};

static const Track TRACK_CREDITS_DEF = {
    .left = track2_left,
    .right = track2_right,
    .left_length = &track2_left_length,
    .right_length = &track2_right_length
};

const Track * const TRACK_LEVEL_1 = &TRACK_LEVEL_1_DEF;
const Track * const TRACK_CREDITS = &TRACK_CREDITS_DEF;

static const int8_t VOLUME = 0x08;

#else

const Track * const TRACK_LEVEL_1 = NULL;
const Track * const TRACK_CREDITS = NULL;

#endif

void music_start(const Track *track) {
#ifdef MUSIC_INCLUDED
    assert(*track->left_length == *track->right_length);

    const uint8_t left_channel = 0;
    const uint8_t right_channel = 1;
    const uint16_t pitch = 0x800;

    AudioAlignedAddresses left_addresses;
    audio_aligned_addresses(track->left, *track->left_length, &left_addresses);

    AudioCommand left_cmd = {
        .channel = left_channel,
        .addresses = left_addresses,
        .volumes = {.left = VOLUME, .right = 0},
        .pitch = pitch,
        .flags = AUDIO_FLAG_LOOP
    };
    acq_add(&left_cmd);

    AudioAlignedAddresses right_addresses;
    audio_aligned_addresses(track->right, *track->right_length, &right_addresses);

    AudioCommand right_cmd = {
        .channel = right_channel,
        .addresses = right_addresses,
        .volumes = {.left = 0, .right = VOLUME},
        .pitch = pitch,
        .flags = AUDIO_FLAG_LOOP
    };
    acq_add(&right_cmd);
#endif
}

void music_scale_volume(uint8_t fraction) {
#ifdef MUSIC_INCLUDED
    int8_t scaled_volume = sys_multiply(VOLUME, fraction) / 256;
    AudioVolumes ch0_volumes = { .left = scaled_volume, .right = 0 };
    AudioVolumes ch1_volumes = { .left = 0, .right = scaled_volume };

    AUDIO->channels[0].volumes = ch0_volumes;
    AUDIO->channels[1].volumes = ch1_volumes;
#endif
}
