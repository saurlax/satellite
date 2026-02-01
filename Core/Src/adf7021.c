#include "adf7021.h"
#include "string.h"

/* ============================================================
 * Helper Functions
 * ============================================================ */

/**
 * @brief  Microsecond delay using HAL_Delay
 * @param  us: Microseconds to delay
 * @note   For delays < 1ms, uses HAL_Delay(1) as minimum granularity
 */
static void ADF7021_DelayUs(uint32_t us)
{
    if (us >= 1000) {
        HAL_Delay(us / 1000);
    } else {
        HAL_Delay(1); /* Minimum 1ms granularity */
    }
}

/* ============================================================
 * Register Write/Read Functions
 * ============================================================ */

/**
 * @brief  Write a 32-bit register to ADF7021 via SPI
 * @param  config: Pointer to ADF7021 configuration structure
 * @param  reg_num: Register number (0-9)
 * @param  reg_data: 32-bit register data (already shifted by reg_num)
 * @retval HAL_StatusTypeDef
 * 
 * @note   Register format: [7:5]=reg_num, [24:0]=register data
 */
HAL_StatusTypeDef ADF7021_WriteReg(ADF7021_Config_t *config, uint8_t reg_num, uint32_t reg_data)
{
    if (!config || !config->hspi) {
        return HAL_ERROR;
    }
    
    /* Ensure reg_num is in valid range */
    if (reg_num > 9) {
        return HAL_ERROR;
    }
    
    /* Construct SPI frame: MSByte first */
    uint8_t tx_buffer[4];
    uint32_t spi_data = ADF7021_REG_ADDR(reg_num) | (reg_data & 0x01FFFFFF);
    
    tx_buffer[0] = (spi_data >> 24) & 0xFF;
    tx_buffer[1] = (spi_data >> 16) & 0xFF;
    tx_buffer[2] = (spi_data >> 8) & 0xFF;
    tx_buffer[3] = spi_data & 0xFF;
    
    return HAL_SPI_Transmit(config->hspi, tx_buffer, 4, 100);
}

/**
 * @brief  Read a 32-bit register from ADF7021 via SPI
 * @param  config: Pointer to ADF7021 configuration structure
 * @retval 32-bit register data
 * 
 * @note   SPI read requires clocking out 4 bytes to retrieve the status word
 */
uint32_t ADF7021_ReadReg(ADF7021_Config_t *config)
{
    if (!config || !config->hspi) {
        return 0;
    }
    
    uint8_t rx_buffer[4] = {0};
    uint8_t tx_buffer[4] = {0xFF, 0xFF, 0xFF, 0xFF};
    
    HAL_SPI_TransmitReceive(config->hspi, tx_buffer, rx_buffer, 4, 100);
    
    return ((uint32_t)rx_buffer[0] << 24) | 
           ((uint32_t)rx_buffer[1] << 16) | 
           ((uint32_t)rx_buffer[2] << 8) | 
           (uint32_t)rx_buffer[3];
}

/* ============================================================
 * Hardware Reset and Enable
 * ============================================================ */

/**
 * @brief  Hardware enable (CE pin high) - initializes regulator
 * @param  config: Pointer to ADF7021 configuration structure
 * @retval HAL_StatusTypeDef
 */
static HAL_StatusTypeDef ADF7021_HardwareEnable(ADF7021_Config_t *config)
{
    if (!config || !config->en_port) {
        return HAL_ERROR;
    }
    
    /* Pull EN (CE) pin high to enable regulator */
    HAL_GPIO_WritePin(config->en_port, config->en_pin, GPIO_PIN_SET);
    
    /* Wait for regulator to stabilize (~50us) */
    ADF7021_DelayUs(50);
    
    return HAL_OK;
}

/* ============================================================
 * Calibration Functions
 * ============================================================ */

/**
 * @brief  Perform IF Filter Coarse Calibration
 * @param  config: Pointer to ADF7021 configuration structure
 * @retval HAL_StatusTypeDef
 * 
 * @note   Calibrates the IF filter to compensate for manufacturing tolerances
 *         Must be performed after PLL configuration
 */
