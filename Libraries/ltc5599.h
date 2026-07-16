#ifndef LTC5599_H
#define LTC5599_H

#include "stm32h7xx_hal.h"
#include <stdint.h>

#define LTC5599_SPI_TIMEOUT_DEFAULT_MS 100U

#define LTC5599_WRITE 0U
#define LTC5599_READ  1U

typedef struct {
    SPI_HandleTypeDef *hspi;
    GPIO_TypeDef *cs_port;
    uint16_t cs_pin;
    uint32_t spi_timeout_ms;
} LTC5599_Config_t;

void LTC5599_DefaultConfig(LTC5599_Config_t *config, SPI_HandleTypeDef *hspi);
HAL_StatusTypeDef LTC5599_Transfer(LTC5599_Config_t *config,
                                   uint8_t address,
                                   uint8_t rw,
                                   uint8_t tx,
                                   uint8_t *rx);
HAL_StatusTypeDef LTC5599_WriteReg(LTC5599_Config_t *config, uint8_t address, uint8_t value);
HAL_StatusTypeDef LTC5599_ReadReg(LTC5599_Config_t *config, uint8_t address, uint8_t *value);
HAL_StatusTypeDef LTC5599_Probe(LTC5599_Config_t *config, uint8_t *reg0_value);

#endif /* LTC5599_H */
