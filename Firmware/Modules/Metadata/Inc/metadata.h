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

#define FIRMWARE_METADATA_MAGIC  0xDEADBEEFU

typedef enum
{
    CANDIDATE_STATE_EMPTY = 0U,
    CANDIDATE_STATE_PENDING_VALIDATION,
    CANDIDATE_STATE_VALIDATED,
    CANDIDATE_STATE_BOOT_PENDING,
    CANDIDATE_STATE_CONFIRMED,
    CANDIDATE_STATE_INVALID,
    CANDIDATE_STATE_ROLLBACK
} candidate_update_state_t;

typedef struct
{
    uint32_t magic;
    uint32_t image_size;
    uint32_t version;
    uint8_t  sha256[SHA256_DIGEST_SIZE];
    uint32_t update_state;
} firmware_metadata_t;

bool Metadata_Validate(
    const firmware_metadata_t *metadata,
    uintptr_t image_start,
    uintptr_t image_region_end);

bool Metadata_IsCandidateNewer(
    const firmware_metadata_t *active_metadata,
    const firmware_metadata_t *candidate_metadata);

#endif /* INC_METADATA_H_ */
