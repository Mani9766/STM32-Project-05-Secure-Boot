# Secure Boot – Security Requirements

## 1. Purpose

This document defines the security requirements for the STM32F407 Secure Boot and firmware validation implementation.

The requirements provide the basis for architecture, implementation, verification, and future security enhancements.

The current implementation focuses on firmware metadata validation, application memory validation, SHA-256 integrity verification, secure application handover, and candidate/active image handling.

Digital Signature Authentication and advanced rollback mechanisms are planned for future enhancement.

---

## 2. Bootloader Requirements

### SEC-BOOT-001 – Bootloader Startup

The Bootloader shall execute before the Application after MCU reset.

### SEC-BOOT-002 – Application Validation

The Bootloader shall validate the Application firmware before transferring execution to it.

### SEC-BOOT-003 – Invalid Firmware

The Bootloader shall not execute an Application that fails any mandatory validation check.

### SEC-BOOT-004 – Memory Boundary

The Bootloader shall verify that the Application image lies within its permitted Flash memory region.

### SEC-BOOT-005 – Firmware Size

The Bootloader shall verify that the Application image size does not exceed the configured Application memory region.

---

## 3. Metadata Requirements

### SEC-META-001 – Metadata Availability

The Bootloader shall read the firmware metadata from the configured metadata region before validating the corresponding firmware image.

### SEC-META-002 – Magic Number

The Bootloader shall validate the firmware metadata magic value.

### SEC-META-003 – Image Size

The Bootloader shall validate the firmware image size against the configured image region.

### SEC-META-004 – Firmware Version

The firmware metadata shall contain firmware version information.

### SEC-META-005 – Reference Digest

The firmware metadata shall contain the reference SHA-256 digest associated with the firmware image.

---

## 4. Integrity Requirements

### SEC-INT-001 – Firmware Hash

The Bootloader shall calculate a cryptographic hash over the defined firmware image data.

### SEC-INT-002 – SHA-256

The current implementation shall use SHA-256 for firmware integrity verification.

### SEC-INT-003 – Hash Verification

The Bootloader shall compare the calculated SHA-256 digest against the digest stored in firmware metadata.

### SEC-INT-004 – Hash Failure

If SHA-256 verification fails, the Bootloader shall reject the corresponding firmware image.

### SEC-INT-005 – Independent Verification

The Bootloader shall independently calculate the firmware digest from the firmware image stored in Flash.

---

## 5. Candidate / Active Image Requirements

### SEC-FOTA-001 – Image Separation

The candidate firmware and active firmware shall be stored in separate Flash regions.

### SEC-FOTA-002 – Candidate Metadata

The Bootloader shall read and validate candidate firmware metadata before processing the candidate image.

### SEC-FOTA-003 – Candidate Version Check

The Bootloader shall compare the candidate firmware version with the active firmware version.

### SEC-FOTA-004 – Candidate Integrity

A candidate firmware image shall pass SHA-256 integrity verification before execution.

### SEC-FOTA-005 – Candidate Rejection

The Bootloader shall reject a candidate image when its metadata, memory boundaries, version criteria, or SHA-256 verification fails.

### SEC-FOTA-006 – Active Image Preservation

The existing active firmware shall remain available while the candidate firmware is being evaluated.

### SEC-FOTA-007 – Active Fallback

When a candidate image is rejected, the Bootloader shall verify the active firmware and use it as the fallback image when its integrity verification succeeds.

---

## 6. Application Handover Requirements

### SEC-JMP-001 – Vector Table

The Bootloader shall configure the Application vector table before transferring execution.

### SEC-JMP-002 – Stack Pointer Validation

The Bootloader shall validate the Application initial stack pointer against the configured SRAM region.

### SEC-JMP-003 – Reset Handler Validation

The Bootloader shall validate the Application reset-handler address against the permitted application memory region.

### SEC-JMP-004 – Reset Handler Handover

The Bootloader shall transfer execution to the Application reset handler only after successful validation.

---

## 7. Failure Handling Requirements

### SEC-FAIL-001 – Verification Failure

The Bootloader shall not execute firmware when a mandatory validation or integrity check fails.

### SEC-FAIL-002 – Invalid Candidate

If candidate firmware validation fails, the Bootloader shall reject the candidate and continue with active-image validation.

### SEC-FAIL-003 – Invalid Active Image

