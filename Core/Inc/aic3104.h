#ifndef AIC3104_H
#define AIC3104_H

#include "stm32h7xx_hal.h"
#include <stdbool.h>

/* 7-bit I2C address when ADDR pin = GND */
#define AIC3104_I2C_ADDR_0  0x18U

/* Register map (Page 0) */
#define AIC3104_REG_PAGE_SELECT               0x00U
#define AIC3104_REG_SOFT_RESET                0x01U
#define AIC3104_REG_SAMPLE_RATE               0x02U
#define AIC3104_REG_PLL_A                     0x03U
#define AIC3104_REG_PLL_B                     0x04U
#define AIC3104_REG_PLL_C                     0x05U
#define AIC3104_REG_PLL_D                     0x06U
#define AIC3104_REG_CODEC_DATAPATH            0x07U
#define AIC3104_REG_ASD_IF_A                  0x08U
#define AIC3104_REG_ASD_IF_B                  0x09U
#define AIC3104_REG_ASD_IF_C                  0x0AU
#define AIC3104_REG_OVRF_PLLR                 0x0BU
#define AIC3104_REG_CODEC_DFILT               0x0CU
#define AIC3104_REG_HPDET_A                   0x0DU
#define AIC3104_REG_HPDET_B                   0x0EU
#define AIC3104_REG_LADC_VOL                  0x0FU
#define AIC3104_REG_RADC_VOL                  0x10U
#define AIC3104_REG_MIC3_TO_LADC              0x11U
#define AIC3104_REG_MIC3_TO_RADC              0x12U
#define AIC3104_REG_LINE1L_TO_LADC            0x13U
#define AIC3104_REG_LINE2L_TO_LADC            0x14U
#define AIC3104_REG_LINE1R_TO_LADC            0x15U
#define AIC3104_REG_LINE1R_TO_RADC            0x16U
#define AIC3104_REG_LINE2R_TO_RADC            0x17U
#define AIC3104_REG_LINE1L_TO_RADC            0x18U
#define AIC3104_REG_MICBIAS                   0x19U
#define AIC3104_REG_LAGC_A                    0x1AU
#define AIC3104_REG_RAGC_A                    0x1DU
#define AIC3104_REG_DAC_PWR                   0x25U
#define AIC3104_REG_HPRCOM_CFG                0x26U
#define AIC3104_REG_HPOUT_SC                  0x28U
#define AIC3104_REG_DAC_LINE_MUX              0x29U
#define AIC3104_REG_HPOUT_POP                 0x2AU
#define AIC3104_REG_LDAC_VOL                  0x2BU
#define AIC3104_REG_RDAC_VOL                  0x2CU
#define AIC3104_REG_PGAL_TO_HPLOUT            0x2EU
#define AIC3104_REG_DACL1_TO_HPLOUT           0x2FU
#define AIC3104_REG_PGAR_TO_HPLOUT            0x31U
#define AIC3104_REG_DACR1_TO_HPLOUT           0x32U
#define AIC3104_REG_HPLOUT_CTRL               0x33U
#define AIC3104_REG_PGAL_TO_HPLCOM            0x35U
#define AIC3104_REG_DACL1_TO_HPLCOM           0x36U
#define AIC3104_REG_PGAR_TO_HPLCOM            0x38U
#define AIC3104_REG_DACR1_TO_HPLCOM           0x39U
#define AIC3104_REG_HPLCOM_CTRL               0x3AU
#define AIC3104_REG_PGAL_TO_HPROUT            0x3CU
#define AIC3104_REG_DACL1_TO_HPROUT           0x3DU
#define AIC3104_REG_PGAR_TO_HPROUT            0x3FU
#define AIC3104_REG_DACR1_TO_HPROUT           0x40U
#define AIC3104_REG_HPROUT_CTRL               0x41U
#define AIC3104_REG_PGAL_TO_HPRCOM            0x43U
#define AIC3104_REG_DACL1_TO_HPRCOM           0x44U
#define AIC3104_REG_PGAR_TO_HPRCOM            0x46U
#define AIC3104_REG_DACR1_TO_HPRCOM           0x47U
#define AIC3104_REG_HPRCOM_CTRL               0x48U
#define AIC3104_REG_PGAL_TO_LLOPM             0x51U
#define AIC3104_REG_DACL1_TO_LLOPM            0x52U
#define AIC3104_REG_PGAR_TO_LLOPM             0x54U
#define AIC3104_REG_DACR1_TO_LLOPM            0x55U
#define AIC3104_REG_LLOPM_CTRL                0x56U
#define AIC3104_REG_PGAL_TO_RLOPM             0x58U
#define AIC3104_REG_DACL1_TO_RLOPM            0x59U
#define AIC3104_REG_PGAR_TO_RLOPM             0x5BU
#define AIC3104_REG_DACR1_TO_RLOPM            0x5CU
#define AIC3104_REG_RLOPM_CTRL                0x5DU
#define AIC3104_REG_CLKGEN_CTRL               0x66U

