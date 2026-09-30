/*
 * sha256.c
 *
 *  Created on: Sep 8, 2026
 *      Author: Manisha Daigavhane
 */

#include "sha256.h"

#define ROTR32(x, n) (((x) >> (n)) | ((x) << (32U - (n))))

#define CH(x, y, z)  (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))

#define BIG_SIGMA0(x) \
    (ROTR32((x), 2U) ^ ROTR32((x), 13U) ^ ROTR32((x), 22U))

#define BIG_SIGMA1(x) \
    (ROTR32((x), 6U) ^ ROTR32((x), 11U) ^ ROTR32((x), 25U))

#define SMALL_SIGMA0(x) \
    (ROTR32((x), 7U) ^ ROTR32((x), 18U) ^ ((x) >> 3U))

#define SMALL_SIGMA1(x) \
    (ROTR32((x), 17U) ^ ROTR32((x), 19U) ^ ((x) >> 10U))

static const uint32_t K[64] =
{
    0x428A2F98U, 0x71374491U, 0xB5C0FBCFU, 0xE9B5DBA5U,
    0x3956C25BU, 0x59F111F1U, 0x923F82A4U, 0xAB1C5ED5U,
    0xD807AA98U, 0x12835B01U, 0x243185BEU, 0x550C7DC3U,
    0x72BE5D74U, 0x80DEB1FEU, 0x9BDC06A7U, 0xC19BF174U,
    0xE49B69C1U, 0xEFBE4786U, 0x0FC19DC6U, 0x240CA1CCU,
    0x2DE92C6FU, 0x4A7484AAU, 0x5CB0A9DCU, 0x76F988DAU,
    0x983E5152U, 0xA831C66DU, 0xB00327C8U, 0xBF597FC7U,
    0xC6E00BF3U, 0xD5A79147U, 0x06CA6351U, 0x14292967U,
    0x27B70A85U, 0x2E1B2138U, 0x4D2C6DFCU, 0x53380D13U,
    0x650A7354U, 0x766A0ABBU, 0x81C2C92EU, 0x92722C85U,
    0xA2BFE8A1U, 0xA81A664BU, 0xC24B8B70U, 0xC76C51A3U,
    0xD192E819U, 0xD6990624U, 0xF40E3585U, 0x106AA070U,
    0x19A4C116U, 0x1E376C08U, 0x2748774CU, 0x34B0BCB5U,
    0x391C0CB3U, 0x4ED8AA4AU, 0x5B9CCA4FU, 0x682E6FF3U,
    0x748F82EEU, 0x78A5636FU, 0x84C87814U, 0x8CC70208U,
    0x90BEFFFAU, 0xA4506CEBU, 0xBEF9A3F7U, 0xC67178F2U
};

/**
 * @brief   Processes a single 512-bit message block.
 *
 * @param   ctx     Pointer to the SHA-256 context.
 * @param   block   Pointer to a 64-byte message block.
 *
 * @return  void
 */
static void SHA256_Transform(SHA256_Context *ctx, const uint8_t block[64])
{
    uint32_t W[64];

    uint32_t a, b, c, d;
    uint32_t e, f, g, h;

    uint32_t T1, T2;

    /* Prepare message schedule W[0..63] */

    for (uint32_t i = 0U; i < 16U; i++)
    {
        W[i] = ((uint32_t)block[i * 4U] << 24U) |
               ((uint32_t)block[i * 4U + 1U] << 16U) |
               ((uint32_t)block[i * 4U + 2U] << 8U) |
               ((uint32_t)block[i * 4U + 3U]);
    }

    for (uint32_t i = 16U; i < 64U; i++)
    {
        W[i] = SMALL_SIGMA1(W[i - 2U]) +
               W[i - 7U] +
               SMALL_SIGMA0(W[i - 15U]) +
               W[i - 16U];
    }

    /* Initialize working variables */

    a = ctx->state[0];
    b = ctx->state[1];
    c = ctx->state[2];
    d = ctx->state[3];

    e = ctx->state[4];
    f = ctx->state[5];
    g = ctx->state[6];
    h = ctx->state[7];

    /* 64 SHA-256 rounds */

    for (uint32_t i = 0U; i < 64U; i++)
    {
        T1 = h +
             BIG_SIGMA1(e) +
             CH(e, f, g) +
             K[i] +
             W[i];

        T2 = BIG_SIGMA0(a) +
             MAJ(a, b, c);

        h = g;
        g = f;
        f = e;
        e = d + T1;
        d = c;
        c = b;
        b = a;
        a = T1 + T2;
    }

    /* Add the result back into the hash state */

    ctx->state[0] += a;
    ctx->state[1] += b;
    ctx->state[2] += c;
    ctx->state[3] += d;
    ctx->state[4] += e;
    ctx->state[5] += f;
    ctx->state[6] += g;
    ctx->state[7] += h;
}