static HAL_StatusTypeDef ADF7021_CalibrationIFCoarse(ADF7021_Config_t *config)
{
    /* Configure Register 5 with IF filter divider for coarse calibration */
    /* IF_FILTER_DIVIDER = XTAL_FREQ_HZ / 50000 (target 50kHz) */
    uint32_t if_filter_div = config->xtal_freq_hz / 50000UL;
    if (if_filter_div > 511) {
        if_filter_div = 511; /* 9-bit limit */
    }
    
    /* Register 5: [8:0] = IF_FILTER_DIVIDER */
    uint32_t reg5_data = if_filter_div & 0x1FF;
    
    /* Trigger coarse calibration by setting IF_COARSE_CAL bit (R5_DB4) */
    reg5_data |= (1U << 24); /* Bit 24 in the 25-bit register data */
    
    HAL_StatusTypeDef status = ADF7021_WriteReg(config, 5, reg5_data);
    if (status != HAL_OK) {
        return status;
    }
    
    /* Wait for calibration to complete (~5ms) */
    ADF7021_DelayUs(5000);
    
    return HAL_OK;
}

/**
 * @brief  Perform IF Filter Fine Calibration
 * @param  config: Pointer to ADF7021 configuration structure
 * @retval HAL_StatusTypeDef
 * 
 * @note   Optional, for higher precision. Must be after coarse calibration.
 */
static HAL_StatusTypeDef ADF7021_CalibrationIFFine(ADF7021_Config_t *config)
{
    /* Configure Register 6 for fine calibration */
    /* In practice, fine calibration uses similar divider settings */
    uint32_t if_filter_div = config->xtal_freq_hz / 100000UL; /* 100kHz target for fine */
    if (if_filter_div > 511) {
        if_filter_div = 511;
    }
    
    uint32_t reg6_data = if_filter_div & 0x1FF;
    
    /* Trigger fine calibration by setting IF_FINE_CAL bit */
    reg6_data |= (1U << 24);
    
    HAL_StatusTypeDef status = ADF7021_WriteReg(config, 6, reg6_data);
    if (status != HAL_OK) {
        return status;
    }
    
    /* Wait for calibration to complete */
    ADF7021_DelayUs(5000);
    
    return HAL_OK;
}

/**
 * @brief  Perform Image Rejection Calibration
 * @param  config: Pointer to ADF7021 configuration structure
 * @retval HAL_StatusTypeDef
 * 
 * @note   Significantly improves image rejection ratio (>56dB)
 */
static HAL_StatusTypeDef ADF7021_CalibrationImageRejection(ADF7021_Config_t *config)
{
    /* Start IR calibration by setting IR_CAL_START bit in Register 1 */
    /* This is typically bit 24 (IR_CAL_START) */
    uint32_t reg1_ir_cal = (1U << 24);
    
    HAL_StatusTypeDef status = ADF7021_WriteReg(config, 1, reg1_ir_cal);
    if (status != HAL_OK) {
        return status;
    }
    
    /* Wait for IR calibration to complete (~10ms) */
    ADF7021_DelayUs(10000);
    
    return HAL_OK;
}

/* ============================================================
 * Core Configuration Functions
 * ============================================================ */

/**
 * @brief  Calculate PLL divider (N) from center frequency
 * @param  center_freq: Center frequency in Hz
 * @param  ref_freq: Reference frequency in Hz
 * @retval Calculated N value
 * 
 * @note   N = (Center_Freq_kHz * 2) / (Ref_Freq_kHz)
 *         Typically Ref_Freq = XTAL_FREQ or 2*XTAL_FREQ
 */
static uint16_t ADF7021_CalculatePLLDiv(uint32_t center_freq, uint32_t ref_freq)
{
    /* Simplified calculation; adjust based on actual frequency plan */
    uint32_t n_value = (center_freq * 2) / ref_freq;
    
    if (n_value > 8191) {
        n_value = 8191; /* 13-bit limit */
    }
    
    return (uint16_t)n_value;
}

/**
 * @brief  Calculate frequency deviation divider
 * @param  freq_dev: Frequency deviation in Hz
 * @param  ref_freq: Reference frequency in Hz
 * @retval Calculated divider value
 */
static uint16_t ADF7021_CalculateFreqDev(uint32_t freq_dev, uint32_t ref_freq)
{
    /* FREQ_DEVIATION = (Desired_Deviation_Hz / Ref_Freq_Hz) * 2^12 */
    uint32_t div_value = (freq_dev * 4096) / ref_freq;
    
    if (div_value > 4095) {
        div_value = 4095; /* 12-bit limit */
    }
    
    return (uint16_t)div_value;
}

