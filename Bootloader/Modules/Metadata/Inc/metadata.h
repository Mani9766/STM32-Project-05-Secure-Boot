/*
 * metadata.h
 *
 *  Created on: Sep 12, 2026
 *      Author: Manisha Daigavhane
 */

#ifndef METADATA_H
#define METADATA_H

#include <stdbool.h>
#include <stdint.h>

#include "sha256.h"

#define FIRMWARE_METADATA_MAGIC    0xDEADBEEFU
#define METADATA_RECORD_MAGIC      0x4D455441U
#define METADATA_COMMIT_MARKER     0xA5A55A5AU

typedef enum
{
    FIRMWARE_STATE_EMPTY = 0U,
    FIRMWARE_STATE_PENDING_VALIDATION,
    FIRMWARE_STATE_VALIDATED,
    FIRMWARE_STATE_BOOT_PENDING,
    FIRMWARE_STATE_CONFIRMED,
    FIRMWARE_STATE_INVALID,
    FIRMWARE_STATE_ROLLBACK

} firmware_update_state_t;

typedef struct
{
    uint32_t magic;
    uint32_t image_size;
    uint32_t version;
    uint8_t  sha256[SHA256_DIGEST_SIZE];
    uint32_t update_state;

} firmware_metadata_t;

typedef struct
{
    uint32_t record_magic;
    uint32_t sequence;
    firmware_metadata_t metadata;
    uint32_t metadata_crc;
    uint32_t commit_marker;

} firmware_metadata_record_t;

bool Metadata_Validate(
    const firmware_metadata_t *metadata,
    uintptr_t image_start,
    uintptr_t image_region_end);

uint32_t Metadata_CalculateRecordCRC(
    const firmware_metadata_record_t *record);

bool Metadata_ValidateRecord(
    const firmware_metadata_record_t *record);

bool Metadata_IsSequenceNewer(
    uint32_t current_sequence,
    uint32_t new_sequence);

bool Metadata_ReadLatestRecord(
    uintptr_t sector_start,
    uintptr_t sector_end,
    firmware_metadata_record_t *latest_record);

#endif /* METADATA_H */