/**
 * @brief   Initializes the SHA-256 context.
 *
 * @param   ctx     Pointer to the SHA-256 context.
 *
 * @return  void
 */
void SHA256_Init(SHA256_Context *ctx)
{
    ctx->state[0] = 0x6A09E667U;
    ctx->state[1] = 0xBB67AE85U;
    ctx->state[2] = 0x3C6EF372U;
    ctx->state[3] = 0xA54FF53AU;
    ctx->state[4] = 0x510E527FU;
    ctx->state[5] = 0x9B05688CU;
    ctx->state[6] = 0x1F83D9ABU;
    ctx->state[7] = 0x5BE0CD19U;

    ctx->bit_length = 0U;
    ctx->buffer_length = 0U;
}

/**
 * @brief   Updates the SHA-256 hash with input data.
 *
 * @param   ctx      Pointer to the SHA-256 context.
 * @param   data     Pointer to the input data.
 * @param   length   Length of input data in bytes.
 *
 * @return  void
 */
void SHA256_Update(SHA256_Context *ctx,
                   const uint8_t *data,
                   size_t length)
{
    size_t i = 0U;

    ctx->bit_length += (uint64_t)length * 8U;

    while (i < length)
    {
        ctx->buffer[ctx->buffer_length] = data[i];
        ctx->buffer_length++;
        i++;

        /* Process a complete 512-bit block */
        if (ctx->buffer_length == SHA256_BLOCK_SIZE)
        {
            SHA256_Transform(ctx, ctx->buffer);
            ctx->buffer_length = 0U;
        }
    }
}

void SHA256_Padding(SHA256_Context *ctx)
{
    uint64_t bit_length = ctx->bit_length;

    /* Append 1 bit */
    ctx->buffer[ctx->buffer_length++] = 0x80U;

    /* If length field doesn't fit, complete this block */
    if (ctx->buffer_length > 56U)
    {
        while (ctx->buffer_length < 64U)
        {
            ctx->buffer[ctx->buffer_length++] = 0x00U;
        }

        SHA256_Transform(ctx, ctx->buffer);

        ctx->buffer_length = 0U;
    }

    /* Fill with zeros until 56 bytes */
    while (ctx->buffer_length < 56U)
    {
        ctx->buffer[ctx->buffer_length++] = 0x00U;
    }

    /* Append original message length in bits */
    ctx->buffer[56] = (uint8_t)(bit_length >> 56);
    ctx->buffer[57] = (uint8_t)(bit_length >> 48);
    ctx->buffer[58] = (uint8_t)(bit_length >> 40);
    ctx->buffer[59] = (uint8_t)(bit_length >> 32);
    ctx->buffer[60] = (uint8_t)(bit_length >> 24);
    ctx->buffer[61] = (uint8_t)(bit_length >> 16);
    ctx->buffer[62] = (uint8_t)(bit_length >> 8);
    ctx->buffer[63] = (uint8_t)(bit_length);
}

/**
 * @brief   Finalizes the SHA-256 calculation and generates the digest.
 *
 * @param   ctx      Pointer to the SHA-256 context.
 * @param   digest   Buffer to store the 32-byte SHA-256 digest.
 *
 * @return  void
 */
void SHA256_Final(SHA256_Context *ctx,
                  uint8_t digest[SHA256_DIGEST_SIZE])
{
    size_t i;

    SHA256_Padding(ctx);

    /*
     * Process the final block.
     */
    SHA256_Transform(ctx, ctx->buffer);

    /*
     * Convert the final hash state to a 32-byte digest.
     */
    for (i = 0U; i < 8U; i++)
    {
        digest[(i * 4U)]     = (uint8_t)(ctx->state[i] >> 24U);
        digest[(i * 4U) + 1U] = (uint8_t)(ctx->state[i] >> 16U);
        digest[(i * 4U) + 2U] = (uint8_t)(ctx->state[i] >> 8U);
        digest[(i * 4U) + 3U] = (uint8_t)(ctx->state[i]);
    }
}

