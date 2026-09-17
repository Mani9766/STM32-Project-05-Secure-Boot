/*
 * flash_storage.c
 *
 *  Created on: Sep 15, 2026
 *      Author: Manisha Daigavhane
 */

#include "flash_storage.h"
#include <string.h>

#define FLASH_METADATA_SECTOR    FLASH_SECTOR_2
#define FLASH_METADATA_VOLTAGE   FLASH_VOLTAGE_RANGE_3
#define FLASH_METADATA_ADDRESS  0x08008000U

HAL_StatusTypeDef FlashStorage_EraseMetadataSector(void)
{
    FLASH_EraseInitTypeDef erase_init;
    uint32_t sector_error = 0xFFFFFFFFU;
    HAL_StatusTypeDef status;

    erase_init.TypeErase = FLASH_TYPEERASE_SECTORS;
    erase_init.Sector = FLASH_METADATA_SECTOR;
    erase_init.NbSectors = 1U;
    erase_init.VoltageRange = FLASH_METADATA_VOLTAGE;

    HAL_FLASH_Unlock();

    status = HAL_FLASHEx_Erase(&erase_init, &sector_error);

    HAL_FLASH_Lock();

    return status;
}

HAL_StatusTypeDef FlashStorage_ProgramMetadata(
    const firmware_metadata_t *metadata)
{
    uint32_t address = FLASH_METADATA_ADDRESS;
    uint32_t data;
    HAL_StatusTypeDef status;

    HAL_FLASH_Unlock();

    for (uint32_t offset = 0U;
         offset < sizeof(firmware_metadata_t);
         offset += sizeof(uint32_t))
    {
        memcpy(&data,
               ((const uint8_t *)metadata) + offset,
               sizeof(uint32_t));

        status = HAL_FLASH_Program(
            FLASH_TYPEPROGRAM_WORD,
            address + offset,
            data);

        if (status != HAL_OK)
        {
            HAL_FLASH_Lock();
            return status;
        }
    }

    HAL_FLASH_Lock();

    return HAL_OK;
}

HAL_StatusTypeDef FlashStorage_ReadMetadata(
    firmware_metadata_t *metadata)
{
    if (metadata == NULL)
    {
        return HAL_ERROR;
    }

    memcpy(metadata,
           (const void *)FLASH_METADATA_ADDRESS,
           sizeof(firmware_metadata_t));

    return HAL_OK;
}