/**
 * @brief  Configure Register 0 (PLL and Tx/Rx Mode)
 * @param  config: Pointer to ADF7021 configuration structure
 * @retval HAL_StatusTypeDef
 */
static HAL_StatusTypeDef ADF7021_ConfigReg0(ADF7021_Config_t *config)
{
    uint16_t n_value = ADF7021_CalculatePLLDiv(config->center_freq_hz, config->ref_freq_hz);
    
    /* Register 0: [12:0] = N divider, [13] = Tx/Rx mode (1=Tx, 0=Rx) */
    uint32_t reg0_data = n_value & 0x1FFF;
    
    /* Default to RX mode; can be changed later with ADF7021_SetTxMode */
    reg0_data |= (0 << 13); /* 0 = RX mode */
    
    return ADF7021_WriteReg(config, 0, reg0_data);
}

/**
 * @brief  Configure Register 1 (VCO, Oscillator, Calibration)
 * @param  config: Pointer to ADF7021 configuration structure
 * @retval HAL_StatusTypeDef
 */
static HAL_StatusTypeDef ADF7021_ConfigReg1(ADF7021_Config_t *config)
{
    uint32_t reg1_data = 0;
    
    /* [24] = MUXOUT_SELECT (0 = regulator ready) - for monitoring */
    /* [23] = MUXOUT_LEVEL */
    
    /* [17] = VCO_ENABLE (1 = enabled) */
    reg1_data |= (1U << 17);
    
    /* [16] = OSC_ENABLE (depends on TCXO configuration) */
    if (config->xtal_type == 1) {
        /* External TCXO: disable internal oscillator */
        reg1_data &= ~(1U << 16);
    } else {
        /* Crystal: enable internal oscillator */
        reg1_data |= (1U << 16);
    }
    
    /* [15:14] = XTAL_BIAS (00 = normal, for crystal) */
    /* [13:11] = XTAL_RANGE (depends on crystal frequency) */
    /* For TCXO, these are typically set based on input frequency */
    
    return ADF7021_WriteReg(config, 1, reg1_data);
}

/**
 * @brief  Configure Register 2 (Modulation, Frequency Deviation, TX Power)
 * @param  config: Pointer to ADF7021 configuration structure
 * @retval HAL_StatusTypeDef
 */
static HAL_StatusTypeDef ADF7021_ConfigReg2(ADF7021_Config_t *config)
{
    uint32_t reg2_data = 0;
    
    uint16_t freq_dev = ADF7021_CalculateFreqDev(config->freq_deviation, config->ref_freq_hz);
    
    /* [21:20] = MOD_TYPE (00=2FSK, 01=4FSK, 10=GFSK, 11=OOK) */
    reg2_data |= ((uint32_t)config->mod_type & 0x3) << 20;
    
    /* [19:8] = FREQ_DEVIATION (12 bits) */
    reg2_data |= (freq_dev & 0xFFF) << 8;
    
    /* [5:3] = TX_POWER (PA level) */
    reg2_data |= ((uint32_t)config->tx_power & 0x7) << 3;
    
    return ADF7021_WriteReg(config, 2, reg2_data);
}

/**
 * @brief  Configure Register 3 (Data Rate Clock)
 * @param  config: Pointer to ADF7021 configuration structure
 * @retval HAL_StatusTypeDef
 */
static HAL_StatusTypeDef ADF7021_ConfigReg3(ADF7021_Config_t *config)
{
    /* CLK_DIV = XTAL_FREQ / (2 * DATA_RATE) */
    uint32_t clk_div = config->xtal_freq_hz / (2 * config->data_rate_bps);
    
    if (clk_div > 65535) {
        clk_div = 65535; /* 16-bit limit */
    }
    
    /* Register 3: [15:0] = CLK_DIV */
    uint32_t reg3_data = clk_div & 0xFFFF;
    
    return ADF7021_WriteReg(config, 3, reg3_data);
}

/**
 * @brief  Configure Register 4 (RX Demodulator)
 * @param  config: Pointer to ADF7021 configuration structure
 * @retval HAL_StatusTypeDef
 */
