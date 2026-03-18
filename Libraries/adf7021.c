#include "adf7021.h"

#include <math.h>
#include <string.h>

#define ADF7021_ADDR_MASK             0x0FU
#define ADF7021_REG_DATA_MASK         0xFFFFFFF0UL

static uint32_t adf7021_timeout_ms(const ADF7021_Config_t *config)
{
    if ((config == NULL) || (config->spi_timeout_ms == 0U)) {
        return ADF7021_SPI_TIMEOUT_DEFAULT_MS;
    }

    return config->spi_timeout_ms;
}

static uint32_t adf7021_pfd_hz(const ADF7021_Config_t *config)
{
    uint32_t r_div;
    uint32_t pfd;

    r_div = (config->r_counter == 0U) ? 1U : config->r_counter;
    pfd = config->xtal_hz / r_div;

    if (config->xtal_doubler) {
        pfd *= 2U;
    }

    return pfd;
}

static uint32_t adf7021_round_u32(double x)
{
    if (x <= 0.0) {
        return 0U;
    }

    return (uint32_t)(x + 0.5);
}

static uint32_t adf7021_clamp_u32(uint32_t val, uint32_t min_v, uint32_t max_v)
{
    if (val < min_v) {
        return min_v;
    }

    if (val > max_v) {
        return max_v;
    }

    return val;
}

static uint8_t adf7021_symbol_den(const ADF7021_Config_t *config)
{
    switch (config->tx_modulation) {
    case ADF7021_MOD_4FSK:
    case ADF7021_MOD_RCOS_4FSK:
        return 2U;
    default:
        return 1U;
    }
}

static uint32_t adf7021_effective_fdev_hz(const ADF7021_Config_t *config)
{
    if (config->freq_deviation_hz != 0U) {
        return config->freq_deviation_hz;
    }

    if ((config->mod_index_x10 == 0U) || (config->data_rate_bps == 0U)) {
        return 0U;
    }

    return (uint32_t)(((uint64_t)config->mod_index_x10 * (uint64_t)config->data_rate_bps) / 20ULL);
}

static HAL_StatusTypeDef adf7021_write_word(ADF7021_Config_t *config, uint32_t word)
{
    uint8_t tx[4];

    if ((config == NULL) || (config->hspi == NULL)) {
        return HAL_ERROR;
    }

    tx[0] = (uint8_t)((word >> 24) & 0xFFU);
    tx[1] = (uint8_t)((word >> 16) & 0xFFU);
    tx[2] = (uint8_t)((word >> 8) & 0xFFU);
    tx[3] = (uint8_t)(word & 0xFFU);

    return HAL_SPI_Transmit(config->hspi, tx, 4U, adf7021_timeout_ms(config));
}

static HAL_StatusTypeDef adf7021_program_common_sys_regs(ADF7021_Config_t *config)
{
    uint32_t r1;

    r1 = 0U;
    r1 |= ((uint32_t)(config->vco_external_inductor ? 1U : 0U) & 0x1U) << 25;
    r1 |= ((uint32_t)config->vco_adjust & 0x3U) << 23;
    r1 |= ((uint32_t)config->vco_bias & 0xFU) << 19;
    r1 |= ((uint32_t)(config->rf_divide_by_2 ? 1U : 0U) & 0x1U) << 18;
    r1 |= (1U << 17); /* VCO enable */
    r1 |= ((uint32_t)config->cp_current & 0x3U) << 15;
    r1 |= ((uint32_t)config->xtal_bias & 0x3U) << 13;
    r1 |= ((uint32_t)(config->xosc_enable ? 1U : 0U) & 0x1U) << 12;
    r1 |= ((uint32_t)(config->xtal_doubler ? 1U : 0U) & 0x1U) << 11;
    r1 |= (7U << 7); /* CLKOUT divide */
    r1 |= ((uint32_t)adf7021_clamp_u32(config->r_counter, 1U, 7U) & 0x7U) << 4;

    if (ADF7021_WriteReg(config, ADF7021_REG_1, r1) != HAL_OK) {
        return HAL_ERROR;
    }

    config->cached.reg15 = (7U << 17); /* CLK_MUX = TxRxCLK for SPI visibility */
    return ADF7021_WriteReg(config, ADF7021_REG_15, config->cached.reg15);
}

