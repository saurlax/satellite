#include "aic3104.h"
#include <string.h>

#define AIC3104_TIMEOUT_DEFAULT_MS 100U
#define AIC3104_PAGE_0 0x00U

static HAL_StatusTypeDef aic3104_select_page(AIC3104_Config_t *config, uint8_t page)
{
    return AIC3104_WriteReg(config, AIC3104_REG_PAGE_SELECT, page);
}

static HAL_StatusTypeDef aic3104_write_checked(AIC3104_Config_t *config, uint8_t reg, uint8_t val)
{
    HAL_StatusTypeDef st = AIC3104_WriteReg(config, reg, val);
    return st;
}

static HAL_StatusTypeDef aic3104_sync_sample_rate_reg(AIC3104_Config_t *config)
{
    uint8_t sr_code;

    if (config == NULL) {
        return HAL_ERROR;
    }

    switch (config->sample_rate) {
    case AIC3104_FS_8K:
        sr_code = 0x0AU;
        break;
    case AIC3104_FS_16K:
        sr_code = 0x04U;
        break;
    case AIC3104_FS_44K:
    case AIC3104_FS_48K:
        sr_code = 0x00U;
        break;
    default:
        return HAL_ERROR;
    }

    config->clock.sample_rate_reg = (uint8_t)((sr_code << 4) | sr_code);
    return HAL_OK;
}

static HAL_StatusTypeDef aic3104_output_ctrl_reg(AIC3104_OutputType_e output, bool right_channel, uint8_t *reg)
{
    if (reg == NULL) {
        return HAL_ERROR;
    }

    switch (output) {
    case AIC3104_OUTPUT_HP:
        *reg = right_channel ? AIC3104_REG_HPROUT_CTRL : AIC3104_REG_HPLOUT_CTRL;
        return HAL_OK;
    case AIC3104_OUTPUT_LINE:
        *reg = right_channel ? AIC3104_REG_RLOPM_CTRL : AIC3104_REG_LLOPM_CTRL;
        return HAL_OK;
    case AIC3104_OUTPUT_HPCOM:
        *reg = right_channel ? AIC3104_REG_HPRCOM_CTRL : AIC3104_REG_HPLCOM_CTRL;
        return HAL_OK;
    default:
        return HAL_ERROR;
    }
}

static HAL_StatusTypeDef aic3104_mixer_reg(AIC3104_OutputType_e output,
                                           bool right_channel,
                                           AIC3104_MixerInput_e source,
                                           uint8_t *reg)
{
    if (reg == NULL) {
        return HAL_ERROR;
    }

    switch (output) {
    case AIC3104_OUTPUT_HP:
        if (!right_channel) {
            if (source == AIC3104_MIXER_PGAL) *reg = AIC3104_REG_PGAL_TO_HPLOUT;
            else if (source == AIC3104_MIXER_DACL1) *reg = AIC3104_REG_DACL1_TO_HPLOUT;
            else if (source == AIC3104_MIXER_PGAR) *reg = AIC3104_REG_PGAR_TO_HPLOUT;
            else if (source == AIC3104_MIXER_DACR1) *reg = AIC3104_REG_DACR1_TO_HPLOUT;
            else return HAL_ERROR;
            return HAL_OK;
        }
        if (source == AIC3104_MIXER_PGAL) *reg = AIC3104_REG_PGAL_TO_HPROUT;
        else if (source == AIC3104_MIXER_DACL1) *reg = AIC3104_REG_DACL1_TO_HPROUT;
        else if (source == AIC3104_MIXER_PGAR) *reg = AIC3104_REG_PGAR_TO_HPROUT;
        else if (source == AIC3104_MIXER_DACR1) *reg = AIC3104_REG_DACR1_TO_HPROUT;
        else return HAL_ERROR;
        return HAL_OK;

    case AIC3104_OUTPUT_LINE:
        if (!right_channel) {
            if (source == AIC3104_MIXER_PGAL) *reg = AIC3104_REG_PGAL_TO_LLOPM;
            else if (source == AIC3104_MIXER_DACL1) *reg = AIC3104_REG_DACL1_TO_LLOPM;
            else if (source == AIC3104_MIXER_PGAR) *reg = AIC3104_REG_PGAR_TO_LLOPM;
            else if (source == AIC3104_MIXER_DACR1) *reg = AIC3104_REG_DACR1_TO_LLOPM;
            else return HAL_ERROR;
            return HAL_OK;
        }
        if (source == AIC3104_MIXER_PGAL) *reg = AIC3104_REG_PGAL_TO_RLOPM;
        else if (source == AIC3104_MIXER_DACL1) *reg = AIC3104_REG_DACL1_TO_RLOPM;
        else if (source == AIC3104_MIXER_PGAR) *reg = AIC3104_REG_PGAR_TO_RLOPM;
        else if (source == AIC3104_MIXER_DACR1) *reg = AIC3104_REG_DACR1_TO_RLOPM;
        else return HAL_ERROR;
        return HAL_OK;

    case AIC3104_OUTPUT_HPCOM:
        if (!right_channel) {
            if (source == AIC3104_MIXER_PGAL) *reg = AIC3104_REG_PGAL_TO_HPLCOM;
            else if (source == AIC3104_MIXER_DACL1) *reg = AIC3104_REG_DACL1_TO_HPLCOM;
            else if (source == AIC3104_MIXER_PGAR) *reg = AIC3104_REG_PGAR_TO_HPLCOM;
            else if (source == AIC3104_MIXER_DACR1) *reg = AIC3104_REG_DACR1_TO_HPLCOM;
            else return HAL_ERROR;
            return HAL_OK;
        }
        if (source == AIC3104_MIXER_PGAL) *reg = AIC3104_REG_PGAL_TO_HPRCOM;
        else if (source == AIC3104_MIXER_DACL1) *reg = AIC3104_REG_DACL1_TO_HPRCOM;
        else if (source == AIC3104_MIXER_PGAR) *reg = AIC3104_REG_PGAR_TO_HPRCOM;
        else if (source == AIC3104_MIXER_DACR1) *reg = AIC3104_REG_DACR1_TO_HPRCOM;
        else return HAL_ERROR;
        return HAL_OK;

    default:
        return HAL_ERROR;
    }
}

