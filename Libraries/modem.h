#ifndef MODEM_H
#define MODEM_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

  /**
   * @brief Analog Modulation (Complex I/Q baseband output)
   * 
   * All modulation functions output complex I/Q samples in interleaved format:
   * [I0, Q0, I1, Q1, I2, Q2, ...]
   * 
   * Output buffer size must be: length * 2 (for I and Q samples)
   */

  void AM_Modulate (const float *message, float *iq_out, uint32_t length,
                    float sampleRate, float carrierAmp,
                    float modulationIndex);

  void AM_Demodulate (const float *iq_in, float *message, uint32_t length,
                      float carrierAmp, float modulationIndex, 
                      float lpCutoffHz);

  void PM_Modulate (const float *message, float *iq_out, uint32_t length,
                    float sampleRate, float carrierAmp,
                    float phaseSensitivity);

  void PM_Demodulate (const float *iq_in, float *message, uint32_t length,
                      float phaseSensitivity);

  void FM_Modulate (const float *message, float *iq_out, uint32_t length,
                    float sampleRate, float carrierAmp,
                    float freqSensitivity);

  void FM_Demodulate (const float *iq_in, float *message, uint32_t length,
                      float sampleRate, float freqSensitivity);

  /**
   * @brief Digital Modulation (Complex I/Q baseband output)
   * 
   * Output buffer size must be: bitCount * samplesPerBit * 2 (for I and Q samples)
   */

  void ASK_Modulate (const uint8_t *bits, uint32_t bitCount, float *iq_out,
                     uint32_t samplesPerBit, float sampleRate,
                     float amp0, float amp1);

  void ASK_Demodulate (const float *iq_in, uint8_t *bits,
                       uint32_t bitCount, uint32_t samplesPerBit,
                       float sampleRate, float decisionThreshold);

  void FSK_Modulate (const uint8_t *bits, uint32_t bitCount, float *iq_out,
                     uint32_t samplesPerBit, float sampleRate, float freq0,
                     float freq1);

  void FSK_Demodulate (const float *iq_in, uint8_t *bits,
                       uint32_t bitCount, uint32_t samplesPerBit,
                       float sampleRate, float freq0, float freq1);

  void PSK_Modulate (const uint8_t *bits, uint32_t bitCount, float *iq_out,
                     uint32_t samplesPerBit, float sampleRate,
                     float carrierAmp);

  void PSK_Demodulate (const float *iq_in, uint8_t *bits,
                       uint32_t bitCount, uint32_t samplesPerBit,
                       float sampleRate);

  void AFSK_Modulate (const uint8_t *bits, uint32_t bitCount, float *iq_out,
                      uint32_t samplesPerBit, float sampleRate, float markFreq,
                      float spaceFreq);

  void AFSK_Demodulate (const float *iq_in, uint8_t *bits,
                        uint32_t bitCount, uint32_t samplesPerBit,
                        float sampleRate, float markFreq, float spaceFreq);

#ifdef __cplusplus
}
#endif

#endif