static uint32_t adf7021_build_r0(const ADF7021_Config_t *config, uint32_t rf_hz, ADF7021_Mode_e mode)
{
    uint32_t pfd;
    double divider;
    uint32_t n_int;
    uint32_t n_frac;
    uint32_t reg;

    pfd = adf7021_pfd_hz(config);
    if (pfd == 0U) {
        return 0U;
    }

    if (config->rf_divide_by_2) {
        divider = (2.0 * (double)rf_hz) / (double)pfd;
    } else {
        divider = (double)rf_hz / (double)pfd;
    }

    n_int = adf7021_clamp_u32((uint32_t)floor(divider), 23U, 255U);
    n_frac = adf7021_clamp_u32(adf7021_round_u32((divider - floor(divider)) * 32768.0), 0U, 32767U);

    reg = 0U;
    reg |= ((uint32_t)config->muxout & 0x7U) << 29;
    reg |= ((uint32_t)(config->uart_mode ? 1U : 0U) & 0x1U) << 28;
    reg |= ((uint32_t)mode & 0x1U) << 27;
    reg |= (n_int & 0xFFU) << 19;
    reg |= (n_frac & 0x7FFFU) << 4;

    return reg;
}

static uint32_t adf7021_build_r2_tx(const ADF7021_Config_t *config)
{
    uint32_t pfd;
    uint32_t fdev_hz;
    uint32_t tx_dev;
    uint32_t reg;

    pfd = adf7021_pfd_hz(config);
    fdev_hz = adf7021_effective_fdev_hz(config);

    if ((pfd == 0U) || (fdev_hz == 0U)) {
        tx_dev = 0U;
    } else if (config->rf_divide_by_2) {
        tx_dev = adf7021_round_u32(((double)fdev_hz * 131072.0) / (double)pfd);
    } else {
        tx_dev = adf7021_round_u32(((double)fdev_hz * 65536.0) / (double)pfd);
    }

    tx_dev = adf7021_clamp_u32(tx_dev, 0U, 0x1FFU);

    reg = 0U;
    reg |= (tx_dev & 0x1FFU) << 19;
    reg |= ((uint32_t)config->tx_power & 0x3FU) << 13;
    reg |= ((uint32_t)config->pa_bias & 0x3U) << 11;
    reg |= ((uint32_t)config->pa_ramp & 0x7U) << 8;
    reg |= ((uint32_t)(config->pa_enable ? 1U : 0U) & 0x1U) << 7;
    reg |= ((uint32_t)config->tx_modulation & 0x7U) << 4;

    return reg;
}

static void adf7021_build_rx_clocks(ADF7021_Config_t *config, uint32_t *dem_div_out, uint32_t *cdr_div_out)
{
    uint32_t dem_div_best;
    uint32_t cdr_div_best;
    uint32_t residual_best;
    uint32_t pfd;
    uint32_t fdev_hz;
    uint32_t i;

    dem_div_best = 1U;
    cdr_div_best = 1U;
    residual_best = 0xFFFFFFFFUL;

    pfd = adf7021_pfd_hz(config);
    fdev_hz = adf7021_effective_fdev_hz(config);

    for (i = 1U; i <= 15U; i++) {
        double dem_clk;
        double symbol_rate;
        uint32_t cdr_div;
        uint32_t disc_bw;
        double data_real;
        uint32_t target;
        uint32_t residual;
        double k;

        dem_clk = (double)config->xtal_hz / (double)i;
        symbol_rate = (double)config->data_rate_bps / (double)adf7021_symbol_den(config);
        cdr_div = adf7021_round_u32(dem_clk / (symbol_rate * 32.0));

        if ((cdr_div == 0U) || (cdr_div > 255U)) {
            continue;
        }

        if (fdev_hz == 0U) {
            disc_bw = 1U;
        } else {
            if ((config->rx_demod == ADF7021_DEMOD_4FSK) && (fdev_hz != 0U)) {
                k = 100000.0 / (4.0 * (double)fdev_hz);
            } else {
                k = 100000.0 / (double)fdev_hz;
            }

            disc_bw = adf7021_round_u32((k * dem_clk) / 400000.0);
        }

        if ((disc_bw == 0U) || (disc_bw > 660U)) {
            continue;
        }

        data_real = dem_clk / ((double)cdr_div * 32.0);
        if (adf7021_symbol_den(config) == 2U) {
            data_real *= 2.0;
        }

        target = config->data_rate_bps;
        residual = (target > (uint32_t)data_real) ? (target - (uint32_t)data_real) : ((uint32_t)data_real - target);

        if (residual < residual_best) {
            residual_best = residual;
            dem_div_best = i;
            cdr_div_best = cdr_div;
        }
    }

    if (dem_div_out != NULL) {
        *dem_div_out = dem_div_best;
    }

    if (cdr_div_out != NULL) {
        *cdr_div_out = cdr_div_best;
    }

    (void)pfd;
}