void AIC3104_DefaultConfig(AIC3104_Config_t *config, I2C_HandleTypeDef *hi2c)
{
    if (config == NULL) {
        return;
    }

    memset(config, 0, sizeof(*config));
    config->hi2c = hi2c;
    config->i2c_addr_7bit = AIC3104_I2C_ADDR_0;
    config->i2c_timeout_ms = AIC3104_TIMEOUT_DEFAULT_MS;

    config->mclk_freq = 12288000U;
    config->sample_rate = AIC3104_FS_48K;
    config->i2s_mode = AIC3104_MODE_SLAVE;
    config->clock_input = AIC3104_CLKIN_MCLK;

    config->clock.sample_rate_reg = 0x00U;
    config->clock.codec_datapath_reg = 0x0AU;
    config->clock.asd_if_a_reg = 0x00U;
    config->clock.asd_if_b_reg = 0x00U;
    config->clock.asd_if_c_reg = 0x00U;
    config->clock.pll_a_reg = 0x00U;
    config->clock.pll_b_reg = 0x00U;
    config->clock.pll_c_reg = 0x00U;
    config->clock.pll_d_reg = 0x00U;
    config->clock.pllr_reg = 0x01U;
    config->clock.clkgen_reg = 0x00U;

    config->dac_volume_left = 0x00U;
    config->dac_volume_right = 0x00U;
    config->adc_pga_left = 0x20U;
    config->adc_pga_right = 0x20U;

    config->micbias = AIC3104_MICBIAS_OFF;
    config->output_common_mode = 2U;

    config->enable_adc = true;
    config->enable_dac = true;
    config->enable_hp = true;
    config->enable_lineout = false;
    config->enable_hpcom = false;

    (void)aic3104_sync_sample_rate_reg(config);
}

HAL_StatusTypeDef AIC3104_WriteReg(AIC3104_Config_t *config, uint8_t reg, uint8_t val)
{
    uint8_t addr;
    uint32_t timeout;

    if ((config == NULL) || (config->hi2c == NULL)) {
        return HAL_ERROR;
    }

    addr = (uint8_t)(config->i2c_addr_7bit << 1);
    timeout = (config->i2c_timeout_ms == 0U) ? AIC3104_TIMEOUT_DEFAULT_MS : config->i2c_timeout_ms;

    return HAL_I2C_Mem_Write(config->hi2c, addr, reg, I2C_MEMADD_SIZE_8BIT, &val, 1U, timeout);
}

