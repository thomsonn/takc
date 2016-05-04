#include <stdint.h>

#include "fastlog.h"

float fastlog2(float x)
{
    union {float f; uint32_t x;} u = {x};
    float y = (float) ((u.x >> 23 & 255) - 128);

    u.x &= ~(255 << 23);
    u.x += 127 << 23;
    y += ((-0.34484843)*u.f + 2.02466578)*u.f - 0.67487759;

    return y;
}
