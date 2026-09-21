# Secure Boot Flow

## 1. Overview

The Secure Boot process verifies the application firmware before allowing execution.

The Bootloader is the first software component executed after MCU reset. It validates the application metadata, verifies the firmware image integrity using SHA-256, and transfers control to the application only when the defined validation checks succeed.

## 2. Boot Flow

```text
                 MCU RESET
                     |
                     v
              +--------------+
              |  Bootloader  |
              +--------------+
                     |
                     v
             Read Application
                 Metadata
                     |
                     v
            Validate Metadata
                     |
              +------+------+
              |             |
           INVALID        VALID
              |             |
              v             v
          Fail-Safe    Validate Image
                        Boundaries
                            |
                      +-----+-----+
                      |           |
                   INVALID       VALID
                      |           |
                      v           v
                  Fail-Safe   Calculate
                              SHA-256
                                 |
                          Compare Digest
                                 |
                      +----------+----------+
                      |                     |
                   MISMATCH                MATCH
                      |                     |
                      v                     v
                  Fail-Safe        Prepare Application
                                         Handover
                                             |
                                             v
                                   Jump to Application
                                             |
                                             v
                                        APPLICATION
```

## 3. Detailed Steps

### Step 1 – MCU Reset

After reset, the MCU starts execution from the configured boot memory region.

The Bootloader is located at the beginning of the boot memory region and executes first.

### Step 2 – Bootloader Initialization

The Bootloader performs the minimum required initialization for firmware validation and application handover.

This includes the required hardware and system initialization used by the bootloader.

### Step 3 – Application Metadata Read

The Bootloader reads the firmware metadata associated with the active application image.

The metadata contains information used to validate the firmware image, including:

* Metadata magic value
* Firmware image size
* Firmware version
* Reference SHA-256 digest

### Step 4 – Metadata Validation

The Bootloader validates the metadata before processing the application image.

Validation includes checking:

* Metadata validity
* Firmware image size
* Application image boundaries
* Address range against the configured application region

Invalid metadata causes the Bootloader to enter the defined failure-handling path.

### Step 5 – Firmware Integrity Verification

The Bootloader independently calculates the SHA-256 digest of the application firmware stored in STM32 internal Flash.

The calculated digest is compared with the SHA-256 digest stored in the firmware metadata.

```text
Application Firmware
        |
        v
   SHA-256 Calculation
        |
        v
Calculated Digest
        |
        +---------- Compare ----------+
                                     |
                              Stored Metadata
                              SHA-256 Digest
```

### Step 6 – Verification Decision

If the calculated SHA-256 digest matches the stored reference digest, the application passes the firmware integrity check.

If the digest does not match, the application is rejected and the Bootloader enters the defined failure-handling path.

### Step 7 – Application Handover

After successful validation, the Bootloader prepares the processor for application execution.

The application vector table is configured and the application's initial stack pointer and reset handler address are obtained from the application image.

The Bootloader validates the application reset handler address before transferring control.

### Step 8 – Application Execution

When all required validation checks succeed, the Bootloader transfers execution to the application reset handler.

The validated application firmware then starts execution.

## 4. Failure Handling

When metadata validation, application boundary validation, or SHA-256 integrity verification fails, the Bootloader must not execute the invalid application.

```text
Firmware Validation
        |
        v
      FAILED
        |
        v
Do NOT execute application
        |
        v
Bootloader Fail-Safe Path
```

The current implementation uses a defined Bootloader failure path for invalid firmware conditions.

## 5. Security Principle

The fundamental security rule of the current implementation is:

**Firmware that fails the defined validation and integrity checks must never be executed.**

The Bootloader provides the initial validation boundary between MCU reset and application execution.

The current security mechanism provides firmware **integrity verification using SHA-256**. Firmware authenticity verification using digital signatures is planned as a future enhancement.

## 6. Current Verification Coverage

The Secure Boot implementation has been validated for:

* Firmware metadata validation
* Application image boundary validation
* SHA-256 test vectors
* SHA-256 calculation on firmware stored in STM32 Flash
* Matching SHA-256 digest
* SHA-256 mismatch detection
* Invalid firmware handling
* Application reset-handler validation
* Successful application handover

## 7. Future Extensions

The Secure Boot flow can be extended with:

* Digital Signature Authentication
* Authenticated firmware updates
* Anti-rollback protection
* Enhanced recovery mechanisms
* Persistent firmware update state
* Production-oriented security hardening
* Security-related testing
* Safety requirements and verification activities
* ISO 26262-oriented verification practices