HAL_StatusTypeDef AIC3104_ReadReg(AIC3104_Config_t *config, uint8_t reg, uint8_t *val)
{
    uint8_t addr;
    uint32_t timeout;

    if ((config == NULL) || (config->hi2c == NULL) || (val == NULL)) {
        return HAL_ERROR;
    }

    addr = (uint8_t)(config->i2c_addr_7bit << 1);
    timeout = (config->i2c_timeout_ms == 0U) ? AIC3104_TIMEOUT_DEFAULT_MS : config->i2c_timeout_ms;

    return HAL_I2C_Mem_Read(config->hi2c, addr, reg, I2C_MEMADD_SIZE_8BIT, val, 1U, timeout);
}

HAL_StatusTypeDef AIC3104_UpdateBits(AIC3104_Config_t *config, uint8_t reg, uint8_t mask, uint8_t val)
{
    HAL_StatusTypeDef st;
    uint8_t cur;

    st = AIC3104_ReadReg(config, reg, &cur);
    if (st != HAL_OK) {
        return st;
    }

    cur = (uint8_t)((cur & (uint8_t)(~mask)) | (val & mask));
    return AIC3104_WriteReg(config, reg, cur);
}

HAL_StatusTypeDef AIC3104_HardwareReset(AIC3104_Config_t *config)
{
    if (config == NULL) {
        return HAL_ERROR;
    }

    if (config->reset_port == NULL) {
        return HAL_OK;
    }

    HAL_GPIO_WritePin(config->reset_port, config->reset_pin, GPIO_PIN_RESET);
    HAL_Delay(1U);
    HAL_GPIO_WritePin(config->reset_port, config->reset_pin, GPIO_PIN_SET);
    HAL_Delay(10U);

    return HAL_OK;
}

HAL_StatusTypeDef AIC3104_ApplyClockConfig(AIC3104_Config_t *config)
{
    HAL_StatusTypeDef st;
    uint8_t asd_if_b;

    if (config == NULL) {
        return HAL_ERROR;
    }

    st = aic3104_select_page(config, AIC3104_PAGE_0);
    if (st != HAL_OK) return st;

    st = aic3104_write_checked(config, AIC3104_REG_SAMPLE_RATE, config->clock.sample_rate_reg);
    if (st != HAL_OK) return st;

    st = aic3104_write_checked(config, AIC3104_REG_PLL_A, config->clock.pll_a_reg);
    if (st != HAL_OK) return st;
    st = aic3104_write_checked(config, AIC3104_REG_PLL_B, config->clock.pll_b_reg);
    if (st != HAL_OK) return st;
    st = aic3104_write_checked(config, AIC3104_REG_PLL_C, config->clock.pll_c_reg);
    if (st != HAL_OK) return st;
    st = aic3104_write_checked(config, AIC3104_REG_PLL_D, config->clock.pll_d_reg);
    if (st != HAL_OK) return st;

    st = aic3104_write_checked(config, AIC3104_REG_CODEC_DATAPATH, config->clock.codec_datapath_reg);
    if (st != HAL_OK) return st;

    st = aic3104_write_checked(config, AIC3104_REG_ASD_IF_A, config->clock.asd_if_a_reg);
    if (st != HAL_OK) return st;

    asd_if_b = config->clock.asd_if_b_reg;
    if (config->i2s_mode == AIC3104_MODE_MASTER) {
        asd_if_b |= 0xC0U;
    } else {
        asd_if_b &= (uint8_t)(~0xC0U);
    }

    st = aic3104_write_checked(config, AIC3104_REG_ASD_IF_B, asd_if_b);
    if (st != HAL_OK) return st;

    st = aic3104_write_checked(config, AIC3104_REG_ASD_IF_C, config->clock.asd_if_c_reg);
    if (st != HAL_OK) return st;

    st = aic3104_write_checked(config, AIC3104_REG_OVRF_PLLR, config->clock.pllr_reg);
    if (st != HAL_OK) return st;

    return aic3104_write_checked(config, AIC3104_REG_CLKGEN_CTRL, config->clock.clkgen_reg);
}

