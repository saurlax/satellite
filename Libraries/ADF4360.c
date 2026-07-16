#include "ADF4360.h"

#include "main.h"
#include "spi.h"
#include <string.h>

#define ADF4360_TIMEOUT_DEFAULT_MS 100U
#define ADF4360_MAX_PFD_HZ         8000000UL

#define ADF4360_CTRL_PRESCALE(x)         (((uint32_t)(x) & 0x3UL) << 22)
#define ADF4360_CTRL_PWR_DWN(x)          (((uint32_t)(x) & 0x3UL) << 20)
#define ADF4360_CTRL_CURRENT1(x)         (((uint32_t)(x) & 0x7UL) << 17)
#define ADF4360_CTRL_CURRENT2(x)         (((uint32_t)(x) & 0x7UL) << 14)
#define ADF4360_CTRL_OUT_PWR_LVL(x)      (((uint32_t)(x) & 0x3UL) << 12)
#define ADF4360_CTRL_MTLD                (1UL << 11)
#define ADF4360_CTRL_CP_GAIN             (1UL << 10)
#define ADF4360_CTRL_CP_THREE_STATE      (1UL << 9)
#define ADF4360_CTRL_PHASE_DETECT_POL(x) (((uint32_t)((x) ? 1U : 0U)) << 8)
#define ADF4360_CTRL_MUXOUT(x)           (((uint32_t)(x) & 0x7UL) << 5)
#define ADF4360_CTRL_COUNTER_RESET       (1UL << 4)
#define ADF4360_CTRL_CORE_POWER(x)       (((uint32_t)(x) & 0x3UL) << 2)

#define ADF4360_N_CNT_DIVIDE_2_SELECT    (1UL << 23)
#define ADF4360_N_CNT_DIVIDE_2           (1UL << 22)
#define ADF4360_N_CNT_CP_GAIN            (1UL << 21)
#define ADF4360_N_CNT_B_COUNTER(x)       (((uint32_t)(x) & 0x1FFFUL) << 8)
#define ADF4360_N_CNT_A_COUNTER(x)       (((uint32_t)(x) & 0x1FUL) << 2)

#define ADF4360_R_CNT_BAND_CLK(x)        (((uint32_t)(x) & 0x3UL) << 20)
#define ADF4360_R_CNT_LD_PRECISION       (1UL << 18)
#define ADF4360_R_CNT_ANTIBACKLASH(x)    (((uint32_t)(x) & 0x3UL) << 16)
#define ADF4360_R_CNT_REF_COUNTER(x)     (((uint32_t)(x) & 0x3FFFUL) << 2)

static const ADF4360_PartSpec_t adf4360_specs[] = {
    {2400000000ULL, 2725000000ULL, 300000000UL, 32U},
    {2050000000ULL, 2450000000ULL, 300000000UL, 32U},
    {1850000000ULL, 2150000000ULL, 300000000UL, 32U},
    {1600000000ULL, 1950000000ULL, 300000000UL, 32U},
    {1450000000ULL, 1750000000ULL, 300000000UL, 32U},
    {1200000000ULL, 1400000000ULL, 300000000UL, 32U},
    {1050000000ULL, 1250000000ULL, 300000000UL, 32U},
    {350000000ULL,  1800000000ULL, 300000000UL, 16U},
    {65000000ULL,   400000000ULL,  400000000UL, 1U},
    {65000000ULL,   400000000ULL,  400000000UL, 1U},
};

static ADF4360_Config_t g_adf4360_default;

static uint32_t adf4360_timeout_ms(const ADF4360_Config_t *config)
{
    return ((config != NULL) && (config->spi_timeout_ms != 0U)) ?
               config->spi_timeout_ms :
               ADF4360_TIMEOUT_DEFAULT_MS;
}

static uint64_t adf4360_clamp_u64(uint64_t value, uint64_t min_value, uint64_t max_value)
{
    if (value < min_value) return min_value;
    if (value > max_value) return max_value;
    return value;
}