static uint32_t adf7021_build_r3(const ADF7021_Config_t *config, uint32_t dem_div, uint32_t cdr_div)
{
    uint32_t seq_div;
    uint32_t seq_clk;
    uint32_t agc_div;
    uint32_t bbos_div;
    uint32_t reg;

    seq_div = adf7021_round_u32((double)config->xtal_hz / 100000.0);
    seq_div = adf7021_clamp_u32(seq_div, 1U, 255U);

    seq_clk = config->xtal_hz / seq_div;
    agc_div = adf7021_round_u32((double)seq_clk / 10000.0);
    agc_div = adf7021_clamp_u32(agc_div, 1U, 127U);

    bbos_div = adf7021_round_u32((double)config->xtal_hz / 1500000.0);
    if (bbos_div <= 4U) {
        bbos_div = 4U;
    } else if (bbos_div <= 8U) {
        bbos_div = 8U;
    } else if (bbos_div <= 16U) {
        bbos_div = 16U;
    } else {
        bbos_div = 32U;
    }

    reg = 0U;
    reg |= (agc_div & 0x3FU) << 26;
    reg |= (seq_div & 0xFFU) << 18;
    reg |= (cdr_div & 0xFFU) << 10;
    reg |= (dem_div & 0x0FU) << 6;

    switch (bbos_div) {
    case 4U:
        reg |= 0U << 4;
        break;
    case 8U:
        reg |= 1U << 4;
        break;
    case 16U:
        reg |= 2U << 4;
        break;
    default:
        reg |= 3U << 4;
        break;
    }

    return reg;
}

static uint32_t adf7021_build_r4(const ADF7021_Config_t *config, uint32_t dem_div)
{
    uint32_t fdev_hz;
    double dem_clk;
    double k;
    uint32_t disc_bw;
    uint32_t post_bw;
    uint32_t rx_invert;
    uint32_t dot_product;
    uint32_t reg;

    fdev_hz = adf7021_effective_fdev_hz(config);
    dem_clk = (double)config->xtal_hz / (double)dem_div;

    if (fdev_hz == 0U) {
        disc_bw = 1U;
        dot_product = 1U;
        rx_invert = 0U;
    } else {
        uint32_t k_int;

        if (config->rx_demod == ADF7021_DEMOD_4FSK) {
            k = 100000.0 / (4.0 * (double)fdev_hz);
            k_int = adf7021_round_u32(k);
            disc_bw = adf7021_round_u32((k * dem_clk) / 400000.0);
            if ((k_int & 1U) != 0U) {
                dot_product = 1U;
                rx_invert = (((k_int + 1U) / 2U) & 1U) ? 2U : 0U;
            } else {
                dot_product = 0U;
                rx_invert = ((k_int / 2U) & 1U) ? 2U : 0U;
            }
        } else {
            k = 100000.0 / (double)fdev_hz;
            k_int = adf7021_round_u32(k);
            disc_bw = adf7021_round_u32((k * dem_clk) / 400000.0);
            if ((k_int & 1U) != 0U) {
                dot_product = 1U;
                rx_invert = (((k_int + 1U) / 2U) & 1U) ? 2U : 0U;
            } else {
                dot_product = 0U;
                rx_invert = ((k_int / 2U) & 1U) ? 2U : 0U;
            }
        }
    }

    if (config->rx_demod == ADF7021_DEMOD_4FSK) {
        double symbol_rate;
        symbol_rate = (double)config->data_rate_bps / 2.0;
        post_bw = adf7021_round_u32((1.6 * symbol_rate * 3.141592654 * 2048.0) / dem_clk);
    } else if (config->rx_demod == ADF7021_DEMOD_3FSK) {
        post_bw = adf7021_round_u32(((double)config->data_rate_bps * 3.141592654 * 2048.0) / dem_clk);
    } else {
        post_bw = adf7021_round_u32((0.75 * (double)config->data_rate_bps * 3.141592654 * 2048.0) / dem_clk);
    }

    disc_bw = adf7021_clamp_u32(disc_bw, 1U, 660U);
    post_bw = adf7021_clamp_u32(post_bw, 1U, 1023U);

    reg = 0U;
    reg |= ((uint32_t)config->rx_if_bw & 0x3U) << 30;
    reg |= (post_bw & 0x3FFU) << 20;
    reg |= (disc_bw & 0x3FFU) << 10;
    reg |= (rx_invert & 0x3U) << 8;
    reg |= (dot_product & 0x1U) << 7;
    reg |= ((uint32_t)config->rx_demod & 0x7U) << 4;

    return reg;
}

