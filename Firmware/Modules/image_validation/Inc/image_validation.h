/*
 * image_validation.h
 *
 *  Created on: Sep 17, 2026
 *      Author: Manisha Daigavhane
 */

#ifndef IMAGE_VALIDATION_H_
#define IMAGE_VALIDATION_H_

#include <stdint.h>
#include <stdbool.h>

void ImageValidation_CalculateSHA256(
    uintptr_t image_start,
    uint32_t image_size,
    uint8_t *digest);

bool ImageValidation_VerifySHA256(
    const uint8_t *calculated_digest,
    const uint8_t *reference_digest);

#endif /* IMAGE_VALIDATION_H_ */
