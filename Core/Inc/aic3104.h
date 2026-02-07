#ifndef AIC3104_H
#define AIC3104_H

#include "stm32h7xx_hal.h"

/* AIC3104 I2C Address (7-bit) - depends on ADDR pin level */
#define AIC3104_I2C_ADDR  0x18

/* Audio sampling rates */
typedef enum {
    AIC3104_FS_8K   = 8000,
    AIC3104_FS_16K  = 16000,
    AIC3104_FS_44K  = 44100,
    AIC3104_FS_48K  = 48000
} AIC3104_SampleRate_e;

/* I2S Mode */
typedef enum {
    AIC3104_MODE_MASTER = 0,
    AIC3104_MODE_SLAVE  = 1
} AIC3104_I2SMode_e;

/* AIC3104 Configuration Structure */
typedef struct {
    I2C_HandleTypeDef    *hi2c;           /* I2C handle for register access */
    I2S_HandleTypeDef    *hi2s;           /* I2S handle for audio transmission */
    GPIO_TypeDef         *reset_port;     /* GPIO port for RESET pin */
    uint16_t             reset_pin;       /* GPIO pin for RESET */
    
    uint32_t             mclk_freq;       /* MCLK frequency in Hz */
    AIC3104_SampleRate_e sample_rate;    /* Audio sampling rate */
    AIC3104_I2SMode_e    i2s_mode;       /* I2S Master/Slave mode */
} AIC3104_Config_t;

/* Function Prototypes */
HAL_StatusTypeDef AIC3104_Init(AIC3104_Config_t *config);
HAL_StatusTypeDef AIC3104_WriteReg(AIC3104_Config_t *config, uint8_t reg, uint8_t val);
HAL_StatusTypeDef AIC3104_ReadReg(AIC3104_Config_t *config, uint8_t reg, uint8_t *val);
HAL_StatusTypeDef AIC3104_HardwareReset(AIC3104_Config_t *config);
HAL_StatusTypeDef AIC3104_SetMute(AIC3104_Config_t *config, uint8_t mute);
HAL_StatusTypeDef AIC3104_SetVolume(AIC3104_Config_t *config, uint8_t volume_left, uint8_t volume_right);

#endif /* AIC3104_H */