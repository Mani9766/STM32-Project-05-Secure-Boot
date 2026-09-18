/*
 * flash_storage.h
 *
 *  Created on: Sep 15, 2026
 *      Author: Manisha Daigavhane
 */

#ifndef FLASH_STORAGE_H
#define FLASH_STORAGE_H

#include "stm32f4xx_hal.h"
#include "metadata.h"

HAL_StatusTypeDef FlashStorage_EraseSector(
    uint32_t sector,
    uint32_t voltage_range);

HAL_StatusTypeDef FlashStorage_ProgramMetadata(
    uint32_t address,
    const firmware_metadata_t *metadata);

HAL_StatusTypeDef FlashStorage_ReadMetadata(
    uint32_t address,
    firmware_metadata_t *metadata);

#endif /* FLASH_STORAGE_H_ */