static HAL_StatusTypeDef ADF7021_ConfigReg4(ADF7021_Config_t *config)
{
    uint32_t reg4_data = 0;
    
    /* [24:21] = SLICER_THRESHOLD (usually 0x8 = mid-range) */
    reg4_data |= (0x8 << 21);
    
    /* [20] = DEMOD_AGC_ENABLE (1 = enabled) */
    reg4_data |= (1U << 20);
    
    /* [19:16] = SLICER_MODE (depends on modulation) */
    /* For FSK modes, typically 0xA (slicer on DISCRIM) */
    reg4_data |= (0xA << 16);
    
    return ADF7021_WriteReg(config, 4, reg4_data);
}

/**
 * @brief  Configure Register 5 (IF Filter) - already done in calibration
 * @param  config: Pointer to ADF7021 configuration structure
 * @retval HAL_StatusTypeDef
 */
static HAL_StatusTypeDef ADF7021_ConfigReg5(ADF7021_Config_t *config)
{
    /* IF Filter bandwidth calculation */
    /* For standard IF filter: BWSEL = XTAL / (2 * DESIRED_BW) */
    uint32_t bw_sel = config->xtal_freq_hz / (2 * config->if_filter_bw);
    
    if (bw_sel > 511) {
        bw_sel = 511; /* 9-bit limit */
    }
    
    uint32_t reg5_data = bw_sel & 0x1FF;
    
    return ADF7021_WriteReg(config, 5, reg5_data);
}

/**
 * @brief  Configure Register 9 (AGC)
 * @param  config: Pointer to ADF7021 configuration structure
 * @retval HAL_StatusTypeDef
 */
static HAL_StatusTypeDef ADF7021_ConfigReg9(ADF7021_Config_t *config)
{
    uint32_t reg9_data = 0;
    
    /* [24] = AGC_ENABLE (1 = enabled) */
    reg9_data |= (1U << 24);
    
    /* [23:20] = AGC_ATTACK_TIME */
    /* [19:16] = AGC_DECAY_TIME */
    /* Use default values: attack = 0xE, decay = 0x9 */
    reg9_data |= (0xE << 20);
    reg9_data |= (0x9 << 16);
    
    return ADF7021_WriteReg(config, 9, reg9_data);
}

/* ============================================================
 * Main Initialization Function
 * ============================================================ */

/**
 * @brief  Initialize ADF7021 transceiver
 * @param  config: Pointer to ADF7021 configuration structure
 *         Must contain valid SPI handle, GPIO info, and frequency parameters
 * @retval HAL_StatusTypeDef
 * 
 * @note   Initialization sequence:
 *         1. Hardware enable (EN pin high)
 *         2. Register 1 configuration (clock source, VCO)
 *         3. Register 0-9 configuration (PLL, modulation, data rate, etc.)
 *         4. IF Filter Coarse Calibration
 *         5. IF Filter Fine Calibration
 *         6. Image Rejection Calibration
 */
HAL_StatusTypeDef ADF7021_Init(ADF7021_Config_t *config)
{
    HAL_StatusTypeDef status = HAL_OK;
    
    if (!config || !config->hspi || !config->en_port) {
        return HAL_ERROR;
    }
    
    /* Step 1: Hardware Enable (CE pin high) - enables regulator */
    status = ADF7021_HardwareEnable(config);
    if (status != HAL_OK) {
        return status;
    }
    
    /* Step 2: Configure Register 1 (Clock source for TCXO) */
    status = ADF7021_ConfigReg1(config);
    if (status != HAL_OK) {
        return status;
    }
    
    ADF7021_DelayUs(100);
    
    /* Step 3: Configure core registers (order matters) */
    status = ADF7021_ConfigReg0(config);
    if (status != HAL_OK) return status;
    
    status = ADF7021_ConfigReg2(config);
    if (status != HAL_OK) return status;
    
    status = ADF7021_ConfigReg3(config);
    if (status != HAL_OK) return status;
    
    status = ADF7021_ConfigReg4(config);
    if (status != HAL_OK) return status;
    
    status = ADF7021_ConfigReg5(config);
    if (status != HAL_OK) return status;
    
    status = ADF7021_ConfigReg9(config);
    if (status != HAL_OK) return status;
    
    ADF7021_DelayUs(100);
    
    /* Step 4: IF Filter Coarse Calibration */
    status = ADF7021_CalibrationIFCoarse(config);
    if (status != HAL_OK) {
        return status;
    }
    
    /* Step 5: IF Filter Fine Calibration */
    status = ADF7021_CalibrationIFFine(config);
    if (status != HAL_OK) {
        return status;
    }
    
    /* Step 6: Image Rejection Calibration */
    status = ADF7021_CalibrationImageRejection(config);
    if (status != HAL_OK) {
        return status;
    }
    
    ADF7021_DelayUs(100);
    
    return HAL_OK;
}