static uint32_t adf4360_round_div_u64(uint64_t numerator, uint32_t denominator)
{
    if (denominator == 0U) {
        return 0U;
    }

    return (uint32_t)((numerator + ((uint64_t)denominator / 2ULL)) / (uint64_t)denominator);
}

static uint16_t adf4360_tune_r_counter(uint32_t ref_in_hz)
{
    uint16_t r = 1U;

    while ((r < 0x3FFFU) && ((ref_in_hz / (uint32_t)r) > ADF4360_MAX_PFD_HZ)) {
        r++;
    }

    return r;
}

static uint8_t adf4360_band_bits(uint32_t pfd_hz)
{
    uint8_t divider = 1U;

    while ((divider < 8U) && ((pfd_hz / divider) > 1000000UL)) {
        divider = (uint8_t)(divider * 2U);
    }

    switch (divider) {
    case 1U:
        return 0U;
    case 2U:
        return 1U;
    case 4U:
        return 2U;
    default:
        return 3U;
    }
}

static uint32_t adf4360_build_control(const ADF4360_Settings_t *settings)
{
    uint32_t reg = 0U;

    reg |= ADF4360_CTRL_PWR_DWN(settings->power_down_mode);
    reg |= ADF4360_CTRL_CURRENT1(settings->current_setting_1);
    reg |= ADF4360_CTRL_CURRENT2(settings->current_setting_2);
    reg |= ADF4360_CTRL_OUT_PWR_LVL(settings->output_power_level);
    reg |= settings->mute_till_lock_detect ? ADF4360_CTRL_MTLD : 0U;
    reg |= settings->charge_pump_gain ? ADF4360_CTRL_CP_GAIN : 0U;
    reg |= settings->charge_pump_three_state ? ADF4360_CTRL_CP_THREE_STATE : 0U;
    reg |= ADF4360_CTRL_PHASE_DETECT_POL(settings->phase_detector_positive);
    reg |= ADF4360_CTRL_MUXOUT(settings->muxout);
    reg |= ADF4360_CTRL_CORE_POWER(settings->core_power_level);

    return reg;
}

static uint32_t adf4360_build_r(const ADF4360_Settings_t *settings)
{
    uint32_t reg = 0U;

    reg |= settings->lock_detect_five_cycles ? ADF4360_R_CNT_LD_PRECISION : 0U;
    reg |= ADF4360_R_CNT_ANTIBACKLASH(settings->antibacklash_width);

    return reg;
}

static uint32_t adf4360_build_n(const ADF4360_Settings_t *settings)
{
    uint32_t reg = 0U;

    reg |= settings->divide_by_2_select ? ADF4360_N_CNT_DIVIDE_2_SELECT : 0U;
    reg |= settings->divide_by_2 ? ADF4360_N_CNT_DIVIDE_2 : 0U;
    reg |= settings->charge_pump_gain ? ADF4360_N_CNT_CP_GAIN : 0U;

    return reg;
}

void ADF4360_DefaultConfig(ADF4360_Config_t *config, SPI_HandleTypeDef *hspi)
{
    if (config == NULL) {
        return;
    }

    memset(config, 0, sizeof(*config));
    config->hspi = hspi;
    config->le_port = GPIOA;
    config->le_pin = GPIO_PIN_4;
    config->ce_port = TXPLL__CE_GPIO_Port;
    config->ce_pin = TXPLL__CE_Pin;
    config->enable_port = TXPLL__ENE13_GPIO_Port;
    config->enable_pin = TXPLL__ENE13_Pin;
    config->lock_port = TXPLL_LD_GPIO_Port;
    config->lock_pin = TXPLL_LD_Pin;
    config->spi_timeout_ms = ADF4360_TIMEOUT_DEFAULT_MS;
    config->part = ADF4360_7;

    config->settings.ref_in_hz = 25000000UL;
    config->settings.power_down_mode = ADF4360_PWR_NORMAL_OPERATION;
    config->settings.current_setting_1 = 7U;
    config->settings.current_setting_2 = 7U;
    config->settings.output_power_level = ADF4360_OUT_POWER_11_0;
    config->settings.mute_till_lock_detect = false;
    config->settings.charge_pump_gain = false;
    config->settings.charge_pump_three_state = false;
    config->settings.phase_detector_positive = true;
    config->settings.muxout = ADF4360_MUX_DIGITAL_LD;
    config->settings.core_power_level = ADF4360_CORE_POWER_5;
    config->settings.divide_by_2_select = false;
    config->settings.divide_by_2 = false;
    config->settings.lock_detect_five_cycles = false;
    config->settings.antibacklash_width = 0U;
}

