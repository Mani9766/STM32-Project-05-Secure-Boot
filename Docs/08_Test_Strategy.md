# Secure Boot – Test Strategy

## 1. Purpose

This document defines the verification and testing strategy for the STM32F407 Secure Boot and FOTA-oriented firmware validation implementation.

The objective is to demonstrate that the Bootloader:

* Validates firmware metadata.
* Validates application memory boundaries.
* Verifies firmware integrity using SHA-256.
* Rejects invalid or corrupted firmware.
* Correctly handles candidate and active firmware images.
* Transfers control only to a validated application.
* Enters the defined fallback or fail-safe path when validation fails.

Digital Signature Authentication and persistent rollback mechanisms are planned for future enhancement and will be tested separately.

---

## 2. Verification Approach

Verification is performed incrementally using multiple test levels and techniques appropriate to the implemented functionality.

Current verification includes:

* SHA-256 algorithm testing
* Functional testing
* Boot-flow integration testing
* Positive testing
* Negative testing
* Boundary testing
* Firmware corruption testing
* Debugger-based fault injection
* Candidate/active image validation
* Application handover testing
* Regression testing

The current project validation is performed primarily on the STM32F407 target using STM32CubeIDE, ST-LINK/SWD, GDB, LED status indication, and screenshots as test evidence.

---

## 3. Bootloader Tests

### TEST-BOOT-001 – Bootloader Startup

**Objective:**
Verify that the Bootloader executes after MCU reset.

**Expected Result:**
Bootloader starts successfully and enters the firmware validation flow.

**Status:** Verified

---

### TEST-BOOT-002 – Valid Application

**Objective:**
Verify that a valid application image passes the required validation checks and is executed.

**Expected Result:**
Bootloader validates the firmware and transfers control to the application.

**Status:** Verified

---

### TEST-BOOT-003 – Invalid Active Firmware

**Objective:**
Verify that the Bootloader does not execute an active firmware image whose integrity verification fails.

**Expected Result:**
Active firmware is rejected and the Bootloader enters the defined fail-safe path.

**Status:** Verified

---

## 4. Metadata Validation Tests

### TEST-META-001 – Valid Metadata

**Objective:**
Verify that valid firmware metadata is accepted.

**Expected Result:**
Bootloader continues with firmware validation.

**Status:** Verified

---

### TEST-META-002 – Invalid Magic Number

**Objective:**
Verify that firmware metadata with an invalid magic value is rejected.

**Expected Result:**
Corresponding firmware image is rejected.

**Status:** Verified

---

### TEST-META-003 – Invalid Firmware Size

**Objective:**
Verify that an invalid firmware image size is rejected.

**Expected Result:**
Bootloader does not process or execute the invalid image.

**Status:** Verified

---

### TEST-META-004 – Metadata Read Failure

**Objective:**
Verify the failure path when firmware metadata cannot be read successfully.

**Expected Result:**
Invalid metadata is not used for firmware execution.

**Status:** Verified

---

## 5. SHA-256 Integrity Tests

### TEST-INT-001 – Standard SHA-256 Test Vectors

**Objective:**
Verify the SHA-256 implementation against standard test vectors.

**Test coverage includes:**

* Empty input
* `"abc"`
* 55-byte input
* 56-byte input
* Multi-block input

**Expected Result:**
Calculated digests match the expected SHA-256 values.

**Status:** Verified

---

### TEST-INT-002 – Active Firmware SHA-256 Verification

**Objective:**
Verify the SHA-256 digest of firmware stored in STM32 Flash.

**Expected Result:**
Calculated active-image digest matches the digest stored in active metadata.

**Status:** Verified

---

### TEST-INT-003 – Candidate Firmware SHA-256 Verification

**Objective:**
Verify the SHA-256 digest of a valid candidate firmware image stored in STM32 Flash.

**Expected Result:**
Calculated candidate-image digest matches the digest stored in candidate metadata.

**Status:** Verified

---

### TEST-INT-004 – Firmware Modification / SHA-256 Mismatch

**Objective:**
Modify the firmware image and verify that the Bootloader detects the change.

**Expected Result:**
Calculated digest does not match the stored digest and the affected firmware image is rejected.

**Status:** Verified

---

## 6. Memory Boundary Tests

### TEST-MEM-001 – Valid Application Region

**Objective:**
Verify that an application image within the configured memory region is accepted.

**Expected Result:**
Application passes memory validation.

**Status:** Verified

---

### TEST-MEM-002 – Invalid Application Size

**Objective:**
Verify that an image size outside the permitted range is rejected.

**Expected Result:**
Bootloader rejects the image.

**Status:** Verified

---

### TEST-MEM-003 – Invalid Reset Handler Address

**Objective:**
Verify that an application reset-handler address outside the permitted application region is rejected.

**Expected Result:**
Bootloader does not perform the application handover.

**Status:** Verified

---

## 7. Candidate / Active FOTA Tests

The current FOTA validation verifies candidate and active firmware independently without programming the candidate into the active image region.

### TEST-FOTA-001 – Active Metadata Read

Verify successful reading of active firmware metadata.

**Status:** Verified

### TEST-FOTA-002 – Active Metadata Read Failure

Verify handling when active metadata reading fails.

**Status:** Verified

### TEST-FOTA-003 – Active Metadata Validation Failure

Verify rejection of invalid active metadata.

**Status:** Verified

### TEST-FOTA-004 – Valid Active Metadata

Verify that valid active metadata allows the validation flow to continue.

**Status:** Verified

