/*
 * image_validation.c
 *
 *  Created on: Sep 17, 2026
 *      Author: Manisha Daigavhane
 */


#include "image_validation.h"
#include "sha256.h"
#include <string.h>

void ImageValidation_CalculateSHA256(
    uintptr_t image_start,
    uint32_t image_size,
    uint8_t *digest)
{
    SHA256_Context ctx;

    if (digest == NULL)
    {
        return;
    }

    SHA256_Init(&ctx);

    SHA256_Update(
        &ctx,
        (const uint8_t *)image_start,
        image_size);

    SHA256_Final(&ctx, digest);
}

bool ImageValidation_VerifySHA256(
    const uint8_t *calculated_digest,
    const uint8_t *reference_digest)
{
    if ((calculated_digest == NULL) ||
        (reference_digest == NULL))
    {
        return false;
    }

    return (memcmp(calculated_digest,
                   reference_digest,
                   SHA256_DIGEST_SIZE) == 0);
}
