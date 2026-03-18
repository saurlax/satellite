#ifndef ADF7021_H
#define ADF7021_H

#include "stm32h7xx_hal.h"
#include <stdbool.h>

#define ADF7021_SPI_TIMEOUT_DEFAULT_MS   100U
#define ADF7021_RX_IF_DEFAULT_HZ         100000UL

typedef enum {
    ADF7021_REG_0 = 0U,
    ADF7021_REG_1 = 1U,
    ADF7021_REG_2 = 2U,
    ADF7021_REG_3 = 3U,
    ADF7021_REG_4 = 4U,
    ADF7021_REG_5 = 5U,
    ADF7021_REG_6 = 6U,
    ADF7021_REG_7 = 7U,
    ADF7021_REG_8 = 8U,
    ADF7021_REG_9 = 9U,
    ADF7021_REG_10 = 10U,
    ADF7021_REG_11 = 11U,
    ADF7021_REG_12 = 12U,
    ADF7021_REG_13 = 13U,
    ADF7021_REG_14 = 14U,
    ADF7021_REG_15 = 15U
} ADF7021_Reg_e;

typedef enum {
    ADF7021_MODE_TX = 0,
    ADF7021_MODE_RX = 1
} ADF7021_Mode_e;

typedef enum {
    ADF7021_MUX_REGULATOR_READY = 0,
    ADF7021_MUX_FILTER_CAL_COMPLETE = 1,
    ADF7021_MUX_DIGITAL_LOCK_DETECT = 2,
    ADF7021_MUX_RSSI_READY = 3,
    ADF7021_MUX_TXRX = 4,
    ADF7021_MUX_LOGIC_ZERO = 5,
    ADF7021_MUX_TRISTATE = 6,
    ADF7021_MUX_LOGIC_ONE = 7
} ADF7021_Muxout_e;

typedef enum {
    ADF7021_MOD_2FSK = 0,
    ADF7021_MOD_GAUSSIAN_2FSK = 1,
    ADF7021_MOD_3FSK = 2,
    ADF7021_MOD_4FSK = 3,
    ADF7021_MOD_OVERSAMPLED_2FSK = 4,
    ADF7021_MOD_RCOS_2FSK = 5,
    ADF7021_MOD_RCOS_3FSK = 6,
    ADF7021_MOD_RCOS_4FSK = 7
} ADF7021_Modulation_e;

typedef enum {
    ADF7021_IFBW_12K5 = 0,
    ADF7021_IFBW_18K75 = 1,
    ADF7021_IFBW_25K = 2
} ADF7021_IfBw_e;

typedef enum {
    ADF7021_DEMOD_2FSK_LINEAR = 0,
    ADF7021_DEMOD_2FSK_CORR = 1,
    ADF7021_DEMOD_3FSK = 2,
    ADF7021_DEMOD_4FSK = 3
} ADF7021_Demod_e;

typedef enum {
    ADF7021_SYNC_LEN_12 = 0,
    ADF7021_SYNC_LEN_16 = 1,
    ADF7021_SYNC_LEN_20 = 2,
    ADF7021_SYNC_LEN_24 = 3
} ADF7021_SyncLen_e;

typedef enum {
    ADF7021_SYNC_ERR_0 = 0,
    ADF7021_SYNC_ERR_1 = 1,
    ADF7021_SYNC_ERR_2 = 2,
    ADF7021_SYNC_ERR_3 = 3
} ADF7021_SyncErrTol_e;

typedef struct {
    SPI_HandleTypeDef *hspi;
    GPIO_TypeDef *ce_port;
    uint16_t ce_pin;
    uint32_t spi_timeout_ms;

    uint32_t xtal_hz;
    uint8_t r_counter;
    bool xtal_doubler;
    bool xosc_enable;

    bool vco_external_inductor;
    bool rf_divide_by_2;
    uint8_t vco_adjust;
    uint8_t vco_bias;
    uint8_t cp_current;
    uint8_t xtal_bias;

    uint32_t rx_freq_hz;
    uint32_t tx_freq_hz;
    uint32_t rx_if_hz;

    uint32_t data_rate_bps;
    uint32_t freq_deviation_hz;
    uint8_t mod_index_x10;

    ADF7021_Modulation_e tx_modulation;
    ADF7021_Demod_e rx_demod;
    ADF7021_IfBw_e rx_if_bw;

    uint8_t tx_power;
    uint8_t pa_bias;
    uint8_t pa_ramp;
    bool pa_enable;

    bool afc_enable;
    uint8_t afc_ki;
    uint8_t afc_kp;
    uint32_t afc_range_hz;

    uint32_t sync_word;
    ADF7021_SyncLen_e sync_len;
    ADF7021_SyncErrTol_e sync_err_tol;

    ADF7021_Muxout_e muxout;
    bool uart_mode;

    ADF7021_Mode_e mode;

    struct {
        uint32_t reg0_rx;
        uint32_t reg0_tx;
        uint32_t reg2_tx;
        uint32_t reg3;
        uint32_t reg4;
        uint32_t reg5;
        uint32_t reg6;
        uint32_t reg9;
        uint32_t reg10;
        uint32_t reg11;
        uint32_t reg12;
        uint32_t reg15;
    } cached;
} ADF7021_Config_t;

void ADF7021_DefaultConfig(ADF7021_Config_t *config, SPI_HandleTypeDef *hspi);
HAL_StatusTypeDef ADF7021_Init(ADF7021_Config_t *config);

HAL_StatusTypeDef ADF7021_WriteReg(ADF7021_Config_t *config, ADF7021_Reg_e reg, uint32_t value_no_addr);
HAL_StatusTypeDef ADF7021_Readback(ADF7021_Config_t *config, uint8_t readback_sel, uint16_t *value);

HAL_StatusTypeDef ADF7021_SetRxMode(ADF7021_Config_t *config);
HAL_StatusTypeDef ADF7021_SetTxMode(ADF7021_Config_t *config);
HAL_StatusTypeDef ADF7021_SetTxPower(ADF7021_Config_t *config, uint8_t power);
HAL_StatusTypeDef ADF7021_RecalibrateIF(ADF7021_Config_t *config);

#endif /* ADF7021_H */
