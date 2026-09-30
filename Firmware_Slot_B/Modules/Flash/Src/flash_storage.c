/*
 * flash_storage.c
 *
 *  Created on: Sep 15, 2026
 *      Author: Manisha Daigavhane
 */

#include "flash_storage.h"
#include <string.h>

#define FLASH_CANDIDATE_METADATA_SECTOR    FLASH_SECTOR_3
#define FLASH_CANDIDATE_METADATA_VOLTAGE   FLASH_VOLTAGE_RANGE_3
#define FLASH_CANDIDATE_METADATA_ADDRESS   0x0800C000U

HAL_StatusTypeDef FlashStorage_EraseCandidateMetadataSector(void)
{
    FLASH_EraseInitTypeDef erase_init = {0};
    uint32_t sector_error = 0U;

    erase_init.TypeErase    = FLASH_TYPEERASE_SECTORS;
    erase_init.Sector       = FLASH_CANDIDATE_METADATA_SECTOR;
    erase_init.NbSectors    = 1U;
    erase_init.VoltageRange = FLASH_CANDIDATE_METADATA_VOLTAGE;

    HAL_FLASH_Unlock();

    HAL_StatusTypeDef status =
        HAL_FLASHEx_Erase(&erase_init, &sector_error);

    HAL_FLASH_Lock();

    return status;
}

HAL_StatusTypeDef FlashStorage_ProgramMetadata(
    const firmware_metadata_t *metadata)
{
    const uint32_t *data = (const uint32_t *)metadata;

    HAL_FLASH_Unlock();

    for (uint32_t i = 0U;
         i < (sizeof(firmware_metadata_t) / sizeof(uint32_t));
         i++)
    {
        if (HAL_FLASH_Program(
                FLASH_TYPEPROGRAM_WORD,
                FLASH_CANDIDATE_METADATA_ADDRESS + (i * sizeof(uint32_t)),
                data[i]) != HAL_OK)
        {
            HAL_FLASH_Lock();
            return HAL_ERROR;
        }
    }

    HAL_FLASH_Lock();

    return HAL_OK;
}
