#include "modem.h"
#include "arm_math.h"
#include <math.h>

#define TWO_PI_F 6.28318530717958647692f

static float
clamp_min (float value, float minValue)
{
  return (value < minValue) ? minValue : value;
}

static float
unwrap_phase (float phase, float *lastPhase, float *offset)
{
  float delta = phase - *lastPhase;

  if (delta > PI)
  {
    *offset -= TWO_PI_F;
  }
  else if (delta < -PI)
  {
    *offset += TWO_PI_F;
  }

  *lastPhase = phase;
  return phase + *offset;
}

/**
 * @brief Extract magnitude from complex I/Q sample
 * @param i Real component
 * @param q Imaginary component
 * @return sqrt(i^2 + q^2)
 */
static float
complex_magnitude (float i, float q)
{
  return sqrtf (i * i + q * q);
}

void
AM_Modulate (const float *message, float *iq_out, uint32_t length,
             float sampleRate, float carrierAmp, float modulationIndex)
{
  if ((message == 0) || (iq_out == 0) || (length == 0U))
  {
    return;
  }

  /* Complex baseband: s_bb[n] = A * (1 + m[n]) * e^(j*0) = A * (1 + m[n])
   * Output: I[n] = A*(1 + m[n]), Q[n] = 0 (in interleaved format) */

  for (uint32_t n = 0U; n < length; n++)
  {
    float envelope = 1.0f + modulationIndex * message[n];
    float i_sample = carrierAmp * envelope;
    float q_sample = 0.0f;

    iq_out[2 * n] = i_sample;
    iq_out[2 * n + 1] = q_sample;
  }
}

void
AM_Demodulate (const float *iq_in, float *message, uint32_t length,
               float carrierAmp, float modulationIndex, float lpCutoffHz)
{
  if ((iq_in == 0) || (message == 0) || (length == 0U) || (carrierAmp == 0.0f)
      || (modulationIndex == 0.0f))
  {
    return;
  }

  /* Extract envelope from complex IQ: envelope = sqrt(I^2 + Q^2)
   * Demodulate: m[n] = (envelope / A) - 1 */

  float cutoff = clamp_min (lpCutoffHz, 1.0f);

  /* Note: lpCutoffHz not directly used here, can be applied as preprocessing */
  (void)cutoff; /* Suppress unused warning if lpCutoffHz filtering not implemented */

  for (uint32_t n = 0U; n < length; n++)
  {
    float i_sample = iq_in[2 * n];
    float q_sample = iq_in[2 * n + 1];

    float envelope = complex_magnitude (i_sample, q_sample);
    message[n] = (envelope / carrierAmp) - 1.0f;
  }
}

void
PM_Modulate (const float *message, float *iq_out, uint32_t length,
             float sampleRate, float carrierAmp, float phaseSensitivity)
{
  if ((message == 0) || (iq_out == 0) || (length == 0U))
  {
    return;
  }

  /* Complex baseband: s_bb[n] = A * e^(j * k_p * m[n])
   * Output: I[n] = A*cos(k_p*m[n]), Q[n] = A*sin(k_p*m[n]) */

  (void)sampleRate; /* Not used in baseband PM */

  for (uint32_t n = 0U; n < length; n++)
  {
    float phase = phaseSensitivity * message[n];
    float i_sample = carrierAmp * arm_cos_f32 (phase);
    float q_sample = carrierAmp * arm_sin_f32 (phase);

    iq_out[2 * n] = i_sample;
    iq_out[2 * n + 1] = q_sample;
  }
}

void
PM_Demodulate (const float *iq_in, float *message, uint32_t length,
               float phaseSensitivity)
{
  if ((iq_in == 0) || (message == 0) || (length == 0U)
      || (phaseSensitivity == 0.0f))
  {
    return;
  }

  /* Extract phase from complex IQ: phase = atan2(Q, I)
   * Demodulate: m[n] = phase / k_p */

  for (uint32_t n = 0U; n < length; n++)
  {
    float i_sample = iq_in[2 * n];
    float q_sample = iq_in[2 * n + 1];

    float phase = atan2f (q_sample, i_sample);
    message[n] = phase / phaseSensitivity;
  }
}

void
FM_Modulate (const float *message, float *iq_out, uint32_t length,
             float sampleRate, float carrierAmp, float freqSensitivity)
{
  if ((message == 0) || (iq_out == 0) || (length == 0U)
      || (sampleRate <= 0.0f))
  {
    return;
  }

  /* Complex baseband FM: s_bb[n] = A * e^(j * phi[n])
   * Phase accumulation: phi[n] = phi[n-1] + 2*pi*k_f*m[n]/Fs
   * Output: I[n] = A*cos(phi[n]), Q[n] = A*sin(phi[n]) */

  float phase = 0.0f;

  for (uint32_t n = 0U; n < length; n++)
  {
    /* Accumulate phase based on instantaneous frequency deviation */
    phase += TWO_PI_F * freqSensitivity * message[n] / sampleRate;

    float i_sample = carrierAmp * arm_cos_f32 (phase);
    float q_sample = carrierAmp * arm_sin_f32 (phase);

    iq_out[2 * n] = i_sample;
    iq_out[2 * n + 1] = q_sample;
  }
}

