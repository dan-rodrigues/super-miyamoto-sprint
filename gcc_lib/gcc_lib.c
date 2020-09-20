#include "gcc_lib.h"

#include <stdint.h>

// Using for / while loops here generated suboptimal code
// The do-while wrapped in an if statement looks odd but the inner loop is tighter

__attribute__ ((used))
__attribute__((optimize("-fno-strict-aliasing")))
void *memcpy(void *s1, const void *s2, size_t n) {
    // 32bit aligned part

    uint32_t *s1_32 = s1;
    const uint32_t *s2_32 = s2;

    if (!(((uintptr_t)s1 | (uintptr_t)s2) & 3) && n >= 4) {
        uint32_t copied_words = n / 4;
        const uint32_t * const s1_end = s1_32 + copied_words;

         do {
            *s1_32++ = *s2_32++;
         } while (s1_32 < s1_end);

        n -= copied_words * 4;
    }

    // Unaligned part

    char *c1 = (char *)s1_32;
    const char *c2 = (const char *)s2_32;

    const char * const c1_end = c1 + n;

    if (n > 0) {
        do {
            *c1++ = *c2++;
        } while (c1 < c1_end);
    }

    return s1;
}

__attribute__ ((used))
__attribute__((optimize("-fno-strict-aliasing")))
void *memset(void *dest, int val, size_t len) {
    // 32bit aligned part

    uint32_t *dest_32 = dest;

    if (len >= 16 && !((uintptr_t)dest & 3)) {
        uint32_t copied_words = len / 4;
        const uint32_t * const dest_end = dest_32 + copied_words;

        uint32_t val_32 = val;
        val_32 <<= 8;
        val_32 |= val;
        val_32 <<= 8;
        val_32 |= val;
        val_32 <<= 8;
        val_32 |= val;

         do {
            *dest_32++ = val_32;
         } while (dest_32 < dest_end);

        len -= copied_words * 4;
    }

    // Unaligned part

    char *c1 = (char *)dest_32;
    const char * const c1_end = c1 + len;

    if (len > 0) {
        do {
            *c1++ = val;
        } while (c1 < c1_end);
    }

    return dest;
}

// __mulsi3 and friends...
