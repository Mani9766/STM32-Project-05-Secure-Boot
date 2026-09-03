# Failure Handling

## 1. Overview

A Secure Bootloader must define what happens when the Application firmware cannot be trusted or safely executed.

The fundamental rule is:

> If firmware verification fails, the Bootloader must not execute the Application.

## 2. Possible Failure Conditions

The Bootloader may detect failures such as:

- Application firmware is not present.
- Invalid firmware header.
- Invalid magic number.
- Firmware size is outside the allowed range.
- Application address is invalid.
- Firmware extends outside the Application memory region.
- Firmware hash verification fails.
- Digital signature verification fails.
- Firmware version is not acceptable.
- Application vector table is invalid.
- Firmware image is corrupted.

## 3. Failure Decision

The general decision flow is:

```text
              Verify Application
                     |
                +----+----+
                |         |
             VALID       INVALID
                |           |
                v           v
        Prepare Application  |
                |             |
                v             v
       Jump to Application  Reject
                              |
                              v
                       Failure Handling
```
## 4. Application Not Present

If no valid Application is detected, the Bootloader must not attempt to jump to the Application.

Possible actions include:

- Enter recovery mode.
- Wait for a firmware update.
- Report an error.
- Remain in Bootloader mode.

The exact behavior will be defined during implementation.
## 5. Integrity Verification Failure

If the calculated firmware hash does not match the expected value, the firmware must be rejected.
```text
Calculated Hash
       |
       v
Compare with Expected Hash
       |
       +---- Match ----> Continue
       |
       +---- Mismatch -> Reject Firmware
```
Possible causes include:
- Flash corruption
- Incomplete firmware update
- Accidental modification
- Unauthorized modification

## 6. Authenticity Verification Failure

If digital signature verification fails, the firmware must not be executed.

Possible causes include:

- Invalid signature
- Incorrect signing key
- Modified firmware
- Untrusted firmware source
- Corrupted signature data
## 7. Invalid Memory Configuration

The Bootloader must verify that the Application image remains within its allocated memory region.
```text
Application Start
       |
       v
Application Size
       |
       v
Within Allowed Flash Region?
       |
    +--+--+
   YES    NO
    |      |
    v      v
 Continue Reject
```
This prevents an invalid image from causing the Bootloader to access unintended memory regions.

## 8. Recovery Mechanism

A recovery mechanism may allow the device to receive a valid firmware image after an invalid image is detected.

Potential recovery mechanisms include:

- UART
- CAN
- USB
- Other supported communication interfaces

The recovery mechanism will be selected based on the target system.

## 9. Fail-Safe Principle

The Bootloader should follow a fail-safe approach:
```text
Unable to establish firmware trust
              |
              v
       Do NOT execute
              |
              v
       Enter safe state /
       recovery mechanism
```
The system must never assume that an unverified firmware image is safe to execute.

## 10. Logging and Diagnostics

During development, diagnostic information may be provided to help identify verification failures.

Examples:

- Invalid header
- Invalid firmware size
- Hash mismatch
- Signature verification failure
- Invalid vector table

Production implementations should carefully control diagnostic information to avoid exposing sensitive security details.

## 11. Testing

Failure handling will be tested using intentionally invalid firmware images.

Planned tests include:

- No Application present
- Corrupted Application
- Modified Application
- Invalid hash
- Invalid signature
- Wrong public key
- Invalid firmware size
- Invalid Application address
- Invalid vector table
## 12. Implementation Status

Status: Design Phase

Failure and recovery behavior will be finalized as the Secure Boot implementation progresses.
