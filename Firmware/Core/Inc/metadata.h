/*
 * metadata.h
 *
 *  Created on: Sep 12, 2026
 *      Author: Manisha Daigavhane
 */

#ifndef INC_METADATA_H_
#define INC_METADATA_H_

#include <stdint.h>

typedef struct
{
    uint32_t magic;
    uint32_t image_size;
    uint8_t  sha256[32];
    uint32_t version;
} firmware_metadata_t;

extern const firmware_metadata_t firmware_metadata;


#endif /* INC_METADATA_H_ */
