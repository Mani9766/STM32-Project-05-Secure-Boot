# Secure Boot – Test Strategy

## 1. Purpose

This document defines the initial verification and testing strategy for the Secure Boot implementation.

The objective is to demonstrate that the Bootloader correctly validates firmware and prevents execution of invalid or untrusted firmware.

## 2. Verification Approach

Testing will be performed at multiple levels:

- Unit testing
- Integration testing
- Boot flow testing
- Negative testing
- Security testing
- Boundary testing
- Fault injection
- Regression testing

## 3. Bootloader Tests

### TEST-BOOT-001 – Bootloader Startup

**Objective:**  
Verify that the Bootloader executes after MCU reset.

**Expected Result:**  
Bootloader starts successfully.

### TEST-BOOT-002 – Valid Application

**Objective:**  
Verify that a valid Application is detected and executed.

**Expected Result:**  
Bootloader validates the Application and transfers control to it.

### TEST-BOOT-003 – No Application

**Objective:**  
Verify behavior when no Application firmware is present.

**Expected Result:**  
Application execution is prevented and the defined failure/recovery mechanism is entered.

## 4. Integrity Tests

### TEST-INT-001 – Valid Firmware Hash

**Objective:**  
Verify that a valid firmware image passes hash verification.

**Expected Result:**  
Calculated hash matches the expected hash.

### TEST-INT-002 – Modified Firmware

**Objective:**  
Modify the Application firmware and verify the integrity check.

**Expected Result:**  
Hash verification fails and the Application is rejected.

### TEST-INT-003 – Corrupted Firmware

**Objective:**  
Introduce corruption into the firmware image.

**Expected Result:**  
Bootloader detects the corruption and prevents Application execution.

## 5. Authenticity Tests

### TEST-AUTH-001 – Valid Signature

**Objective:**  
Verify a firmware image signed using the trusted private key.

**Expected Result:**  
Signature verification succeeds.

### TEST-AUTH-002 – Invalid Signature

**Objective:**  
Replace or modify the firmware signature.

**Expected Result:**  
Signature verification fails and the Application is rejected.

### TEST-AUTH-003 – Wrong Signing Key

**Objective:**  
Sign the firmware using an untrusted private key.

**Expected Result:**  
Bootloader rejects the firmware.

## 6. Memory Boundary Tests

### TEST-MEM-001 – Valid Application Address

**Objective:**  
Verify that an Application located within the permitted memory region is accepted.

**Expected Result:**  
Application passes memory validation.

### TEST-MEM-002 – Application Outside Allowed Region

**Objective:**  
Provide an Application image that exceeds the configured memory boundary.

**Expected Result:**  
Bootloader rejects the image.

### TEST-MEM-003 – Invalid Application Size

**Objective:**  
Provide an invalid firmware size in the image metadata.

**Expected Result:**  
Bootloader rejects the image.

## 7. Application Handover Tests

### TEST-JMP-001 – Vector Table Configuration

**Objective:**  
Verify that the Application vector table is correctly configured before the jump.

**Expected Result:**  
Application starts using its own vector table.

### TEST-JMP-002 – Reset Handler

**Objective:**  
Verify that the Bootloader transfers execution to the correct Application reset handler.

**Expected Result:**  
Application starts successfully.

## 8. Negative Testing

Negative testing is an important part of Secure Boot verification.

The following invalid conditions will be tested:

- Invalid magic number
- Missing Application
- Corrupted firmware
- Modified firmware
- Invalid hash
- Invalid signature
- Wrong public key
- Invalid firmware size
- Invalid memory address
- Invalid vector table
- Unsupported firmware version

For every negative test:

> **The Application must not execute.**

## 9. Requirement Traceability

Each requirement will be mapped to one or more test cases.

Example:

```text
SEC-INT-002
    |
    v
Hash Verification
    |
    v
TEST-INT-001
TEST-INT-002
TEST-INT-003
```
## 10. Test Results

Test results will be recorded as the implementation progresses.

Example:
| Test ID       | Description        | Result | Status  |
| ------------- | ------------------ | ------ | ------- |
| TEST-BOOT-001 | Bootloader Startup | TBD    | Planned |
| TEST-BOOT-002 | Valid Application  | TBD    | Planned |
| TEST-INT-001  | Valid Hash         | TBD    | Planned |
| TEST-INT-002  | Modified Firmware  | TBD    | Planned |
| TEST-AUTH-001 | Valid Signature    | TBD    | Planned |

## 11. Regression Testing

Previously verified functionality shall be re-tested after significant changes to:

- Bootloader code
- Application code
- Firmware image format
- Cryptographic implementation
- Linker configuration
- Memory layout
## 12. Implementation Status

Status: Test Strategy Defined

Test cases will be implemented and executed as the Secure Boot functionality is developed.
