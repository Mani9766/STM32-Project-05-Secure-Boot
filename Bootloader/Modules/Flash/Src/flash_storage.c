#include "flash_storage.h"
#include <string.h>

HAL_StatusTypeDef FlashStorage_EraseSector(
    uint32_t sector,
    uint32_t voltage_range)
{
    FLASH_EraseInitTypeDef erase_init = {0};
    uint32_t sector_error = 0xFFFFFFFFU;
    HAL_StatusTypeDef status;

    erase_init.TypeErase = FLASH_TYPEERASE_SECTORS;
    erase_init.Sector = sector;
    erase_init.NbSectors = 1U;
    erase_init.VoltageRange = voltage_range;

    HAL_FLASH_Unlock();

    status = HAL_FLASHEx_Erase(&erase_init, &sector_error);

    HAL_FLASH_Lock();

    return status;
}

HAL_StatusTypeDef FlashStorage_ProgramMetadata(
    uint32_t address,
    const firmware_metadata_t *metadata)
{
    uint32_t data;
    HAL_StatusTypeDef status;

    if (metadata == NULL)
    {
        return HAL_ERROR;
    }

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
    uint32_t address,
    firmware_metadata_t *metadata)
{
    if (metadata == NULL)
    {
        return HAL_ERROR;
    }

    memcpy(metadata,
           (const void *)address,
           sizeof(firmware_metadata_t));

    return HAL_OK;
}

HAL_StatusTypeDef FlashStorage_ProgramImage(
    uint32_t destination_address,
    uint32_t source_address,
    uint32_t image_size)
{
    uint32_t data;
    HAL_StatusTypeDef status;

    if ((image_size == 0U) ||
        (destination_address == 0U) ||
        (source_address == 0U))
    {
        return HAL_ERROR;
    }

    HAL_FLASH_Unlock();

    for (uint32_t offset = 0U;
         offset < image_size;
         offset += sizeof(uint32_t))
    {
        data = 0xFFFFFFFFU;

        uint32_t remaining = image_size - offset;
        uint32_t copy_size =
            (remaining >= sizeof(uint32_t)) ?
            sizeof(uint32_t) :
            remaining;

        memcpy(
            &data,
            (const void *)(source_address + offset),
            copy_size);

        status = HAL_FLASH_Program(
            FLASH_TYPEPROGRAM_WORD,
            destination_address + offset,
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
