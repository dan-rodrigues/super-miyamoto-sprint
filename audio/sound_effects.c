#include "sound_effects.h"

#include "audio.h"

#include "audio_command_queue.h"
#include "jump.h"
#include "coin.h"
#include "thud.h"
#include "stomp.h"
#include "hurt.h"
#include "launch.h"
#include "explosion.h"
#include "powerup.h"
#include "dead.h"

static void queue_sound(const int16_t *data, size_t length, int8_t volume, uint8_t channel);

void se_jump() {
    const uint8_t channel = 2;
    queue_sound(jump, jump_length, 0x10, channel);
}

void se_stomp() {
    const uint8_t channel = 2;
    queue_sound(stomp, stomp_length, 0x10, channel);
}

void se_coin() {
    const uint8_t channel = 3;
    queue_sound(coin, coin_length, 0x08, channel);
}

void se_thud() {
    const uint8_t channel = 4;
    queue_sound(thud, thud_length, 0x0c, channel);
}

void se_explosion() {
    const uint8_t channel = 5;
    queue_sound(explosion, explosion_length, 0x08, channel);
}

void se_powerup() {
    const uint8_t channel = 6;
    queue_sound(powerup, powerup_length, 0x08, channel);
}

void se_hero_hurt() {
    const uint8_t channel = 6;
    queue_sound(hurt, hurt_length, 0x10, channel);
}

// Could use a pitch slide down instead of just storing another sample
void se_hero_dead() {
    const uint8_t channel = 6;
    queue_sound(dead, dead_length, 0x10, channel);
}

void se_missile_launch() {
    const uint8_t channel = 7;
    queue_sound(launch, launch_length, 0x08, channel);
}

static void queue_sound(const int16_t *data, size_t length, int8_t volume, uint8_t channel) {
    AudioAlignedAddresses addresses;
    audio_aligned_addresses(data, length, &addresses);

    AudioCommand cmd = {
        .channel = channel,
        .addresses = addresses,
        .volumes = {volume, volume},
        .pitch = 0x1000,
        .flags = 0
    };

    acq_add(&cmd);
}