/* Common bit definitions */
#define AIC3104_SOFT_RESET                    0x80U
#define AIC3104_PLL_ENABLE                    0x80U
#define AIC3104_ROUTE_ON                      0x80U

#define AIC3104_DAC_PWR_L_ON                  0x80U
#define AIC3104_DAC_PWR_R_ON                  0x40U

#define AIC3104_ADC_PWR_ON                    0x04U
#define AIC3104_ADC_MUTE_BIT                  0x80U
#define AIC3104_DAC_MUTE_BIT                  0x80U

#define AIC3104_OUTPUT_PWR_BIT                0x01U
#define AIC3104_OUTPUT_SWITCH_BIT             0x08U
#define AIC3104_OUTPUT_VOL_MASK               0xF0U
#define AIC3104_OUTPUT_VOL_SHIFT              4U

/* Audio sampling rates */
typedef enum {
    AIC3104_FS_8K   = 8000,
    AIC3104_FS_16K  = 16000,
    AIC3104_FS_44K  = 44100,
    AIC3104_FS_48K  = 48000
} AIC3104_SampleRate_e;

/* Codec BCLK/WCLK ownership */
typedef enum {
    AIC3104_MODE_MASTER = 0,
    AIC3104_MODE_SLAVE  = 1
} AIC3104_I2SMode_e;

typedef enum {
    AIC3104_CLKIN_MCLK = 0,
    AIC3104_CLKIN_GPIO2 = 1,
    AIC3104_CLKIN_BCLK = 2
} AIC3104_ClockInput_e;

typedef enum {
    AIC3104_OUTPUT_HP = 0,
    AIC3104_OUTPUT_LINE,
    AIC3104_OUTPUT_HPCOM
} AIC3104_OutputType_e;

typedef enum {
    AIC3104_MIXER_PGAL = 0,
    AIC3104_MIXER_DACL1,
    AIC3104_MIXER_PGAR,
    AIC3104_MIXER_DACR1
} AIC3104_MixerInput_e;

typedef enum {
    AIC3104_ADC_LEFT = 0,
    AIC3104_ADC_RIGHT,
    AIC3104_ADC_BOTH
} AIC3104_ADCChannel_e;

typedef enum {
    AIC3104_INPUT_LINE1L = 0,
    AIC3104_INPUT_LINE1R,
    AIC3104_INPUT_MIC2L,
    AIC3104_INPUT_MIC2R
} AIC3104_Input_e;

typedef enum {
    AIC3104_HPF_DISABLED = 0,
    AIC3104_HPF_0P0045_FS = 1,
    AIC3104_HPF_0P0125_FS = 2,
    AIC3104_HPF_0P025_FS = 3
} AIC3104_ADCHpf_e;

typedef enum {
    AIC3104_MICBIAS_OFF = 0,
    AIC3104_MICBIAS_2V0 = 1,
    AIC3104_MICBIAS_2V5 = 2,
    AIC3104_MICBIAS_AVDD = 3
} AIC3104_MicBias_e;

typedef struct {
    uint8_t sample_rate_reg;
    uint8_t codec_datapath_reg;
    uint8_t asd_if_a_reg;
    uint8_t asd_if_b_reg;
    uint8_t asd_if_c_reg;
    uint8_t pll_a_reg;
    uint8_t pll_b_reg;
    uint8_t pll_c_reg;
    uint8_t pll_d_reg;
    uint8_t pllr_reg;
    uint8_t clkgen_reg;
} AIC3104_ClockConfig_t;