HAL_StatusTypeDef AIC3104_SetDacMute(AIC3104_Config_t *config, bool mute)
{
    uint8_t v = mute ? AIC3104_DAC_MUTE_BIT : 0x00U;
    HAL_StatusTypeDef st;

    st = AIC3104_UpdateBits(config, AIC3104_REG_LDAC_VOL, AIC3104_DAC_MUTE_BIT, v);
    if (st != HAL_OK) return st;
    return AIC3104_UpdateBits(config, AIC3104_REG_RDAC_VOL, AIC3104_DAC_MUTE_BIT, v);
}

HAL_StatusTypeDef AIC3104_SetAdcMute(AIC3104_Config_t *config, AIC3104_ADCChannel_e channel, bool mute)
{
    uint8_t v = mute ? AIC3104_ADC_MUTE_BIT : 0x00U;
    HAL_StatusTypeDef st = HAL_OK;

    if ((channel == AIC3104_ADC_LEFT) || (channel == AIC3104_ADC_BOTH)) {
        st = AIC3104_UpdateBits(config, AIC3104_REG_LADC_VOL, AIC3104_ADC_MUTE_BIT, v);
        if (st != HAL_OK) return st;
    }

    if ((channel == AIC3104_ADC_RIGHT) || (channel == AIC3104_ADC_BOTH)) {
        st = AIC3104_UpdateBits(config, AIC3104_REG_RADC_VOL, AIC3104_ADC_MUTE_BIT, v);
        if (st != HAL_OK) return st;
    }

    return HAL_OK;
}

HAL_StatusTypeDef AIC3104_SetDacVolume(AIC3104_Config_t *config, uint8_t left, uint8_t right)
{
    HAL_StatusTypeDef st;

    st = AIC3104_UpdateBits(config, AIC3104_REG_LDAC_VOL, (uint8_t)(~AIC3104_DAC_MUTE_BIT), left);
    if (st != HAL_OK) return st;

    return AIC3104_UpdateBits(config, AIC3104_REG_RDAC_VOL, (uint8_t)(~AIC3104_DAC_MUTE_BIT), right);
}

HAL_StatusTypeDef AIC3104_SetAdcPga(AIC3104_Config_t *config, uint8_t left, uint8_t right)
{
    HAL_StatusTypeDef st;

    st = AIC3104_UpdateBits(config, AIC3104_REG_LADC_VOL, 0x7FU, left);
    if (st != HAL_OK) return st;

    return AIC3104_UpdateBits(config, AIC3104_REG_RADC_VOL, 0x7FU, right);
}

HAL_StatusTypeDef AIC3104_SetAdcHpf(AIC3104_Config_t *config, AIC3104_ADCHpf_e hpf)
{
    uint8_t v = (uint8_t)(((uint8_t)hpf & 0x03U) << 4);

    return AIC3104_UpdateBits(config, AIC3104_REG_CODEC_DFILT, 0x30U, v);
}

HAL_StatusTypeDef AIC3104_SetDeemphasis(AIC3104_Config_t *config, bool left_enable, bool right_enable)
{
    uint8_t v = 0U;

    if (left_enable) {
        v |= 0x04U;
    }
    if (right_enable) {
        v |= 0x01U;
    }

    return AIC3104_UpdateBits(config, AIC3104_REG_CODEC_DFILT, 0x05U, v);
}

HAL_StatusTypeDef AIC3104_SetMicBias(AIC3104_Config_t *config, AIC3104_MicBias_e micbias)
{
    uint8_t v = (uint8_t)(((uint8_t)micbias & 0x03U) << 6);

    return AIC3104_UpdateBits(config, AIC3104_REG_MICBIAS, 0xC0U, v);
}

HAL_StatusTypeDef AIC3104_SetAgc(AIC3104_Config_t *config,
                                  AIC3104_ADCChannel_e channel,
                                  bool enable,
                                  uint8_t target_level,
                                  uint8_t attack_time,
                                  uint8_t decay_time)
{
    HAL_StatusTypeDef st;
    uint8_t regv;

    regv = (uint8_t)((enable ? 0x80U : 0x00U) |
                     ((target_level & 0x07U) << 4) |
                     ((attack_time & 0x03U) << 2) |
                     (decay_time & 0x03U));

    if ((channel == AIC3104_ADC_LEFT) || (channel == AIC3104_ADC_BOTH)) {
        st = AIC3104_WriteReg(config, AIC3104_REG_LAGC_A, regv);
        if (st != HAL_OK) return st;
    }

    if ((channel == AIC3104_ADC_RIGHT) || (channel == AIC3104_ADC_BOTH)) {
        st = AIC3104_WriteReg(config, AIC3104_REG_RAGC_A, regv);
        if (st != HAL_OK) return st;
    }

    return HAL_OK;
}

