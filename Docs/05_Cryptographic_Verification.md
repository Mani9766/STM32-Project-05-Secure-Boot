# Cryptographic Verification

## 1. Overview

Cryptographic verification is a key part of the Secure Boot process.

The current implementation uses **SHA-256** to verify the integrity of application firmware before execution.

The firmware digest is stored in the corresponding firmware metadata and is independently recalculated by the Bootloader during the verification process.

Digital Signature Authentication is planned as a future enhancement to provide firmware authenticity verification.

---

## 2. Firmware Integrity

A cryptographic hash function converts the firmware image into a fixed-length digest.

The current implementation uses SHA-256, which produces a 256-bit (32-byte) digest.

```text
Application Firmware
        |
        v
     SHA-256
        |
        v
  Calculated Digest
```

If the firmware data changes, the calculated digest is expected to change.

The Bootloader uses this property to detect firmware corruption or modification.

---

## 3. SHA-256 Verification

The reference SHA-256 digest is stored in the firmware metadata.

During boot, the Bootloader independently calculates the digest of the firmware image stored in STM32 Flash.

```text
             Firmware Image
                    |
                    v
                SHA-256
                    |
                    v
            Calculated Digest
                    |
             Compare Digests
               /       \
            Match     Mismatch
              |           |
              v           v
        Image Accepted   Reject
```

A matching digest indicates that the firmware data corresponds to the reference digest stored in metadata.

A mismatch causes the firmware image to be rejected.

The SHA-256 calculation is performed independently by the Bootloader rather than trusting a digest supplied by the application.

---

## 4. SHA-256 Implementation

The project contains a dedicated SHA-256 implementation used by the Bootloader.

The implementation has been validated using standard test vectors and additional boundary-oriented test cases.

Current validation includes:

* Empty input
* `"abc"` test vector
* 55-byte input
* 56-byte input
* Multi-block input
* Firmware data stored in STM32 Flash
* Active firmware SHA-256 verification
* Candidate firmware SHA-256 verification
* SHA-256 mismatch detection

These tests provide confidence in both the standalone SHA-256 implementation and its integration with firmware-image verification.

---

## 5. Digest Storage

The calculated firmware digest is associated with the corresponding firmware metadata.

The current metadata structure contains a 32-byte SHA-256 digest.

```text
Firmware Metadata
┌────────────────────────────┐
│ Magic Number               │
├────────────────────────────┤
│ Image Size                 │
├────────────────────────────┤
│ Firmware Version           │
├────────────────────────────┤
│ SHA-256 Digest (32 bytes)  │
└────────────────────────────┘
```

Separate metadata is maintained for the active and candidate firmware images.

---

## 6. Active and Candidate Image Verification

The same SHA-256 verification mechanism is applied independently to both firmware images.

### Active Image

The Bootloader calculates the SHA-256 digest over the configured active firmware image and compares it with the active metadata digest.

A successful match allows the active application to execute.

### Candidate Image

For a valid and newer candidate firmware, the Bootloader calculates the SHA-256 digest over the candidate firmware image and compares it with the candidate metadata digest.

A successful match allows the candidate application to execute from its candidate Flash location.

A mismatch causes the candidate image to be rejected, after which the Bootloader can verify and use the active image as the fallback.

---

## 7. Hash vs Digital Signature

Hashing and digital signatures provide different security properties.

| Mechanism         | Primary Purpose                            |
| ----------------- | ------------------------------------------ |
| SHA-256           | Detect changes to firmware data            |
| Digital Signature | Verify firmware authenticity and integrity |
| Private Key       | Create a digital signature                 |
| Public Key        | Verify a digital signature                 |

SHA-256 alone does not establish who created the firmware.

Digital Signature Authentication will therefore be added as a future security enhancement.

---

## 8. Planned Digital Signature Verification

The planned DSA implementation will use a private/public key model.

On the firmware-signing side:

```text
Firmware
   |
   v
SHA-256
   |
   v
Firmware Digest
   |
   v
Private Key
   |
   v
Digital Signature
```

On the embedded device:

```text
Firmware Image
      |
      +------------------+
      |                  |
      v                  v
    SHA-256       Digital Signature
      |                  |
      v                  |
Calculated Digest        |
      |                  |
      +--------+---------+
               |
               v
          Trusted Public Key
               |
               v
      Signature Verification
               |
          +----+----+
          |         |
        VALID     INVALID
          |         |
          v         v
       Accept     Reject
```

The final signature algorithm, signature format, public-key representation, and verification library will be selected during the DSA enhancement phase.

---

## 9. Trust Model

The planned authentication architecture separates signing keys from the embedded device.

```text
Private Signing Key
        |
        | Signing
        v
Firmware Signing Process
        |
        v
Signed Firmware
        |
        v
Embedded Device
        |
        v
Bootloader
        |
        | Uses trusted public key
        v
Signature Verification
```

The private signing key must remain outside the embedded firmware and must be protected by the signing environment.

The Bootloader will use the trusted public key or another defined trust anchor for signature verification.

---

## 10. Verification Failure

For the current SHA-256 integrity verification:

```text
SHA-256 Verification
        |
        v
      FAILED
        |
        v
Do NOT execute image
        |
        v
Reject Image
        |
        v
Active-Image Fallback / Fail-Safe
```

For the future DSA implementation:

```text
Digital Signature Verification
        |
        v
      FAILED
        |
        v
Do NOT execute firmware
```

The detailed recovery and rollback behavior will be defined as part of the future FOTA enhancement.

---

## 11. Future Cryptographic Enhancements

The planned cryptographic enhancements include:

1. Digital Signature Authentication
2. Host-side firmware signing
3. Public-key integration into the Bootloader
4. Signature verification testing
5. Modified-firmware testing
6. Invalid-signature testing
7. Incorrect-public-key testing
8. Verification performance measurement
9. Verification memory-footprint measurement
10. Key-management considerations

---

## 12. Implementation Status

**Status: SHA-256 Implemented and Verified**

The current implementation includes:

* SHA-256 algorithm implementation
* Standard SHA-256 test-vector validation
* Boundary and multi-block test cases
* SHA-256 calculation over firmware stored in STM32 Flash
* Active-image integrity verification
* Candidate-image integrity verification
* Stored-digest comparison
* SHA-256 mismatch detection
* Firmware rejection on integrity failure

**Digital Signature Authentication: Planned**

Digital signature verification, trusted public-key handling, firmware signing, and related key-management mechanisms will be implemented as the next cryptographic security enhancement.