If the active firmware also fails mandatory validation or SHA-256 verification, the Bootloader shall enter the defined fail-safe path.

### SEC-FAIL-004 – Invalid Application Handover

The Bootloader shall not transfer execution when the Application vector information is invalid.

---

## 8. Application Image Requirements

### SEC-IMG-001 – Image Identification

The Bootloader shall be able to determine whether valid firmware metadata is present for an image.

### SEC-IMG-002 – Image Size

The firmware image size shall be available to the Bootloader through the firmware metadata.

### SEC-IMG-003 – Image Version

The firmware version shall be available to the Bootloader through the firmware metadata.

### SEC-IMG-004 – Image Digest

The reference SHA-256 digest shall be available to the Bootloader through the firmware metadata.

---

## 9. Digital Signature Requirements – Future

The following requirements are planned for the Digital Signature Authentication enhancement.

### SEC-AUTH-001 – Digital Signature

The firmware image shall be associated with a digital signature generated by the authorized firmware signing process.

### SEC-AUTH-002 – Signature Verification

The Bootloader shall verify the firmware digital signature using a trusted public key.

### SEC-AUTH-003 – Invalid Signature

If digital signature verification fails, the Bootloader shall reject the firmware image.

### SEC-AUTH-004 – Private Key Protection

The private signing key shall not be stored in the embedded device firmware.

### SEC-AUTH-005 – Trust Anchor

The Bootloader shall use a defined trusted public key or equivalent trust anchor for signature verification.

---

## 10. Advanced FOTA / Rollback Requirements – Future

The following requirements are intentionally outside the current implementation scope.

### SEC-ROLL-001 – Persistent Update State

The system shall maintain firmware update state in non-volatile storage.

### SEC-ROLL-002 – Pending Image

A newly activated firmware image shall be identifiable as pending until successful application confirmation.

### SEC-ROLL-003 – Image Confirmation

The Application shall provide a mechanism to confirm successful startup and operation of the new firmware.

### SEC-ROLL-004 – Automatic Rollback

If the newly activated firmware fails to boot or remains unconfirmed, the Bootloader shall restore or boot the previous known-good firmware.

### SEC-ROLL-005 – Power-Loss Recovery

The update mechanism shall maintain a recoverable state across unexpected reset or power loss during firmware activation.

### SEC-ROLL-006 – Anti-Rollback

The system shall prevent installation or activation of firmware versions that violate the defined anti-rollback policy.

---

## 11. Requirement Traceability

Each requirement will be traced through the development lifecycle:

```text
Requirement
     |
     v
Architecture / Design
     |
     v
Implementation
     |
     v
Test Case
     |
     v
Test Result
     |
     v
Evidence
```

The current validation records contain test cases, expected behavior, actual results, status, and supporting evidence.

---

## 12. Verification Coverage

Current requirements are verified through:

* SHA-256 test vectors
* Firmware hashing from STM32 Flash
* Metadata validation
* Magic-number validation
* Image-size validation
* Memory-boundary validation
* Candidate version comparison
* Candidate SHA-256 verification
* Active SHA-256 verification
* Candidate rejection
* Active-image fallback
* Application vector validation
* Application handover testing
* Fail-safe testing

GDB checkpoints, LED indications, and screenshots are used as supporting verification evidence.

---

## 13. Future Security Requirements

Additional security requirements may be introduced during future enhancement phases, including:

* Digital Signature Authentication
* Authenticated firmware updates
* Anti-rollback protection
* Secure key provisioning
* Key rotation
* Debug-interface security
* Security-critical memory protection
* Enhanced recovery mechanisms
* Power-loss-resilient firmware activation
* Security hardening and fault handling

---

## 14. Implementation Status

**Status: Core Requirements Implemented and Verified**

The current implementation covers:

* Bootloader startup
* Firmware metadata validation
* Application memory-boundary validation
* Firmware image-size validation
* SHA-256 integrity verification
* Candidate/active image separation
* Candidate version checking
* Candidate-image verification
* Active-image verification
* Candidate rejection
* Active-image fallback
* Application vector validation
* Secure application handover
* Fail-safe handling

Digital Signature Authentication, persistent update state, pending/confirmed handling, candidate-to-active activation, rollback, anti-rollback protection, and advanced recovery remain future enhancements.