HAL_StatusTypeDef AIC3104_SetInputRoute(AIC3104_Config_t *config,
                                         AIC3104_ADCChannel_e adc,
                                         AIC3104_Input_e input,
                                         bool differential,
                                         bool enable)
{
    HAL_StatusTypeDef st;
    uint8_t route_bits;
    uint8_t reg = 0U;

    if (adc == AIC3104_ADC_BOTH) {
        st = AIC3104_SetInputRoute(config, AIC3104_ADC_LEFT, input, differential, enable);
        if (st != HAL_OK) return st;
        return AIC3104_SetInputRoute(config, AIC3104_ADC_RIGHT, input, differential, enable);
    }

    if (adc == AIC3104_ADC_LEFT) {
        if (input == AIC3104_INPUT_LINE1L) reg = AIC3104_REG_LINE1L_TO_LADC;
        else if (input == AIC3104_INPUT_LINE1R) reg = AIC3104_REG_LINE1R_TO_LADC;
        else if (input == AIC3104_INPUT_MIC2L) reg = AIC3104_REG_MIC3_TO_LADC;
        else if (input == AIC3104_INPUT_MIC2R) reg = AIC3104_REG_MIC3_TO_LADC;
        else return HAL_ERROR;
    } else {
        if (input == AIC3104_INPUT_LINE1L) reg = AIC3104_REG_LINE1L_TO_RADC;
        else if (input == AIC3104_INPUT_LINE1R) reg = AIC3104_REG_LINE1R_TO_RADC;
        else if (input == AIC3104_INPUT_MIC2L) reg = AIC3104_REG_MIC3_TO_RADC;
        else if (input == AIC3104_INPUT_MIC2R) reg = AIC3104_REG_MIC3_TO_RADC;
        else return HAL_ERROR;
    }

    route_bits = enable ? 0x00U : 0x0FU;

    st = AIC3104_UpdateBits(config, reg, 0x0FU, route_bits);
    if (st != HAL_OK) return st;

    if (differential) {
        return AIC3104_UpdateBits(config, reg, 0x80U, 0x80U);
    }

    return AIC3104_UpdateBits(config, reg, 0x80U, 0x00U);
}

HAL_StatusTypeDef AIC3104_SetMixerRoute(AIC3104_Config_t *config,
                                         AIC3104_OutputType_e output,
                                         bool right_channel,
                                         AIC3104_MixerInput_e source,
                                         bool enable,
                                         uint8_t gain)
{
    uint8_t reg;
    HAL_StatusTypeDef st;
    uint8_t value;

    st = aic3104_mixer_reg(output, right_channel, source, &reg);
    if (st != HAL_OK) {
        return st;
    }

    value = (uint8_t)(gain & 0x7FU);
    if (enable) {
        value |= AIC3104_ROUTE_ON;
    }

    return AIC3104_WriteReg(config, reg, value);
}

HAL_StatusTypeDef AIC3104_SetOutputDriver(AIC3104_Config_t *config,
                                           AIC3104_OutputType_e output,
                                           bool right_channel,
                                           bool power_on,
                                           bool unmute,
                                           uint8_t volume_step)
{
    HAL_StatusTypeDef st;
    uint8_t reg;
    uint8_t value;

    st = aic3104_output_ctrl_reg(output, right_channel, &reg);
    if (st != HAL_OK) {
        return st;
    }

    value = (uint8_t)((volume_step & 0x0FU) << AIC3104_OUTPUT_VOL_SHIFT);
    if (power_on) {
        value |= AIC3104_OUTPUT_PWR_BIT;
    }
    if (unmute) {
        value |= AIC3104_OUTPUT_SWITCH_BIT;
    }

    return AIC3104_WriteReg(config, reg, value);
}

