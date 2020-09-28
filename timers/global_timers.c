#include "global_timers.h"

static uint32_t shared_timer;

static uint32_t timer_mod_2;
static uint32_t timer_mod_4;
static uint32_t timer_mod_8;

uint32_t gt_mod_2() {
    return timer_mod_2;
}

uint32_t gt_mod_4() {
    return timer_mod_4;
}

uint32_t gt_mod_8() {
    return timer_mod_8;
}

void gt_tick() {
    shared_timer++;

    timer_mod_2 = shared_timer % 2;
    timer_mod_4 = shared_timer % 4;
    timer_mod_8 = shared_timer % 8;
}

void gt_reset() {
    shared_timer = 0;
    gt_tick();
}