HAL_StatusTypeDef ADF4360_WriteConfig(ADF4360_Config_t *config, uint32_t data)
{
    uint8_t tx[3];
    HAL_StatusTypeDef st;

    if ((config == NULL) || (config->hspi == NULL) || (config->le_port == NULL)) {
        return HAL_ERROR;
    }

    tx[0] = (uint8_t)((data >> 16) & 0xFFU);
    tx[1] = (uint8_t)((data >> 8) & 0xFFU);
    tx[2] = (uint8_t)(data & 0xFFU);

    HAL_GPIO_WritePin(config->le_port, config->le_pin, GPIO_PIN_RESET);
    st = HAL_SPI_Transmit(config->hspi, tx, sizeof(tx), adf4360_timeout_ms(config));
    HAL_GPIO_WritePin(config->le_port, config->le_pin, GPIO_PIN_SET);

    return st;
}

HAL_StatusTypeDef ADF4360_InitConfig(ADF4360_Config_t *config)
{
    HAL_StatusTypeDef st;

    if ((config == NULL) || (config->part >= (uint8_t)(sizeof(adf4360_specs) / sizeof(adf4360_specs[0])))) {
        return HAL_ERROR;
    }

    if (config->ce_port != NULL) {
        HAL_GPIO_WritePin(config->ce_port, config->ce_pin, GPIO_PIN_SET);
    }
    if (config->enable_port != NULL) {
        HAL_GPIO_WritePin(config->enable_port, config->enable_pin, GPIO_PIN_SET);
    }
    if (config->le_port != NULL) {
        HAL_GPIO_WritePin(config->le_port, config->le_pin, GPIO_PIN_SET);
    }

    config->cached_r = adf4360_build_r(&config->settings);
    config->cached_control = adf4360_build_control(&config->settings);
    config->cached_n = adf4360_build_n(&config->settings);

    st = ADF4360_WriteConfig(config, ADF4360_REG_R_COUNTER | config->cached_r);
    if (st != HAL_OK) return st;
    st = ADF4360_WriteConfig(config, ADF4360_REG_CONTROL | config->cached_control);
    if (st != HAL_OK) return st;
    HAL_Delay(10U);
    return ADF4360_WriteConfig(config, ADF4360_REG_N_COUNTER | config->cached_n);
}

