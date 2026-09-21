# Failure Handling

## 1. Overview

A Secure Bootloader must define what happens when firmware cannot be validated or safely executed.

The fundamental rule of the current implementation is:

> **If firmware fails the required validation or integrity checks, the Bootloader must not execute that firmware image.**

The current Bootloader implements validation and fail-safe handling for metadata, image boundaries, SHA-256 integrity, and application handover checks.

---

## 2. Current Failure Conditions

The Bootloader can detect failures such as:

* Firmware metadata read failure
* Invalid firmware metadata
* Invalid magic number
* Invalid firmware image size
* Firmware image outside the allowed memory region
* Candidate firmware version not eligible for processing
* Candidate SHA-256 verification failure
* Active SHA-256 verification failure
* Invalid application reset-handler address
* Corrupted firmware image

Digital signature verification is not part of the current implementation and will be added during the future DSA enhancement.

---

## 3. Failure Decision

The current decision flow is:

```text id="0x1b4p"
              Verify Firmware
                    |
               +----+----+
               |         |
            VALID      INVALID
               |           |
               v           v
        Prepare Handover  Reject Image
               |           |
               v           v
       Jump to Application  |
                             v
                    Failure / Fallback
```

For candidate firmware, a validation failure does not overwrite or destroy the active firmware. The Bootloader proceeds to verify the active image and uses it as the fallback when its integrity check succeeds.

---

## 4. Metadata Validation Failure

The Bootloader reads firmware metadata before processing an image.

Invalid metadata can include:

* Invalid magic number
* Invalid image size
* Image boundaries outside the configured region
* Invalid metadata contents

When active-image metadata is invalid, the Bootloader enters the fail-safe path.

When candidate metadata is invalid, the candidate is rejected and the Bootloader continues with active-image validation.

```text id="5n1v31"
Read Metadata
      |
      v
Validate Metadata
      |
   +--+--+
   |     |
 VALID  INVALID
   |       |
   v       v
Continue  Reject Image
```

---

## 5. Firmware Integrity Verification Failure

The Bootloader independently calculates the SHA-256 digest of the firmware image and compares it with the digest stored in metadata.

```text id="e6w1yr"
Firmware Image
      |
      v
Calculate SHA-256
      |
      v
Compare with Metadata Digest
      |
   +--+--+
   |     |
 MATCH  MISMATCH
   |       |
   v       v
Accept   Reject
```

A SHA-256 mismatch may indicate:

* Flash corruption
* Incomplete or invalid firmware data
* Accidental modification
* Unauthorized modification

The affected firmware image is not executed.

For the candidate image, the Bootloader rejects the candidate and attempts the active-image fallback.

---

## 6. Active-Image Fallback

The dual-image architecture allows the active firmware to remain available while the candidate image is being evaluated.

When a candidate image fails validation or SHA-256 verification:

```text id="b0z9x9"
Candidate Validation
        |
        v
      FAILED
        |
        v
Reject Candidate
        |
        v
Verify Active Image
        |
   +----+----+
   |         |
 MATCH     MISMATCH
   |           |
   v           v
Boot Active  Fail-Safe
```

The active image is independently verified before execution.

This provides the current FOTA fallback behavior without modifying the active firmware during candidate evaluation.

---

## 7. Invalid Application Handover

Before transferring control to an application, the Bootloader validates the application vector information.

The current implementation checks the application's initial stack pointer and reset-handler address against the configured memory ranges.

An invalid reset-handler address causes the application handover to be rejected.

```text id="7om5b7"
Application Vector Table
          |
          v
Validate Stack Pointer
          |
          v
Validate Reset Handler
          |
      +---+---+
      |       |
    VALID   INVALID
      |         |
      v         v
 Continue     Reject
 Handover
```

The Bootloader must not jump to an invalid application address.

---

## 8. Active Firmware Failure

If the candidate image is rejected and the active firmware also fails its SHA-256 integrity verification, the Bootloader cannot safely execute either image.

The current implementation then enters the Bootloader fail-safe path.

```text id="2d7o1z"
Candidate Rejected
       |
       v
Verify Active Image
       |
       v
SHA-256 Mismatch
       |
       v
Do NOT execute Active Image
       |
       v
Bootloader Fail-Safe
```

---

## 9. Failure / Fail-Safe Handling

The current Bootloader provides a defined fail-safe path for unrecoverable firmware validation failures.

```text id="q3zi51"
Firmware Cannot Be Validated
            |
            v
     Do NOT execute image
            |
            v
     Bootloader Fail-Safe
```

The current implementation uses Bootloader status indication during development to identify the failure condition.

A full firmware recovery/update mechanism is not part of the current implementation.

---

## 10. Digital Signature Failure

Digital Signature Authentication is planned as a future enhancement.

Once implemented, an invalid digital signature will result in rejection of the firmware image:

```text id="8owf88"
Digital Signature Verification
            |
        +---+---+
        |       |
      VALID   INVALID
        |         |
        v         v
     Continue   Reject
                 Image
```

Potential causes include:

* Invalid signature
* Modified firmware
* Incorrect signing key
* Untrusted firmware source
* Corrupted signature data

Detailed DSA failure handling will be defined during the DSA implementation phase.

---

## 11. Recovery Mechanism

A complete firmware recovery mechanism is outside the current project scope.

Future recovery functionality may include communication-based firmware replacement using interfaces such as:

* UART
* CAN
* USB
* Other supported update interfaces

The exact recovery mechanism will be selected according to the target-system requirements.

---

## 12. Logging and Diagnostics

During development and validation, diagnostic information is provided through debugger checkpoints and status indications.

Examples include:

* Invalid metadata
* Invalid firmware size
* Candidate version rejection
* Candidate SHA-256 mismatch
* Active SHA-256 mismatch
* Invalid application reset-handler
* Fail-safe entry

Production implementations may use a controlled diagnostic mechanism appropriate to the security requirements.

---

## 13. Current Verification

Failure handling has been validated using intentionally invalid conditions and firmware modifications.

Current validation includes:

* Active metadata read failure
* Invalid active metadata
* Invalid candidate metadata
* Candidate version rejection
* Candidate SHA-256 mismatch
* Active SHA-256 mismatch
* Invalid application memory boundaries
* Invalid application reset-handler address
* Candidate rejection followed by active-image fallback
* Fail-safe execution when the active image cannot be validated

Validation evidence is maintained using GDB checkpoints, LED status indication, and screenshots.

---

## 14. Future Enhancements

The following failure-handling capabilities are planned for future enhancement:

* Digital Signature Authentication failure handling
* Candidate-to-active image activation
* Persistent update state
* Pending/confirmed image handling
* Automatic rollback
* Power-loss-resilient update recovery
* Communication-based firmware recovery
* Anti-rollback protection
* Additional production-oriented security hardening

---

## 15. Implementation Status

**Status: Implemented and Verified**

The current implementation provides firmware validation and failure handling for metadata, memory boundaries, SHA-256 integrity, application handover, candidate rejection, active-image fallback, and unrecoverable fail-safe conditions.

Digital signature failure handling, persistent rollback, and full firmware recovery mechanisms remain future enhancements.
