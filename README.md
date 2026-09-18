# STM32-Project-05-Secure-Boot

A practical implementation of a Secure Boot and dual-image firmware update architecture for an STM32F407 embedded system. The project focuses on firmware integrity verification using SHA-256, metadata validation, secure application handover, candidate/active image management, and FOTA-oriented rollback handling.

Digital-signature-based firmware authenticity is planned as a future security extension.

## Project Objective

The goal of this project is to design and implement a secure firmware boot and update mechanism that verifies firmware integrity before execution and separates candidate firmware from the currently active firmware.

The implementation is being developed incrementally, with SHA-256-based firmware integrity verification as the core security mechanism. The dual-image architecture provides the foundation for safe firmware updates and rollback handling.

## Current Secure Boot Flow

1. MCU Reset
2. Bootloader starts execution
3. Read active firmware metadata
4. Validate metadata and application image boundaries
5. Calculate the active firmware SHA-256
6. Compare the calculated digest with the stored reference digest
7. Boot the active application when integrity verification succeeds
8. Reject the image and enter the defined failure-handling path when integrity verification fails

## Current Dual-Image / FOTA Architecture

The firmware update architecture uses separate candidate and active image regions so that a candidate firmware update does not overwrite the currently active application.

```text
STM32F407 Internal Flash
|
+-- Sector 0-1 : Bootloader
|
+-- Sector 2   : Active Image Metadata
|
+-- Sector 3   : Candidate Image Metadata
|
+-- Sector 4   : Candidate Firmware
|
+-- Sector 5   : Active Firmware
```

Current candidate-image validation flow:

1. Read candidate firmware metadata
2. Validate candidate metadata
3. Validate candidate image boundaries
4. Calculate the candidate SHA-256 independently in the bootloader
5. Compare the calculated digest with the candidate reference digest
6. Reject the candidate when verification fails
7. Preserve the existing active firmware while evaluating the candidate
8. Boot the candidate when integrity verification succeeds
9. Confirm the candidate after successful application startup
10. Roll back to the previous active image when the candidate fails to boot or remains unconfirmed

## Firmware Update State Handling

The candidate firmware uses an update-state mechanism to track its activation and confirmation status.

```text
EMPTY
  |
  v
PENDING_VALIDATION
  |
  | SHA-256 verified
  v
BOOT_PENDING
  |
  | Application confirms successful boot
  v
CONFIRMED

BOOT_PENDING
  |
  | Reset without confirmation
  v
ROLLBACK
```

The final rollback and confirmation handling is being validated through bootloader and application-level test scenarios.

## Implemented Security Features

* STM32 bootloader implementation
* Bootloader/application memory partitioning
* Active and candidate firmware image separation
* Firmware metadata management
* Application image boundary validation
* SHA-256 implementation
* SHA-256 test-vector validation
* SHA-256 validation of firmware stored in STM32 flash
* Candidate firmware SHA-256 verification
* Active firmware SHA-256 verification
* Secure application handover
* Invalid/corrupted firmware handling
* FOTA-oriented candidate/active image architecture
* Update-state handling and rollback mechanism under validation

## Planned Security Extension

Digital signature verification will be added after the SHA-256 integrity and firmware-update flow is completed.

SHA-256 provides firmware integrity verification by detecting changes to the firmware image. Digital signatures will extend the design by providing firmware authenticity and enabling verification of trusted firmware sources.

## Verification & Testing

Validation is being performed incrementally during implementation.

Current verification areas include:

* SHA-256 standard test vectors
* Empty and single-block inputs
* Padding-boundary test cases
* Multi-block SHA-256 inputs
* STM32 flash image hashing
* Active-image SHA-256 verification
* Candidate-image SHA-256 verification
* Candidate hash mismatch handling
* Metadata validation
* Application memory boundary validation
* Candidate/active image boot behavior
* Update-state transitions
* Rollback behavior

Test evidence is maintained separately using validation test cases, debugger checkpoints, UART output, and screenshots.

## Hardware & Software

* **MCU:** STM32F407
* **Architecture:** ARM Cortex-M4
* **Language:** Embedded C
* **IDE:** STM32CubeIDE
* **Debugging:** ST-LINK / SWD / GDB
* **Build / Image Handling:** Post-build processing and Python
* **Version Control:** Git / GitHub

## Repository Structure

```text
STM32-Project-05-Secure-Boot/
├── Application/
│   └── Main application firmware
├── Candidate App/
│   └── Candidate firmware used for dual-image/FOTA testing
├── Firmware/
│   └── STM32F407 bootloader implementation
│       ├── Core/
│       ├── Drivers/
│       ├── Modules/
│       ├── Tools/
│       ├── Debug/
│       ├── STM32F407VGTX_FLASH.ld
│       └── STM32F407VGTX_RAM.ld
├── Docs/
│   └── Project documentation and design notes
├── Validation/
│   └── Test cases, validation results, and evidence
├── LICENSE
└── README.md
```

## Future Extensions

* Digital signature verification
* Authenticated firmware updates
* Enhanced rollback/recovery mechanisms
* Automated firmware image generation and validation
* ISO 26262-oriented verification
* Increased unit-test coverage and tool-based coverage analysis
