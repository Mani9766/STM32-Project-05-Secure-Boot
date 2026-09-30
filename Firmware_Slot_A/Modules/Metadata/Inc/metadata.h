/*
 * metadata.h
 *
 *  Created on: Sep 12, 2026
 *      Author: Manisha Daigavhane
 */

#ifndef METADATA_H
#define METADATA_H

#include <stdint.h>
#include "sha256.h"

#define FIRMWARE_METADATA_MAGIC  0xDEADBEEFU

typedef enum
{
    CANDIDATE_STATE_EMPTY = 0U,
    CANDIDATE_STATE_PENDING_VALIDATION,
    CANDIDATE_STATE_VERIFIED,
    CANDIDATE_STATE_INVALID
} candidate_update_state_t;

typedef struct
{
    uint32_t magic;
    uint32_t image_size;
    uint32_t version;
    uint8_t  sha256[SHA256_DIGEST_SIZE];
    uint32_t update_state;
} firmware_metadata_t;

#endif /* INC_METADATA_H_ */
