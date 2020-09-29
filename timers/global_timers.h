#ifndef global_timers_h
#define global_timers_h

#include <stdint.h>

uint32_t gt(void);
uint32_t gt_mod_2(void);
uint32_t gt_mod_4(void);
uint32_t gt_mod_8(void);

void gt_reset(void);
void gt_tick(void);

#endif /* global_timers_h */
