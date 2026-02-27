#include "source.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

void sine_source(float *iq_out, uint32_t length, float freq, float sample_rate, float amplitude)
{
    for (uint32_t i = 0U; i < length; ++i) {
        float t = (float)i / sample_rate;
        float phase = 2.0f * M_PI * freq * t;
        iq_out[2U * i] = amplitude * cosf(phase);
        iq_out[(2U * i) + 1U] = amplitude * sinf(phase);
    }
}
