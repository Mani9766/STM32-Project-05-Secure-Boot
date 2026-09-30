/*
 * metadata.c
 *
 *  Created on: Sep 12, 2026
 *      Author: Manisha Daigavhane
 */

#include "metadata.h"

#include <stddef.h>

#include "stm32f4xx_hal.h"

extern CRC_HandleTypeDef hcrc;

bool Metadata_Validate(
    const firmware_metadata_t *metadata,
    uintptr_t image_start,
    uintptr_t image_region_end)
{
    uintptr_t image_end;

    if (metadata == NULL)
    {
        return false;
    }

    if (metadata->magic != FIRMWARE_METADATA_MAGIC)
    {
        return false;
    }

    if (metadata->image_size == 0U)
    {
        return false;
    }

    image_end =
        image_start + (uintptr_t)metadata->image_size;

    if (image_end < image_start)
    {
        return false;
    }

    if (image_end > image_region_end)
    {
        return false;
    }

    return true;
}

uint32_t Metadata_CalculateRecordCRC(
    const firmware_metadata_record_t *record)
{
    size_t crc_data_length;

    if (record == NULL)
    {
        return 0U;
    }

    /*
     * Calculate CRC over:
     * record_magic
     * sequence
     * metadata
     *
     * Do not include metadata_crc or commit_marker.
     */
    crc_data_length =
        offsetof(
            firmware_metadata_record_t,
            metadata_crc);

    return HAL_CRC_Calculate(
        &hcrc,
        (uint32_t *)record,
        (uint32_t)(crc_data_length / sizeof(uint32_t)));
}

bool Metadata_ValidateRecord(
    const firmware_metadata_record_t *record)
{
    uint32_t calculated_crc;

    if (record == NULL)
    {
        return false;
    }

    /*
     * Verify record identification.
     */
    if (record->record_magic != METADATA_RECORD_MAGIC)
    {
        return false;
    }

    /*
     * Commit marker is written last.
     * An incomplete/erased record will not contain
     * the expected commit marker.
     */
    if (record->commit_marker != METADATA_COMMIT_MARKER)
    {
        return false;
    }

    /*
     * Verify record contents.
     */
    calculated_crc =
        Metadata_CalculateRecordCRC(record);

    if (calculated_crc != record->metadata_crc)
    {
        return false;
    }

    return true;
}

bool Metadata_IsSequenceNewer(
    uint32_t current_sequence,
    uint32_t new_sequence)
{
    /*
     * Sequence numbers are monotonically increasing.
     *
     * Signed subtraction also handles wrap-around correctly
     * as long as the distance between two valid sequence
     * numbers is less than 2^31.
     */
    return ((int32_t)(new_sequence - current_sequence) > 0);
}

bool Metadata_ReadLatestRecord(
    uintptr_t sector_start,
    uintptr_t sector_end,
    firmware_metadata_record_t *latest_record)
{
    firmware_metadata_record_t record;
    bool found_valid_record = false;
    uintptr_t address;

    if ((latest_record == NULL) ||
        (sector_start >= sector_end))
    {
        return false;
    }

    /*
     * Metadata records are fixed-size and must fit completely
     * inside the metadata sector.
     */
    for (address = sector_start;
         (address + sizeof(firmware_metadata_record_t)) <= sector_end;
         address += sizeof(firmware_metadata_record_t))
    {
        /*
         * Read one record directly from Flash.
         */
        record =
            *(const firmware_metadata_record_t *)address;

        /*
         * Ignore erased, incomplete, or corrupted records.
         */
        if (!Metadata_ValidateRecord(&record))
        {
            continue;
        }

        /*
         * First valid record becomes the initial candidate.
         */
        if (!found_valid_record)
        {
            *latest_record = record;
            found_valid_record = true;
            continue;
        }

        /*
         * Keep the record with the newest sequence number.
         */
        if (Metadata_IsSequenceNewer(
                latest_record->sequence,
                record.sequence))
        {
            *latest_record = record;
        }
    }

    return found_valid_record;
}
