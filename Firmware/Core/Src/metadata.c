/*
 * metadata.c
 *
 *  Created on: Sep 12, 2026
 *      Author: Manisha Daigavhane
 */

#include "metadata.h"

__attribute__((section(".metadata"), used))
const firmware_metadata_t firmware_metadata =
{
    .magic      = 0xDEADBEEFU,
    .image_size = 0U,
    .sha256     = {0},
    .version    = 1U
};
