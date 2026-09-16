# STM32-Project-05-Secure-Boot

A practical implementation of a Secure Boot and firmware integrity verification flow for an STM32-based embedded system, with a dual-image firmware update architecture planned for FOTA and a later extension for digital-signature-based firmware authenticity.

## Project Objective

The goal of this project is to design and implement a secure firmware boot and update mechanism that verifies firmware integrity before execution and provides a foundation for authenticated firmware updates and rollback handling.

The project is being developed incrementally, with SHA-256-based firmware integrity verification forming the core security mechanism and digital signatures planned as a later authenticity layer.

## Current Secure Boot Flow

1. MCU Reset
2. Bootloader starts execution
3. Read firmware metadata
4. Validate metadata
5. Validate application image boundaries
6. Calculate application SHA-256
7. Compare calculated digest with the stored reference digest
8. Jump to the application when integrity verification succeeds
9. Reject the application when integrity verification fails

## Planned FOTA / Dual-Image Flow

The firmware update architecture will use separate candidate and active image regions so that an invalid candidate firmware image does not overwrite the currently active application.

```text
Bootloader
   |
   +-- Sector 2: Metadata / Update State
   |
   +-- Sector 3-4: Candidate Image
   |
   +-- Sector 5+: Active Image
```

Planned candidate update flow:

1. Receive and store the new candidate firmware
2. Read candidate metadata
3. Validate candidate image boundaries
4. Calculate the candidate SHA-256 independently in the bootloader
5. Compare the calculated digest with the candidate reference digest
6. Reject the candidate and continue with the active image when verification fails
7. Activate the candidate when verification succeeds
8. Confirm the new image after successful boot
9. Roll back to the previous active image if the candidate is not successfully confirmed

## Planned Security Extension

Digital signature verification will be added after the SHA-256-based integrity and firmware-update flow is completed.

SHA-256 provides firmware integrity verification by detecting modifications to the firmware image. Digital signatures will extend the design by providing firmware authenticity and establishing trust in the firmware source.

## Project Scope

* STM32 bootloader development
* Bootloader/application memory partitioning
* Firmware metadata management
* Application image boundary validation
* SHA-256 implementation and validation
* Firmware integrity verification
* Candidate and active firmware image management
* FOTA update flow
* Candidate rejection and rollback handling
* Secure application handover
* Invalid/corrupted firmware handling
* Linker script and memory layout
* Post-build and Python-based firmware image handling
* Digital signature verification as a future security extension
* Unit testing and verification
* Future extension toward ISO 26262-oriented verification

## Hardware & Software

* **MCU:** STM32F407
* **Architecture:** ARM Cortex-M4
* **Language:** Embedded C
* **IDE:** STM32CubeIDE
* **Debugging:** ST-LINK / SWD
* **Build / Image Handling:** Post-build processing and Python
* **Version Control:** Git / GitHub

## Repository Structure

```text
Secure-Boot/
├── Bootloader/
├── Application/
├── Documentation/
├── Tests/
└── README.md
```
