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

void
AM_Modulate (const float *message, float *modulated, uint32_t length,
             float sampleRate, float carrierFreq, float carrierAmp,
             float modulationIndex)
{
  if ((message == 0) || (modulated == 0) || (length == 0U)
      || (sampleRate <= 0.0f))
  {
    return;
  }

  float phaseStep = TWO_PI_F * carrierFreq / sampleRate;

  for (uint32_t n = 0U; n < length; n++)
  {
    float carrier = arm_cos_f32 (phaseStep * (float)n);
    float envelope = 1.0f + modulationIndex * message[n];
    modulated[n] = carrierAmp * envelope * carrier;
  }
}

void
AM_Demodulate (const float *modulated, float *message, uint32_t length,
               float sampleRate, float carrierFreq, float carrierAmp,
               float modulationIndex, float lpCutoffHz)
{
  if ((modulated == 0) || (message == 0) || (length == 0U)
      || (sampleRate <= 0.0f) || (carrierAmp == 0.0f)
      || (modulationIndex == 0.0f))
  {
    return;
  }

  float cutoff = clamp_min (lpCutoffHz, 1.0f);
  float alpha = TWO_PI_F * cutoff / (TWO_PI_F * cutoff + sampleRate);
  float phaseStep = TWO_PI_F * carrierFreq / sampleRate;
  float lowpass = 0.0f;

  for (uint32_t n = 0U; n < length; n++)
  {
    float lo = arm_cos_f32 (phaseStep * (float)n);
    float mixed = 2.0f * modulated[n] * lo;
    lowpass += alpha * (mixed - lowpass);
    message[n] = ((lowpass / carrierAmp) - 1.0f) / modulationIndex;
  }
}

void
PM_Modulate (const float *message, float *modulated, uint32_t length,
             float sampleRate, float carrierFreq, float carrierAmp,
             float phaseSensitivity)
{
  if ((message == 0) || (modulated == 0) || (length == 0U)
      || (sampleRate <= 0.0f))
  {
    return;
  }

  float phaseStep = TWO_PI_F * carrierFreq / sampleRate;

  for (uint32_t n = 0U; n < length; n++)
  {
    float phase = (phaseStep * (float)n) + phaseSensitivity * message[n];
    modulated[n] = carrierAmp * arm_cos_f32 (phase);
  }
}

void
PM_Demodulate (const float *modulated, float *message, uint32_t length,
               float sampleRate, float carrierFreq, float phaseSensitivity)
{
  if ((modulated == 0) || (message == 0) || (length == 0U)
      || (sampleRate <= 0.0f) || (phaseSensitivity == 0.0f))
  {
    return;
  }

  float phaseStep = TWO_PI_F * carrierFreq / sampleRate;
  float lastPhase = 0.0f;
  float unwrapOffset = 0.0f;

  for (uint32_t n = 0U; n < length; n++)
  {
    float carrierPhase = phaseStep * (float)n;
    float inSample = modulated[n];

    float iVal = inSample * arm_cos_f32 (carrierPhase);
    float qVal = -inSample * arm_sin_f32 (carrierPhase);

    float rawPhase = atan2f (qVal, iVal);
    float unwrapped = unwrap_phase (rawPhase, &lastPhase, &unwrapOffset);

    message[n] = unwrapped / phaseSensitivity;
  }
}

void
FM_Modulate (const float *message, float *modulated, uint32_t length,
             float sampleRate, float carrierFreq, float carrierAmp,
             float freqSensitivity)
{
  if ((message == 0) || (modulated == 0) || (length == 0U)
      || (sampleRate <= 0.0f))
  {
    return;
  }

  float phase = 0.0f;

  for (uint32_t n = 0U; n < length; n++)
  {
    float instFreq = carrierFreq + freqSensitivity * message[n];
    phase += TWO_PI_F * instFreq / sampleRate;
    modulated[n] = carrierAmp * arm_cos_f32 (phase);
  }
}

void
FM_Demodulate (const float *modulated, float *message, uint32_t length,
               float sampleRate, float carrierFreq, float freqSensitivity)
{
  if ((modulated == 0) || (message == 0) || (length < 2U)
      || (sampleRate <= 0.0f) || (freqSensitivity == 0.0f))
  {
    return;
  }

  float phaseStep = TWO_PI_F * carrierFreq / sampleRate;
  float lastPhase = 0.0f;
  float unwrapOffset = 0.0f;
  float prevUnwrapped = 0.0f;

  for (uint32_t n = 0U; n < length; n++)
  {
    float carrierPhase = phaseStep * (float)n;
    float inSample = modulated[n];

    float iVal = inSample * arm_cos_f32 (carrierPhase);
    float qVal = -inSample * arm_sin_f32 (carrierPhase);

    float rawPhase = atan2f (qVal, iVal);
    float unwrapped = unwrap_phase (rawPhase, &lastPhase, &unwrapOffset);

    if (n == 0U)
    {
      message[n] = 0.0f;
    }
    else
    {
      float dPhase = unwrapped - prevUnwrapped;
      float instFreq = (sampleRate * dPhase) / TWO_PI_F;
      message[n] = (instFreq - carrierFreq) / freqSensitivity;
    }

    prevUnwrapped = unwrapped;
  }
}