HAL_StatusTypeDef AIC3104_Init(AIC3104_Config_t *config)
{
    HAL_StatusTypeDef st;
    uint8_t hpout_sc;

    if ((config == NULL) || (config->hi2c == NULL)) {
        return HAL_ERROR;
    }

    st = AIC3104_HardwareReset(config);
    if (st != HAL_OK) return st;

    st = aic3104_select_page(config, AIC3104_PAGE_0);
    if (st != HAL_OK) return st;

    st = aic3104_write_checked(config, AIC3104_REG_SOFT_RESET, AIC3104_SOFT_RESET);
    if (st != HAL_OK) return st;

    HAL_Delay(5U);

    st = aic3104_sync_sample_rate_reg(config);
    if (st != HAL_OK) return st;

    st = AIC3104_ApplyClockConfig(config);
    if (st != HAL_OK) return st;

    hpout_sc = (uint8_t)((config->output_common_mode & 0x03U) << 6);
    st = AIC3104_UpdateBits(config, AIC3104_REG_HPOUT_SC, 0xC0U, hpout_sc);
    if (st != HAL_OK) return st;

    st = AIC3104_SetMicBias(config, config->micbias);
    if (st != HAL_OK) return st;

    if (config->enable_dac) {
        st = AIC3104_UpdateBits(config,
                                AIC3104_REG_DAC_PWR,
                                (uint8_t)(AIC3104_DAC_PWR_L_ON | AIC3104_DAC_PWR_R_ON),
                                (uint8_t)(AIC3104_DAC_PWR_L_ON | AIC3104_DAC_PWR_R_ON));
        if (st != HAL_OK) return st;

        st = AIC3104_SetDacVolume(config, config->dac_volume_left, config->dac_volume_right);
        if (st != HAL_OK) return st;

        st = AIC3104_SetDacMute(config, false);
        if (st != HAL_OK) return st;

        st = AIC3104_WriteReg(config, AIC3104_REG_DAC_LINE_MUX, 0x00U);
        if (st != HAL_OK) return st;
    }

    if (config->enable_adc) {
        st = AIC3104_UpdateBits(config, AIC3104_REG_LINE1L_TO_LADC, AIC3104_ADC_PWR_ON, AIC3104_ADC_PWR_ON);
        if (st != HAL_OK) return st;
        st = AIC3104_UpdateBits(config, AIC3104_REG_LINE1R_TO_RADC, AIC3104_ADC_PWR_ON, AIC3104_ADC_PWR_ON);
        if (st != HAL_OK) return st;

        st = AIC3104_SetAdcPga(config, config->adc_pga_left, config->adc_pga_right);
        if (st != HAL_OK) return st;

        st = AIC3104_SetAdcMute(config, AIC3104_ADC_BOTH, false);
        if (st != HAL_OK) return st;

        st = AIC3104_SetInputRoute(config, AIC3104_ADC_LEFT, AIC3104_INPUT_LINE1L, false, true);
        if (st != HAL_OK) return st;
        st = AIC3104_SetInputRoute(config, AIC3104_ADC_RIGHT, AIC3104_INPUT_LINE1R, false, true);
        if (st != HAL_OK) return st;
    }

    st = AIC3104_SetMixerRoute(config, AIC3104_OUTPUT_HP, false, AIC3104_MIXER_DACL1, true, 0x00U);
    if (st != HAL_OK) return st;
    st = AIC3104_SetMixerRoute(config, AIC3104_OUTPUT_HP, true, AIC3104_MIXER_DACR1, true, 0x00U);
    if (st != HAL_OK) return st;

    st = AIC3104_SetOutputDriver(config, AIC3104_OUTPUT_HP, false, config->enable_hp, config->enable_hp, 0U);
    if (st != HAL_OK) return st;
    st = AIC3104_SetOutputDriver(config, AIC3104_OUTPUT_HP, true, config->enable_hp, config->enable_hp, 0U);
    if (st != HAL_OK) return st;

    st = AIC3104_SetOutputDriver(config, AIC3104_OUTPUT_LINE, false, config->enable_lineout, config->enable_lineout, 0U);
    if (st != HAL_OK) return st;
    st = AIC3104_SetOutputDriver(config, AIC3104_OUTPUT_LINE, true, config->enable_lineout, config->enable_lineout, 0U);
    if (st != HAL_OK) return st;

    st = AIC3104_SetOutputDriver(config, AIC3104_OUTPUT_HPCOM, false, config->enable_hpcom, config->enable_hpcom, 0U);
    if (st != HAL_OK) return st;
    st = AIC3104_SetOutputDriver(config, AIC3104_OUTPUT_HPCOM, true, config->enable_hpcom, config->enable_hpcom, 0U);
    if (st != HAL_OK) return st;

    return HAL_OK;
}