static uint32_t adf7021_build_r5(const ADF7021_Config_t *config, bool do_coarse_cal)
{
    uint32_t div;
    uint32_t reg;

    div = adf7021_round_u32((double)config->xtal_hz / 50000.0);
    div = adf7021_clamp_u32(div, 1U, 511U);

    reg = 0U;
    reg |= (div & 0x1FFU) << 5;
    reg |= ((uint32_t)(do_coarse_cal ? 1U : 0U) & 0x1U) << 4;

    return reg;
}

static uint32_t adf7021_build_r6(const ADF7021_Config_t *config)
{
    uint32_t upper;
    uint32_t lower;
    uint32_t dwell;
    uint32_t reg;

    upper = adf7021_round_u32((double)config->xtal_hz / (131500.0 * 2.0));
    lower = adf7021_round_u32((double)config->xtal_hz / (65800.0 * 2.0));

    upper = adf7021_clamp_u32(upper, 1U, 255U);
    lower = adf7021_clamp_u32(lower, 1U, 255U);

    dwell = 10U;

    reg = 0U;
    reg |= (dwell & 0x7FU) << 21;
    reg |= (upper & 0xFFU) << 13;
    reg |= (lower & 0xFFU) << 5;
    reg |= 1U << 4; /* Enable fine cal path */

    return reg;
}

static uint32_t adf7021_build_r9(void)
{
    uint32_t reg;

    reg = 0U;
    reg |= (0U << 28); /* mixer linearity default */
    reg |= (0U << 26); /* LNA current default */
    reg |= (0U << 25); /* LNA mode default */
    reg |= (0U << 24); /* filter current low */
    reg |= (1U << 22); /* filter gain = 24 */
    reg |= (1U << 20); /* LNA gain = 10 */
    reg |= (0U << 18); /* AGC auto */
    reg |= (70U & 0x7FU) << 11;
    reg |= (30U & 0x7FU) << 4;

    return reg;
}

static uint32_t adf7021_build_r10(const ADF7021_Config_t *config)
{
    uint32_t max_range;
    uint32_t pfd;
    uint32_t scale;
    uint32_t reg;

    pfd = adf7021_pfd_hz(config);
    if (pfd == 0U) {
        return 0U;
    }

    scale = adf7021_round_u32((16777216.0 * 500.0) / (double)config->xtal_hz);
    scale = adf7021_clamp_u32(scale, 1U, 4095U);

    max_range = adf7021_round_u32((double)config->afc_range_hz / 500.0);
    if (config->rf_divide_by_2) {
        max_range *= 2U;
    }
    max_range = adf7021_clamp_u32(max_range, 1U, 255U);

    reg = 0U;
    reg |= (max_range & 0xFFU) << 24;
    reg |= ((uint32_t)config->afc_kp & 0x7U) << 21;
    reg |= ((uint32_t)config->afc_ki & 0xFU) << 17;
    reg |= (scale & 0xFFFU) << 5;
    reg |= ((uint32_t)(config->afc_enable ? 1U : 0U) & 0x1U) << 4;

    (void)pfd;
    return reg;
}

static uint32_t adf7021_build_r11(const ADF7021_Config_t *config)
{
    uint32_t reg;

    reg = 0U;
    reg |= (config->sync_word & 0xFFFFFFUL) << 8;
    reg |= ((uint32_t)config->sync_err_tol & 0x3U) << 6;
    reg |= ((uint32_t)config->sync_len & 0x3U) << 4;

    return reg;
}

static uint32_t adf7021_build_r12(void)
{
    uint32_t reg;

    reg = 0U;
    reg |= (255U << 8);  /* packet length */
    reg |= (1U << 6);    /* SWD mode */
    reg |= (1U << 4);    /* lock threshold mode */

    return reg;
}

HAL_StatusTypeDef ADF7021_WriteReg(ADF7021_Config_t *config, ADF7021_Reg_e reg, uint32_t value_no_addr)
{
    uint32_t word;

    if ((config == NULL) || (config->hspi == NULL)) {
        return HAL_ERROR;
    }

    if ((uint8_t)reg > (uint8_t)ADF7021_REG_15) {
        return HAL_ERROR;
    }

    word = (value_no_addr & ADF7021_REG_DATA_MASK) | ((uint32_t)reg & ADF7021_ADDR_MASK);
    return adf7021_write_word(config, word);
}

