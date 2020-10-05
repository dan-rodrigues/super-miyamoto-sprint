#ifndef affine_h
#define affine_h

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    int16_t scaled_sin;
    int16_t scaled_cos;

    int16_t angle;
    uint8_t animation_acc;
    uint8_t animation_index;

    bool enable_sprites;
} AffineHeroContext;

void al_init(AffineHeroContext *context);

void al_prepare_update(AffineHeroContext *context);
void al_frame_ended_update(void);

void al_upload(void);

void al_set_copper(const AffineHeroContext *context);

#endif /* affine_h */
