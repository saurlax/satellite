#ifndef MODEM_H
#define MODEM_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

  void AM_Modulate (const float *message, float *modulated, uint32_t length,
                    float sampleRate, float carrierFreq, float carrierAmp,
                    float modulationIndex);

  void AM_Demodulate (const float *modulated, float *message, uint32_t length,
                      float sampleRate, float carrierFreq, float carrierAmp,
                      float modulationIndex, float lpCutoffHz);

  void PM_Modulate (const float *message, float *modulated, uint32_t length,
                    float sampleRate, float carrierFreq, float carrierAmp,
                    float phaseSensitivity);

  void PM_Demodulate (const float *modulated, float *message, uint32_t length,
                      float sampleRate, float carrierFreq,
                      float phaseSensitivity);

  void FM_Modulate (const float *message, float *modulated, uint32_t length,
                    float sampleRate, float carrierFreq, float carrierAmp,
                    float freqSensitivity);

  void FM_Demodulate (const float *modulated, float *message, uint32_t length,
                      float sampleRate, float carrierFreq,
                      float freqSensitivity);

  void ASK_Modulate (const uint8_t *bits, uint32_t bitCount, float *modulated,
                     uint32_t samplesPerBit, float sampleRate,
                     float carrierFreq, float amp0, float amp1);

  void ASK_Demodulate (const float *modulated, uint8_t *bits,
                       uint32_t bitCount, uint32_t samplesPerBit,
                       float sampleRate, float carrierFreq,
                       float decisionThreshold);

  void FSK_Modulate (const uint8_t *bits, uint32_t bitCount, float *modulated,
                     uint32_t samplesPerBit, float sampleRate, float freq0,
                     float freq1, float carrierAmp);

  void FSK_Demodulate (const float *modulated, uint8_t *bits,
                       uint32_t bitCount, uint32_t samplesPerBit,
                       float sampleRate, float freq0, float freq1);

  void PSK_Modulate (const uint8_t *bits, uint32_t bitCount, float *modulated,
                     uint32_t samplesPerBit, float sampleRate,
                     float carrierFreq, float carrierAmp);

  void PSK_Demodulate (const float *modulated, uint8_t *bits,
                       uint32_t bitCount, uint32_t samplesPerBit,
                       float sampleRate, float carrierFreq);

  void AFSK_Modulate (const uint8_t *bits, uint32_t bitCount, float *modulated,
                      uint32_t samplesPerBit, float sampleRate, float markFreq,
                      float spaceFreq, float toneAmp);

  void AFSK_Demodulate (const float *modulated, uint8_t *bits,
                        uint32_t bitCount, uint32_t samplesPerBit,
                        float sampleRate, float markFreq, float spaceFreq);

#ifdef __cplusplus
}
#endif

#endif