### TEST-FOTA-005 – Candidate Metadata Read Failure

Verify that a candidate metadata read failure does not prevent active-image fallback.

**Status:** Verified

### TEST-FOTA-006 – Valid Candidate Metadata

Verify acceptance of valid candidate metadata.

**Status:** Verified

### TEST-FOTA-007 – Invalid Candidate Metadata

Verify rejection of invalid candidate metadata.

**Status:** Verified

### TEST-FOTA-008 – Candidate Version Check

Verify that a candidate that is not newer than the active firmware is not processed as a new candidate.

**Status:** Verified

### TEST-FOTA-009 – Candidate SHA-256 Match

Verify that a newer candidate with a matching SHA-256 digest is accepted and booted.

**Status:** Verified

### TEST-FOTA-010 – Candidate SHA-256 Mismatch

Verify that a candidate with a SHA-256 mismatch is rejected.

**Status:** Verified

### TEST-FOTA-011 – Active SHA-256 Match

Verify that the active image passes integrity verification and is booted.

**Status:** Verified

### TEST-FOTA-012 – Active SHA-256 Mismatch

Verify that the active image is rejected when its SHA-256 verification fails and the Bootloader enters the fail-safe path.

**Status:** Verified

---

## 8. Application Handover Tests

### TEST-JMP-001 – Application Vector Table

**Objective:**
Verify that the application vector table is correctly configured before application handover.

**Expected Result:**
Application starts using its own vector table.

**Status:** Verified

---

### TEST-JMP-002 – Initial Stack Pointer

**Objective:**
Verify that the application's initial stack pointer is within the configured SRAM region.

**Expected Result:**
Valid stack pointer is accepted.

**Status:** Verified

---

### TEST-JMP-003 – Reset Handler

**Objective:**
Verify that the Bootloader transfers execution only when the application reset-handler address is valid.

**Expected Result:**
Valid reset-handler address allows application handover; invalid address prevents the jump.

**Status:** Verified

---

## 9. Negative Testing

Negative testing is used to verify that invalid firmware conditions are rejected safely.

Current negative test coverage includes:

* Invalid metadata
* Invalid magic number
* Invalid firmware size
* Invalid application boundary
* Invalid reset-handler address
* Modified firmware
* Candidate SHA-256 mismatch
* Active SHA-256 mismatch
* Candidate version rejection
* Metadata read failure

For every mandatory validation failure:

> **The affected firmware image shall not be executed.**

The candidate/active architecture additionally verifies that a rejected candidate does not prevent the Bootloader from attempting the active-image fallback.

---

## 10. Debugger-Based Fault Injection

GDB is used during validation to force specific conditions and verify the corresponding decision paths.

Examples include:

* Forcing validation conditions
* Inspecting metadata values
* Inspecting firmware addresses
* Verifying calculated values
* Entering invalid memory-validation conditions
* Confirming failure branches
* Confirming successful application handover

This approach allows individual bootloader branches to be validated without requiring additional communication hardware.

---

## 11. Test Evidence

Test evidence is maintained separately in the project validation records.

Evidence may include:

* Test case results
* GDB checkpoints
* Variable and memory inspection
* LED status indications
* Screenshots
* Firmware modification results
* Build and execution results

The validation records provide traceability between the requirement, test procedure, expected result, actual result, status, and evidence.

---

## 12. Requirement Traceability

Security requirements are mapped to corresponding verification activities.

Example:

```text id="eoj8ji"
SEC-INT-002
     |
     v
SHA-256 Verification
     |
     +------------------+
     |                  |
     v                  v
TEST-INT-002       TEST-INT-003
Active SHA         Candidate SHA
     |
     v
TEST-INT-004
SHA-256 Mismatch
```

The same approach is used for metadata validation, memory validation, application handover, candidate handling, and failure paths.

---

## 13. Regression Testing

Previously verified functionality shall be re-tested after significant changes to:

* Bootloader code
* Application code
* Candidate Application
* Metadata structure
* SHA-256 implementation
* Linker configuration
* Flash memory layout
* Firmware validation logic

Regression testing ensures that enhancements do not break previously verified boot and validation behavior.

---

## 14. Future Verification

The following verification activities will be added during future enhancement phases:

### Digital Signature Authentication

* Valid signature
* Invalid signature
* Modified signed firmware
* Incorrect public key
* Invalid signature data
* Host-side signing verification
* Signature verification performance

### Advanced FOTA

* Candidate-to-active image programming
* Persistent update-state handling
* Pending/confirmed image handling
* Automatic rollback
* Power-loss recovery
* Anti-rollback protection

### Production-Oriented Verification

* Extended fault-injection testing
* Security hardening tests
* Additional automated testing
* Increased unit-test coverage
* Tool-based coverage analysis
* ISO 26262-oriented verification activities

---

## 15. Implementation Status

**Status: Core Verification Completed**

The current Secure Boot and FOTA-oriented validation has been implemented and verified on the STM32F407.

Verified areas include:

* Bootloader startup
* Firmware metadata validation
* Magic-number validation
* Firmware-size validation
* Application memory-boundary validation
* SHA-256 implementation and test vectors
* SHA-256 verification of firmware stored in Flash
* Candidate firmware verification
* Active firmware verification
* Candidate version comparison
* Candidate rejection
* Active-image fallback
* Application vector validation
* Application handover
* Fail-safe handling

Digital Signature Authentication, candidate-to-active activation, persistent update state, pending/confirmed handling, rollback, and advanced recovery remain future enhancements.
