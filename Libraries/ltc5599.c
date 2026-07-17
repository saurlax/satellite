#include "ltc5599.h"

#include "main.h"
#include "spi.h"
#include <string.h>

static uint32_t ltc5599_timeout_ms(const LTC5599_Config_t *config)
{
    return ((config != NULL) && (config->spi_timeout_ms != 0U)) ?
               config->spi_timeout_ms :
               LTC5599_SPI_TIMEOUT_DEFAULT_MS;
}

void LTC5599_DefaultConfig(LTC5599_Config_t *config, SPI_HandleTypeDef *hspi)
{
    if (config == NULL) {
        return;
    }

    memset(config, 0, sizeof(*config));
    config->hspi = hspi;
    config->cs_port = LTC5599_CS_GPIO_Port;
    config->cs_pin = LTC5599_CS_Pin;
    config->spi_timeout_ms = LTC5599_SPI_TIMEOUT_DEFAULT_MS;
}

HAL_StatusTypeDef LTC5599_Transfer(LTC5599_Config_t *config,
                                   uint8_t address,
                                   uint8_t rw,
                                   uint8_t tx,
                                   uint8_t *rx)
{
    uint8_t tx_buf[2];
    uint8_t rx_buf[2] = {0U, 0U};
    HAL_StatusTypeDef st;

    if ((config == NULL) || (config->hspi == NULL) || (config->cs_port == NULL)) {
        return HAL_ERROR;
    }

    tx_buf[0] = (uint8_t)((address << 1) | (rw & 0x01U));
    tx_buf[1] = (rw == LTC5599_WRITE) ? tx : 0x00U;

    HAL_GPIO_WritePin(config->cs_port, config->cs_pin, GPIO_PIN_RESET);
    st = HAL_SPI_TransmitReceive(config->hspi, tx_buf, rx_buf, sizeof(tx_buf), ltc5599_timeout_ms(config));
    HAL_GPIO_WritePin(config->cs_port, config->cs_pin, GPIO_PIN_SET);

    if ((st == HAL_OK) && (rx != NULL)) {
        *rx = rx_buf[1];
    }

    return st;
}

HAL_StatusTypeDef LTC5599_WriteReg(LTC5599_Config_t *config, uint8_t address, uint8_t value)
{
    return LTC5599_Transfer(config, address, LTC5599_WRITE, value, NULL);
}

HAL_StatusTypeDef LTC5599_ReadReg(LTC5599_Config_t *config, uint8_t address, uint8_t *value)
{
    if (value == NULL) {
        return HAL_ERROR;
    }

    return LTC5599_Transfer(config, address, LTC5599_READ, 0x00U, value);
}

HAL_StatusTypeDef LTC5599_Probe(LTC5599_Config_t *config, uint8_t *reg0_value)
{
    return LTC5599_ReadReg(config, 0x00U, reg0_value);
}
