/*
 * metadata.h
 *
 *  Created on: Sep 12, 2026
 *      Author: Manisha Daigavhane
 */

#ifndef METADATA_H
#define METADATA_H

#include <stdint.h>

#define FIRMWARE_METADATA_MAGIC  0xDEADBEEFU

typedef struct
{
    uint32_t magic;
    uint32_t image_size;
    uint8_t  sha256[32];
    uint32_t version;
} firmware_metadata_t;

#endif /* INC_METADATA_H_ */
