#ifndef ADF7021_H
#define ADF7021_H

#include "stm32h7xx_hal.h"

/* ADF7021 Register Definitions */
#define ADF7021_REG_ADDR(reg_num)    ((uint32_t)(reg_num) << 25)

/* Calibration Status */
typedef enum {
    ADF7021_CAL_IDLE = 0,
    ADF7021_CAL_IF_COARSE = 1,
    ADF7021_CAL_IF_FINE = 2,
    ADF7021_CAL_IR = 3
} ADF7021_CalStatus_e;

/* Modulation Type */
typedef enum {
    ADF7021_MOD_2FSK = 0,
    ADF7021_MOD_4FSK = 1,
    ADF7021_MOD_GFSK = 2,
    ADF7021_MOD_OOK = 3
} ADF7021_ModType_e;

/* Transmit/Receive Mode */
typedef enum {
    ADF7021_MODE_RX = 0,
    ADF7021_MODE_TX = 1
} ADF7021_TxRxMode_e;

/* Power Amplifier Level */
typedef enum {
    ADF7021_PA_POWER_0dBm = 0,
    ADF7021_PA_POWER_3dBm = 1,
    ADF7021_PA_POWER_6dBm = 2,
    ADF7021_PA_POWER_10dBm = 3,
    ADF7021_PA_POWER_13dBm = 4,
    ADF7021_PA_POWER_16dBm = 5,
    ADF7021_PA_POWER_19dBm = 6,
    ADF7021_PA_POWER_MAX = 7
} ADF7021_PALevel_e;

/* ADF7021 Configuration Structure */
typedef struct {
    SPI_HandleTypeDef    *hspi;           /* SPI handle for register access */
    GPIO_TypeDef         *en_port;        /* GPIO port for EN (CE) pin */
    uint16_t             en_pin;          /* GPIO pin for EN (CE) */
    
    /* Crystal/TCXO Configuration */
    uint32_t             xtal_freq_hz;    /* Crystal frequency in Hz */
    uint8_t              xtal_type;       /* 0: Crystal, 1: TCXO (external ref) */
    
    /* Frequency Configuration */
    uint32_t             center_freq_hz;  /* Center frequency in Hz */
    uint32_t             ref_freq_hz;     /* Reference frequency (usually XTAL or 2*XTAL) */
    
    /* Data Rate Configuration */
    uint32_t             data_rate_bps;   /* Data rate in bits per second */
    
    /* Modulation Configuration */
    ADF7021_ModType_e    mod_type;        /* Modulation type (2FSK, 4FSK, etc.) */
    uint32_t             freq_deviation;  /* Frequency deviation in Hz */
    
    /* TX Configuration */
    ADF7021_PALevel_e    tx_power;        /* TX power level */
    
    /* RX Configuration */
    uint32_t             if_filter_bw;    /* IF filter bandwidth in Hz */
    
} ADF7021_Config_t;

/* Function Prototypes */
HAL_StatusTypeDef ADF7021_Init(ADF7021_Config_t *config);
HAL_StatusTypeDef ADF7021_WriteReg(ADF7021_Config_t *config, uint8_t reg_num, uint32_t reg_data);
uint32_t ADF7021_ReadReg(ADF7021_Config_t *config);
HAL_StatusTypeDef ADF7021_SetTxMode(ADF7021_Config_t *config);
HAL_StatusTypeDef ADF7021_SetRxMode(ADF7021_Config_t *config);
HAL_StatusTypeDef ADF7021_SetTxPower(ADF7021_Config_t *config, ADF7021_PALevel_e power);
HAL_StatusTypeDef ADF7021_Transmit(ADF7021_Config_t *config, uint8_t *data, uint16_t length);
HAL_StatusTypeDef ADF7021_Receive(ADF7021_Config_t *config, uint8_t *data, uint16_t length);

#endif /* ADF7021_H */