/* ============================================================
 * Mode and Control Functions
 * ============================================================ */

/**
 * @brief  Set ADF7021 to TX mode
 * @param  config: Pointer to ADF7021 configuration structure
 * @retval HAL_StatusTypeDef
 */
HAL_StatusTypeDef ADF7021_SetTxMode(ADF7021_Config_t *config)
{
    if (!config) {
        return HAL_ERROR;
    }
    
    /* Read Register 0, set bit 13 (TX/RX mode) to 1 for TX */
    uint16_t n_value = ADF7021_CalculatePLLDiv(config->center_freq_hz, config->ref_freq_hz);
    uint32_t reg0_data = (n_value & 0x1FFF) | (1U << 13); /* 1 = TX mode */
    
    return ADF7021_WriteReg(config, 0, reg0_data);
}

/**
 * @brief  Set ADF7021 to RX mode
 * @param  config: Pointer to ADF7021 configuration structure
 * @retval HAL_StatusTypeDef
 */
HAL_StatusTypeDef ADF7021_SetRxMode(ADF7021_Config_t *config)
{
    if (!config) {
        return HAL_ERROR;
    }
    
    /* Read Register 0, set bit 13 (TX/RX mode) to 0 for RX */
    uint16_t n_value = ADF7021_CalculatePLLDiv(config->center_freq_hz, config->ref_freq_hz);
    uint32_t reg0_data = (n_value & 0x1FFF) | (0U << 13); /* 0 = RX mode */
    
    return ADF7021_WriteReg(config, 0, reg0_data);
}

/**
 * @brief  Set TX Power Level
 * @param  config: Pointer to ADF7021 configuration structure
 * @param  power: Power level (0-7)
 * @retval HAL_StatusTypeDef
 */
HAL_StatusTypeDef ADF7021_SetTxPower(ADF7021_Config_t *config, ADF7021_PALevel_e power)
{
    if (!config) {
        return HAL_ERROR;
    }
    
    config->tx_power = power;
    
    /* Re-configure Register 2 with new power level */
    return ADF7021_ConfigReg2(config);
}

/**
 * @brief  Transmit data (placeholder for actual TX implementation)
 * @param  config: Pointer to ADF7021 configuration structure
 * @param  data: Pointer to data buffer
 * @param  length: Length of data
 * @retval HAL_StatusTypeDef
 * 
 * @note   This is a placeholder. Actual TX requires:
 *         - Setting TX mode via ADF7021_SetTxMode()
 *         - Feeding data bits to the serial data input (SDI) pin
 *         - Synchronizing with TX clock output
 */
HAL_StatusTypeDef ADF7021_Transmit(ADF7021_Config_t *config, uint8_t *data, uint16_t length)
{
    if (!config || !data) {
        return HAL_ERROR;
    }
    
    /* TODO: Implement actual TX data transmission */
    /* This typically involves:
       - Switching to TX mode
       - Feeding bits to SDI pin at the rate of the TX clock
       - Waiting for transmission to complete
    */
    
    return HAL_OK;
}

/**
 * @brief  Receive data (placeholder for actual RX implementation)
 * @param  config: Pointer to ADF7021 configuration structure
 * @param  data: Pointer to data buffer
 * @param  length: Length of data to receive
 * @retval HAL_StatusTypeDef
 * 
 * @note   This is a placeholder. Actual RX requires:
 *         - Setting RX mode via ADF7021_SetRxMode()
 *         - Reading data bits from the serial data output (SDO) pin
 *         - Synchronizing with RX clock output
 *         - Monitoring sync word detection (SWD) pin
 */
HAL_StatusTypeDef ADF7021_Receive(ADF7021_Config_t *config, uint8_t *data, uint16_t length)
{
    if (!config || !data) {
        return HAL_ERROR;
    }
    
    /* TODO: Implement actual RX data reception */
    /* This typically involves:
       - Switching to RX mode
       - Reading bits from SDO pin at the rate of the RX clock
       - Monitoring for sync word detection (optional)
       - Buffering received bits into bytes
    */
    
    return HAL_OK;
}
