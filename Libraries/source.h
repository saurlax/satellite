#ifndef SOURCE_H
#define SOURCE_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Generate complex IQ sine source in interleaved format:
 * [I0, Q0, I1, Q1, ...]
 * iq_out buffer size must be length * 2.
 */
void sine_source(float *iq_out, uint32_t length, float freq, float sample_rate, float amplitude);

#ifdef __cplusplus
}
#endif

#endif // SOURCE_H