HAL_StatusTypeDef ADF7021_Readback(ADF7021_Config_t *config, uint8_t readback_sel, uint16_t *value)
{
    HAL_StatusTypeDef st;
    uint32_t r7;
    uint8_t tx[4] = {0xFFU, 0xFFU, 0xFFU, 0xFFU};
    uint8_t rx[4] = {0U, 0U, 0U, 0U};

    if ((config == NULL) || (config->hspi == NULL) || (value == NULL)) {
        return HAL_ERROR;
    }

    r7 = 0U;
    r7 |= 1U << 8; /* readback enabled */
    r7 |= ((uint32_t)readback_sel & 0x3U) << 6;

    st = ADF7021_WriteReg(config, ADF7021_REG_7, r7);
    if (st != HAL_OK) {
        return st;
    }

    st = HAL_SPI_TransmitReceive(config->hspi, tx, rx, 4U, adf7021_timeout_ms(config));
    if (st != HAL_OK) {
        return st;
    }

    *value = (uint16_t)(((uint16_t)rx[2] << 8) | rx[3]);
    return HAL_OK;
}

void ADF7021_DefaultConfig(ADF7021_Config_t *config, SPI_HandleTypeDef *hspi)
{
    if (config == NULL) {
        return;
    }

    memset(config, 0, sizeof(*config));

    config->hspi = hspi;
    config->spi_timeout_ms = ADF7021_SPI_TIMEOUT_DEFAULT_MS;

    config->xtal_hz = 12288000UL;
    config->r_counter = 1U;
    config->xtal_doubler = false;
    config->xosc_enable = true;

    config->vco_external_inductor = true;
    config->rf_divide_by_2 = true;
    config->vco_adjust = 1U;
    config->vco_bias = 0xFU;
    config->cp_current = 3U;
    config->xtal_bias = 3U;

    config->rx_freq_hz = 435000000UL;
    config->tx_freq_hz = 435000000UL;
    config->rx_if_hz = ADF7021_RX_IF_DEFAULT_HZ;

    config->data_rate_bps = 9600UL;
    config->freq_deviation_hz = 4800UL;
    config->mod_index_x10 = 10U;

    config->tx_modulation = ADF7021_MOD_GAUSSIAN_2FSK;
    config->rx_demod = ADF7021_DEMOD_2FSK_CORR;
    config->rx_if_bw = ADF7021_IFBW_12K5;

    config->tx_power = 20U;
    config->pa_bias = 3U;
    config->pa_ramp = 7U;
    config->pa_enable = true;

    config->afc_enable = true;
    config->afc_ki = 11U;
    config->afc_kp = 4U;
    config->afc_range_hz = 5000UL;

    config->sync_word = 0x9A55E7UL;
    config->sync_len = ADF7021_SYNC_LEN_24;
    config->sync_err_tol = ADF7021_SYNC_ERR_1;

    config->muxout = ADF7021_MUX_DIGITAL_LOCK_DETECT;
    config->uart_mode = true;
    config->mode = ADF7021_MODE_RX;
}

HAL_StatusTypeDef ADF7021_RecalibrateIF(ADF7021_Config_t *config)
{
    HAL_StatusTypeDef st;

    if (config == NULL) {
        return HAL_ERROR;
    }

    st = ADF7021_WriteReg(config, ADF7021_REG_6, config->cached.reg6);
    if (st != HAL_OK) {
        return st;
    }

    st = ADF7021_WriteReg(config, ADF7021_REG_5, adf7021_build_r5(config, true));
    if (st != HAL_OK) {
        return st;
    }

    HAL_Delay(10U);
    config->cached.reg5 = adf7021_build_r5(config, false);

    return ADF7021_WriteReg(config, ADF7021_REG_5, config->cached.reg5);
}

HAL_StatusTypeDef ADF7021_SetRxMode(ADF7021_Config_t *config)
{
    HAL_StatusTypeDef st;

    if (config == NULL) {
        return HAL_ERROR;
    }

    st = ADF7021_WriteReg(config, ADF7021_REG_3, config->cached.reg3);
    if (st != HAL_OK) return st;
    st = ADF7021_WriteReg(config, ADF7021_REG_5, config->cached.reg5);
    if (st != HAL_OK) return st;
    st = ADF7021_WriteReg(config, ADF7021_REG_0, config->cached.reg0_rx);
    if (st != HAL_OK) return st;
    st = ADF7021_WriteReg(config, ADF7021_REG_4, config->cached.reg4);
    if (st != HAL_OK) return st;

    config->mode = ADF7021_MODE_RX;
    return HAL_OK;
}

