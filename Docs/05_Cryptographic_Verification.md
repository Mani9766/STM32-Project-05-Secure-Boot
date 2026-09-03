# Cryptographic Verification

## 1. Overview

Cryptographic verification is a key part of the Secure Boot process.

The Bootloader uses cryptographic mechanisms to determine whether the Application firmware is:

- Intact
- Authentic
- Approved for execution

The project will use a combination of hashing and digital signatures.

## 2. Firmware Integrity

A cryptographic hash function converts the firmware image into a fixed-length digest.

```text
Application Firmware
        |
        v
   Hash Function
        |
        v
   Hash / Digest
```
If even a small portion of the firmware changes, the resulting hash should change significantly.

The Bootloader can therefore detect accidental corruption or unauthorized modification of the firmware.

## 3. Hash Verification

The expected hash is associated with the firmware image.

During boot:
```text
             Firmware Image
                    |
                    v
              Hash Function
                    |
                    v
            Calculated Hash
                    |
             Compare Hashes
               /       \
            Match     Mismatch
              |           |
              v           v
          Continue      Reject
```
A hash match demonstrates that the verified firmware data corresponds to the expected digest.

However, a hash by itself does not establish who generated the firmware.

## 4. Digital Signature

Digital signatures are used to establish firmware authenticity.

The basic model uses a private key and a public key.
```text
                 DEVELOPMENT SIDE

Firmware
   |
   v
Hash Function
   |
   v
Hash
   |
   v
Private Key
   |
   v
Digital Signature
```
The signature is distributed with the firmware image.

The Bootloader contains or has access to the trusted public key.

## 5. Signature Verification

During boot:
```text
Firmware Image
      |
      +------------------+
      |                  |
      v                  v
 Hash Function      Digital Signature
      |                  |
      v                  |
Calculated Hash          |
      |                  |
      +--------+---------+
               |
               v
        Public Key
               |
               v
      Signature Verification
               |
          +----+----+
          |         |
       VALID      INVALID
          |         |
          v         v
       Continue    Reject
```
A valid signature provides evidence that the firmware was signed using the corresponding trusted private key and that the signed data has not been altered.

## 6. Trust Model

The private signing key must remain outside the embedded device and must be protected by the firmware signing process.

The Bootloader should contain only the trusted public key or an equivalent trust anchor required for verification.
```text
Private Key
    |
    | Used for signing
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
## 7. Hash vs Digital Signature

Hashing and digital signatures serve different purposes.
| Mechanism         | Primary Purpose                   |
| ----------------- | --------------------------------- |
| Hash              | Detect data changes               |
| Digital Signature | Verify authenticity and integrity |
| Private Key       | Create signature                  |
| Public Key        | Verify signature                  |

The Secure Boot implementation will use these concepts together rather than relying on a hash alone.

## 8. Cryptographic Algorithm

The exact algorithms will be selected during implementation based on:

- STM32 device capabilities
- Security requirements
- Available cryptographic libraries
- Performance and memory constraints
- Project learning objectives
## 9. Key Management Considerations

The project will also consider:

- Public key storage
- Private key protection
- Key provisioning
- Key replacement
- Trust anchor protection
- Compromised-key handling

Private signing keys must never be embedded in the production firmware.

## 10. Verification Failure

If cryptographic verification fails:
```text
Verification Failed
        |
        v
Do NOT execute Application
        |
        v
Enter Failure / Recovery Path
```
The exact recovery behavior will be defined separately.

## 11. Implementation Plan

Cryptographic verification will be implemented incrementally:

1. Implement firmware hashing.
2. Verify calculated digest.
3. Introduce digital signatures.
4. Generate a test signing key pair.
5. Implement signature generation on the host side.
6. Implement signature verification in the Bootloader.
7. Test modified firmware.
8. Test invalid signatures.
9. Test incorrect keys.
10. Measure verification performance and memory usage.
## 12. Implementation Status

Status: Design Phase

Cryptographic algorithms, libraries, key storage, and the final firmware image format will be finalized during implementation.
