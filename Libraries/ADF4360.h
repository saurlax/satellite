#ifndef ADF4360_H
#define ADF4360_H

#include "stm32h7xx_hal.h"
#include <stdbool.h>
#include <stdint.h>

#define ADF4360_0 0U
#define ADF4360_1 1U
#define ADF4360_2 2U
#define ADF4360_3 3U
#define ADF4360_4 4U
#define ADF4360_5 5U
#define ADF4360_6 6U
#define ADF4360_7 7U
#define ADF4360_8 8U
#define ADF4360_9 9U

#define ADF4360_REG_CONTROL   0U
#define ADF4360_REG_R_COUNTER 1U
#define ADF4360_REG_N_COUNTER 2U

#define ADF4360_PWR_NORMAL_OPERATION  0U
#define ADF4360_PWR_ASYNCH_POWER_DOWN 1U
#define ADF4360_PWR_SYNCH_POWER_DOWN  3U

#define ADF4360_MUX_THREE_STATE 0U
#define ADF4360_MUX_DIGITAL_LD  1U
#define ADF4360_MUX_N_DIVIDER   2U
#define ADF4360_MUX_DVDD        3U
#define ADF4360_MUX_R_DIVIDER   4U
#define ADF4360_MUX_N_LD        5U
#define ADF4360_MUX_SERIAL_DATA 6U
#define ADF4360_MUX_DGND        7U

#define ADF4360_OUT_POWER_3_5  0U
#define ADF4360_OUT_POWER_5_0  1U
#define ADF4360_OUT_POWER_7_5  2U
#define ADF4360_OUT_POWER_11_0 3U

#define ADF4360_CORE_POWER_5  0U
#define ADF4360_CORE_POWER_10 1U
#define ADF4360_CORE_POWER_15 2U
#define ADF4360_CORE_POWER_20 3U

typedef struct {
    uint64_t vco_min_hz;
    uint64_t vco_max_hz;
    uint32_t counters_max_hz;
    uint8_t max_prescaler;
} ADF4360_PartSpec_t;

typedef struct {
    uint32_t ref_in_hz;
    uint8_t power_down_mode;
    uint8_t current_setting_1;
    uint8_t current_setting_2;
    uint8_t output_power_level;
    bool mute_till_lock_detect;
    bool charge_pump_gain;
    bool charge_pump_three_state;
    bool phase_detector_positive;
    uint8_t muxout;
    uint8_t core_power_level;
    bool divide_by_2_select;
    bool divide_by_2;
    bool lock_detect_five_cycles;
    uint8_t antibacklash_width;
} ADF4360_Settings_t;

typedef struct {
    SPI_HandleTypeDef *hspi;
    GPIO_TypeDef *le_port;
    uint16_t le_pin;
    GPIO_TypeDef *ce_port;
    uint16_t ce_pin;
    GPIO_TypeDef *enable_port;
    uint16_t enable_pin;
    GPIO_TypeDef *lock_port;
    uint16_t lock_pin;
    uint32_t spi_timeout_ms;
    uint8_t part;
    ADF4360_Settings_t settings;
    uint32_t cached_control;
    uint32_t cached_r;
    uint32_t cached_n;
    uint64_t last_frequency_hz;
} ADF4360_Config_t;

void ADF4360_DefaultConfig(ADF4360_Config_t *config, SPI_HandleTypeDef *hspi);
HAL_StatusTypeDef ADF4360_InitConfig(ADF4360_Config_t *config);
HAL_StatusTypeDef ADF4360_WriteConfig(ADF4360_Config_t *config, uint32_t data);
HAL_StatusTypeDef ADF4360_SetFrequencyConfig(ADF4360_Config_t *config,
                                             uint64_t frequency_hz,
                                             uint64_t *actual_hz);
HAL_StatusTypeDef ADF4360_PowerConfig(ADF4360_Config_t *config, bool power_on);
GPIO_PinState ADF4360_ReadLockDetect(const ADF4360_Config_t *config);

unsigned char ADF4360_Init(unsigned char adf4360Version);
void ADF4360_Write(unsigned long data);
void ADF4360_Power(unsigned char powerMode);
unsigned long long ADF4360_SetFrequency(unsigned long long frequency);
GPIO_PinState ADF4360_DefaultLockDetect(void);

#endif /* ADF4360_H */
