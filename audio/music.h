#ifndef music_h
#define music_h

#include <stdint.h>

typedef struct Track Track;

extern const Track * const TRACK_LEVEL_1;
extern const Track * const TRACK_CREDITS;

void music_start(const Track *track);
void music_scale_volume(uint8_t fraction);

#endif /* music_h */