void
FM_Demodulate (const float *iq_in, float *message, uint32_t length,
               float sampleRate, float freqSensitivity)
{
  if ((iq_in == 0) || (message == 0) || (length < 2U)
      || (sampleRate <= 0.0f) || (freqSensitivity == 0.0f))
  {
    return;
  }

  /* Extract phase from complex IQ and compute instantaneous frequency
   * phase[n] = atan2(Q[n], I[n])
   * freq_deviation[n] = (d_phase/d_sample) * (Fs / 2*pi) = d_phase * Fs / 2*pi
   * message[n] = freq_deviation / k_f */

  float lastPhase = 0.0f;
  float unwrapOffset = 0.0f;
  float prevUnwrapped = 0.0f;

  for (uint32_t n = 0U; n < length; n++)
  {
    float i_sample = iq_in[2 * n];
    float q_sample = iq_in[2 * n + 1];

    float rawPhase = atan2f (q_sample, i_sample);
    float unwrapped = unwrap_phase (rawPhase, &lastPhase, &unwrapOffset);

    if (n == 0U)
    {
      message[n] = 0.0f;
    }
    else
    {
      float dPhase = unwrapped - prevUnwrapped;
      float instFreq = (sampleRate * dPhase) / TWO_PI_F;
      message[n] = instFreq / freqSensitivity;
    }

    prevUnwrapped = unwrapped;
  }
}

/* ============================================================
 * Digital Modulation Stubs (Complex I/Q Baseband)
 * ============================================================ */

void
ASK_Modulate (const uint8_t *bits, uint32_t bitCount, float *iq_out,
              uint32_t samplesPerBit, float sampleRate,
              float amp0, float amp1)
{
  (void)bits;
  (void)bitCount;
  (void)iq_out;
  (void)samplesPerBit;
  (void)sampleRate;
  (void)amp0;
  (void)amp1;
  /* TODO: Implement ASK modulation with complex output */
}

void
ASK_Demodulate (const float *iq_in, uint8_t *bits,
                uint32_t bitCount, uint32_t samplesPerBit,
                float sampleRate, float decisionThreshold)
{
  (void)iq_in;
  (void)bits;
  (void)bitCount;
  (void)samplesPerBit;
  (void)sampleRate;
  (void)decisionThreshold;
  /* TODO: Implement ASK demodulation from complex input */
}

void
FSK_Modulate (const uint8_t *bits, uint32_t bitCount, float *iq_out,
              uint32_t samplesPerBit, float sampleRate, float freq0,
              float freq1)
{
  (void)bits;
  (void)bitCount;
  (void)iq_out;
  (void)samplesPerBit;
  (void)sampleRate;
  (void)freq0;
  (void)freq1;
  /* TODO: Implement FSK modulation with complex output */
}

void
FSK_Demodulate (const float *iq_in, uint8_t *bits,
                uint32_t bitCount, uint32_t samplesPerBit,
                float sampleRate, float freq0, float freq1)
{
  (void)iq_in;
  (void)bits;
  (void)bitCount;
  (void)samplesPerBit;
  (void)sampleRate;
  (void)freq0;
  (void)freq1;
  /* TODO: Implement FSK demodulation from complex input */
}

void
PSK_Modulate (const uint8_t *bits, uint32_t bitCount, float *iq_out,
              uint32_t samplesPerBit, float sampleRate,
              float carrierAmp)
{
  (void)bits;
  (void)bitCount;
  (void)iq_out;
  (void)samplesPerBit;
  (void)sampleRate;
  (void)carrierAmp;
  /* TODO: Implement PSK modulation with complex output */
}

void
PSK_Demodulate (const float *iq_in, uint8_t *bits,
                uint32_t bitCount, uint32_t samplesPerBit,
                float sampleRate)
{
  (void)iq_in;
  (void)bits;
  (void)bitCount;
  (void)samplesPerBit;
  (void)sampleRate;
  /* TODO: Implement PSK demodulation from complex input */
}

void
AFSK_Modulate (const uint8_t *bits, uint32_t bitCount, float *iq_out,
               uint32_t samplesPerBit, float sampleRate, float markFreq,
               float spaceFreq)
{
  (void)bits;
  (void)bitCount;
  (void)iq_out;
  (void)samplesPerBit;
  (void)sampleRate;
  (void)markFreq;
  (void)spaceFreq;
  /* TODO: Implement AFSK modulation with complex output */
}

void
AFSK_Demodulate (const float *iq_in, uint8_t *bits,
                 uint32_t bitCount, uint32_t samplesPerBit,
                 float sampleRate, float markFreq, float spaceFreq)
{
  (void)iq_in;
  (void)bits;
  (void)bitCount;
  (void)samplesPerBit;
  (void)sampleRate;
  (void)markFreq;
  (void)spaceFreq;
  /* TODO: Implement AFSK demodulation from complex input */
}