HAL_StatusTypeDef ADF7021_SetTxMode(ADF7021_Config_t *config)
{
    HAL_StatusTypeDef st;

    if (config == NULL) {
        return HAL_ERROR;
    }

    st = ADF7021_WriteReg(config, ADF7021_REG_2, config->cached.reg2_tx);
    if (st != HAL_OK) return st;
    st = ADF7021_WriteReg(config, ADF7021_REG_3, config->cached.reg3);
    if (st != HAL_OK) return st;
    st = ADF7021_WriteReg(config, ADF7021_REG_0, config->cached.reg0_tx);
    if (st != HAL_OK) return st;

    config->mode = ADF7021_MODE_TX;
    return HAL_OK;
}

HAL_StatusTypeDef ADF7021_SetTxPower(ADF7021_Config_t *config, uint8_t power)
{
    if (config == NULL) {
        return HAL_ERROR;
    }

    config->tx_power = (uint8_t)adf7021_clamp_u32(power, 0U, 63U);
    config->cached.reg2_tx = adf7021_build_r2_tx(config);

    if (config->mode == ADF7021_MODE_TX) {
        return ADF7021_WriteReg(config, ADF7021_REG_2, config->cached.reg2_tx);
    }

    return HAL_OK;
}

HAL_StatusTypeDef ADF7021_Init(ADF7021_Config_t *config)
{
    HAL_StatusTypeDef st;
    uint32_t dem_div;
    uint32_t cdr_div;
    uint32_t rx_lo_hz;

    if ((config == NULL) || (config->hspi == NULL) || (config->ce_port == NULL)) {
        return HAL_ERROR;
    }

    if ((config->xtal_hz == 0U) || (config->data_rate_bps == 0U)) {
        return HAL_ERROR;
    }

    HAL_GPIO_WritePin(config->ce_port, config->ce_pin, GPIO_PIN_SET);
    HAL_Delay(1U);

    st = adf7021_program_common_sys_regs(config);
    if (st != HAL_OK) {
        return st;
    }

    rx_lo_hz = (config->rx_freq_hz > config->rx_if_hz) ? (config->rx_freq_hz - config->rx_if_hz) : config->rx_freq_hz;

    config->cached.reg0_rx = adf7021_build_r0(config, rx_lo_hz, ADF7021_MODE_RX);
    config->cached.reg0_tx = adf7021_build_r0(config, config->tx_freq_hz, ADF7021_MODE_TX);
    config->cached.reg2_tx = adf7021_build_r2_tx(config);

    adf7021_build_rx_clocks(config, &dem_div, &cdr_div);
    config->cached.reg3 = adf7021_build_r3(config, dem_div, cdr_div);
    config->cached.reg4 = adf7021_build_r4(config, dem_div);
    config->cached.reg5 = adf7021_build_r5(config, false);
    config->cached.reg6 = adf7021_build_r6(config);
    config->cached.reg9 = adf7021_build_r9();
    config->cached.reg10 = adf7021_build_r10(config);
    config->cached.reg11 = adf7021_build_r11(config);
    config->cached.reg12 = adf7021_build_r12();

    st = ADF7021_WriteReg(config, ADF7021_REG_3, config->cached.reg3);
    if (st != HAL_OK) return st;
    st = ADF7021_WriteReg(config, ADF7021_REG_4, config->cached.reg4);
    if (st != HAL_OK) return st;
    st = ADF7021_WriteReg(config, ADF7021_REG_5, config->cached.reg5);
    if (st != HAL_OK) return st;
    st = ADF7021_WriteReg(config, ADF7021_REG_9, config->cached.reg9);
    if (st != HAL_OK) return st;
    st = ADF7021_WriteReg(config, ADF7021_REG_10, config->cached.reg10);
    if (st != HAL_OK) return st;
    st = ADF7021_WriteReg(config, ADF7021_REG_11, config->cached.reg11);
    if (st != HAL_OK) return st;
    st = ADF7021_WriteReg(config, ADF7021_REG_12, config->cached.reg12);
    if (st != HAL_OK) return st;

    st = ADF7021_RecalibrateIF(config);
    if (st != HAL_OK) {
        return st;
    }

    return ADF7021_SetRxMode(config);
}