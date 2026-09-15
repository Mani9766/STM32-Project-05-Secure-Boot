/*
 * sha256.h
 *
 *  Created on: Sep 8, 2026
 *      Author: Manisha Daigavhane
 */

#ifndef SHA256_H_
#define SHA256_H_

#include <stdint.h>
#include <stddef.h>

#define SHA256_BLOCK_SIZE   64U
#define SHA256_DIGEST_SIZE  32U

typedef struct
{
    uint32_t state[8];                              //the eight 32-bit SHA-256 working hash values.
    uint64_t bit_length;                            //total length of the input processed so far
    uint8_t  buffer[SHA256_BLOCK_SIZE];             //temporarily stores data until we have a complete 512-bit block.
    size_t   buffer_length;
} SHA256_Context;

void SHA256_Init(SHA256_Context *ctx);

void SHA256_Update(SHA256_Context *ctx,
                   const uint8_t *data,
                   size_t length);

void SHA256_Final(SHA256_Context *ctx,
                  uint8_t digest[SHA256_DIGEST_SIZE]);

#endif /* SHA256_H_ */