/* AIC3104 Configuration Structure */
typedef struct {
    I2C_HandleTypeDef    *hi2c;
    I2S_HandleTypeDef    *hi2s;
    GPIO_TypeDef         *reset_port;
    uint16_t              reset_pin;
    uint8_t               i2c_addr_7bit;
    uint32_t              i2c_timeout_ms;

    uint32_t              mclk_freq;
    AIC3104_SampleRate_e  sample_rate;
    AIC3104_I2SMode_e     i2s_mode;
    AIC3104_ClockInput_e  clock_input;

    AIC3104_ClockConfig_t clock;

    uint8_t               dac_volume_left;
    uint8_t               dac_volume_right;
    uint8_t               adc_pga_left;
    uint8_t               adc_pga_right;

    AIC3104_MicBias_e     micbias;
    uint8_t               output_common_mode;

    bool                  enable_adc;
    bool                  enable_dac;
    bool                  enable_hp;
    bool                  enable_lineout;
    bool                  enable_hpcom;
} AIC3104_Config_t;

/* Initialization and low-level IO */
void AIC3104_DefaultConfig(AIC3104_Config_t *config, I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef AIC3104_Init(AIC3104_Config_t *config);
HAL_StatusTypeDef AIC3104_WriteReg(AIC3104_Config_t *config, uint8_t reg, uint8_t val);
HAL_StatusTypeDef AIC3104_ReadReg(AIC3104_Config_t *config, uint8_t reg, uint8_t *val);
HAL_StatusTypeDef AIC3104_UpdateBits(AIC3104_Config_t *config, uint8_t reg, uint8_t mask, uint8_t val);
HAL_StatusTypeDef AIC3104_HardwareReset(AIC3104_Config_t *config);

/* Core function blocks */
HAL_StatusTypeDef AIC3104_ApplyClockConfig(AIC3104_Config_t *config);
HAL_StatusTypeDef AIC3104_SetDacMute(AIC3104_Config_t *config, bool mute);
HAL_StatusTypeDef AIC3104_SetAdcMute(AIC3104_Config_t *config, AIC3104_ADCChannel_e channel, bool mute);
HAL_StatusTypeDef AIC3104_SetDacVolume(AIC3104_Config_t *config, uint8_t left, uint8_t right);
HAL_StatusTypeDef AIC3104_SetAdcPga(AIC3104_Config_t *config, uint8_t left, uint8_t right);
HAL_StatusTypeDef AIC3104_SetAdcHpf(AIC3104_Config_t *config, AIC3104_ADCHpf_e hpf);
HAL_StatusTypeDef AIC3104_SetDeemphasis(AIC3104_Config_t *config, bool left_enable, bool right_enable);
HAL_StatusTypeDef AIC3104_SetMicBias(AIC3104_Config_t *config, AIC3104_MicBias_e micbias);
HAL_StatusTypeDef AIC3104_SetAgc(AIC3104_Config_t *config,
                                  AIC3104_ADCChannel_e channel,
                                  bool enable,
                                  uint8_t target_level,
                                  uint8_t attack_time,
                                  uint8_t decay_time);
HAL_StatusTypeDef AIC3104_SetInputRoute(AIC3104_Config_t *config,
                                         AIC3104_ADCChannel_e adc,
                                         AIC3104_Input_e input,
                                         bool differential,
                                         bool enable);
HAL_StatusTypeDef AIC3104_SetMixerRoute(AIC3104_Config_t *config,
                                         AIC3104_OutputType_e output,
                                         bool right_channel,
                                         AIC3104_MixerInput_e source,
                                         bool enable,
                                         uint8_t gain);
HAL_StatusTypeDef AIC3104_SetOutputDriver(AIC3104_Config_t *config,
                                           AIC3104_OutputType_e output,
                                           bool right_channel,
                                           bool power_on,
                                           bool unmute,
                                           uint8_t volume_step);

#endif /* AIC3104_H */