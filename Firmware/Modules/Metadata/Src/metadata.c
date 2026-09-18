/*
 * metadata.c
 *
 *  Created on: Sep 12, 2026
 *      Author: Manisha Daigavhane
 */

#include "metadata.h"

bool Metadata_Validate(
    const firmware_metadata_t *metadata,
    uintptr_t image_start,
    uintptr_t image_region_end)
{
    uintptr_t image_end;

    /*
     * Validate metadata pointer before accessing its fields.
     */
    if (metadata == NULL)
    {
        return false;
    }

    /*
     * Verify that the metadata belongs to a valid firmware image.
     */
    if (metadata->magic != FIRMWARE_METADATA_MAGIC)
    {
        return false;
    }

    /*
     * An image with zero size is not valid.
     */
    if (metadata->image_size == 0U)
    {
        return false;
    }

    /*
     * Calculate the exclusive end address of the image.
     */
    image_end =
        image_start + (uintptr_t)metadata->image_size;

    /*
     * Detect address overflow.
     */
    if (image_end < image_start)
    {
        return false;
    }

    /*
     * Verify that the complete image remains inside
     * its allocated Flash region.
     *
     * image_region_end is an exclusive end address.
     */
    if (image_end > image_region_end)
    {
        return false;
    }

    return true;
}

bool Metadata_IsCandidateNewer(
    const firmware_metadata_t *active_metadata,
    const firmware_metadata_t *candidate_metadata)
{
    /*
     * Both metadata pointers must be valid before
     * accessing their version fields.
     */
    if ((active_metadata == NULL) ||
        (candidate_metadata == NULL))
    {
        return false;
    }

    /*
     * Candidate is considered an update only when
     * its version is greater than the active version.
     */
    return (candidate_metadata->version >
            active_metadata->version);
}