HAL_StatusTypeDef ADF4360_SetFrequencyConfig(ADF4360_Config_t *config,
                                             uint64_t frequency_hz,
                                             uint64_t *actual_hz)
{
    const ADF4360_PartSpec_t *spec;
    uint64_t vco_hz;
    uint8_t prescaler = 8U;
    uint16_t r_counter;
    uint32_t pfd_hz;
    uint32_t ratio;
    uint16_t a = 0U;
    uint16_t b = 0U;
    uint8_t band;
    HAL_StatusTypeDef st;

    if ((config == NULL) || (config->part >= (uint8_t)(sizeof(adf4360_specs) / sizeof(adf4360_specs[0]))) ||
        (config->settings.ref_in_hz == 0U)) {
        return HAL_ERROR;
    }

    spec = &adf4360_specs[config->part];
    vco_hz = adf4360_clamp_u64(frequency_hz, spec->vco_min_hz, spec->vco_max_hz);

    if (config->part > ADF4360_7) {
        prescaler = 1U;
    } else {
        while ((prescaler < spec->max_prescaler) && ((vco_hz / prescaler) > spec->counters_max_hz)) {
            prescaler = (uint8_t)(prescaler * 2U);
        }
    }

    r_counter = adf4360_tune_r_counter(config->settings.ref_in_hz);
    pfd_hz = config->settings.ref_in_hz / (uint32_t)r_counter;
    ratio = adf4360_round_div_u64(vco_hz, pfd_hz);

    if (prescaler == 1U) {
        b = (uint16_t)ratio;
    } else {
        b = (uint16_t)(ratio / prescaler);
        a = (uint16_t)(ratio % prescaler);
        while ((a > b) && (r_counter < 0x3FFFU)) {
            r_counter++;
            pfd_hz = config->settings.ref_in_hz / (uint32_t)r_counter;
            ratio = adf4360_round_div_u64(vco_hz, pfd_hz);
            b = (uint16_t)(ratio / prescaler);
            a = (uint16_t)(ratio % prescaler);
        }
    }

    band = adf4360_band_bits(pfd_hz);

    st = ADF4360_WriteConfig(config,
                             ADF4360_REG_R_COUNTER |
                                 config->cached_r |
                                 ADF4360_R_CNT_BAND_CLK(band) |
                                 ADF4360_R_CNT_REF_COUNTER(r_counter));
    if (st != HAL_OK) return st;

    st = ADF4360_WriteConfig(config,
                             ADF4360_REG_CONTROL |
                                 config->cached_control |
                                 ADF4360_CTRL_PRESCALE(prescaler / 16U));
    if (st != HAL_OK) return st;

    HAL_Delay(10U);

    st = ADF4360_WriteConfig(config,
                             ADF4360_REG_N_COUNTER |
                                 config->cached_n |
                                 ADF4360_N_CNT_B_COUNTER(b) |
                                 ADF4360_N_CNT_A_COUNTER(a));
    if (st != HAL_OK) return st;

    config->last_frequency_hz = (uint64_t)(((uint32_t)b * (uint32_t)prescaler) + (uint32_t)a) * (uint64_t)pfd_hz;
    if (actual_hz != NULL) {
        *actual_hz = config->last_frequency_hz;
    }

    return HAL_OK;
}

HAL_StatusTypeDef ADF4360_PowerConfig(ADF4360_Config_t *config, bool power_on)
{
    uint32_t reg;

    if (config == NULL) {
        return HAL_ERROR;
    }

    reg = (config->cached_control & ~ADF4360_CTRL_PWR_DWN(3U)) |
          ADF4360_CTRL_PWR_DWN(power_on ? ADF4360_PWR_NORMAL_OPERATION : ADF4360_PWR_SYNCH_POWER_DOWN);
    return ADF4360_WriteConfig(config, ADF4360_REG_CONTROL | reg);
}

GPIO_PinState ADF4360_ReadLockDetect(const ADF4360_Config_t *config)
{
    if ((config == NULL) || (config->lock_port == NULL)) {
        return GPIO_PIN_RESET;
    }

    return HAL_GPIO_ReadPin(config->lock_port, config->lock_pin);
}

unsigned char ADF4360_Init(unsigned char adf4360Version)
{
    ADF4360_DefaultConfig(&g_adf4360_default, &hspi3);
    g_adf4360_default.part = adf4360Version;

    return (ADF4360_InitConfig(&g_adf4360_default) == HAL_OK) ? 1U : 0U;
}

void ADF4360_Write(unsigned long data)
{
    (void)ADF4360_WriteConfig(&g_adf4360_default, (uint32_t)data);
}

void ADF4360_Power(unsigned char powerMode)
{
    (void)ADF4360_PowerConfig(&g_adf4360_default, powerMode != 0U);
}

unsigned long long ADF4360_SetFrequency(unsigned long long frequency)
{
    uint64_t actual = 0ULL;

    if (ADF4360_SetFrequencyConfig(&g_adf4360_default, frequency, &actual) != HAL_OK) {
        return 0ULL;
    }

    return (unsigned long long)actual;
}

GPIO_PinState ADF4360_DefaultLockDetect(void)
{
    return ADF4360_ReadLockDetect(&g_adf4360_default);
}
