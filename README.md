# STM32-Project-05-Secure-Boot

A practical implementation of a Secure Boot flow for an STM32-based embedded system.

## Project Objective

The goal of this project is to design and implement a secure boot mechanism that ensures only trusted and valid firmware is executed by the microcontroller.

## Planned Secure Boot Flow

1. MCU Reset
2. Bootloader starts execution
3. Check application image
4. Verify firmware integrity
5. Verify firmware authenticity
6. Validate application image
7. Jump to application if verification succeeds
8. Enter failure/recovery handling if verification fails

## Project Scope

- STM32 Bootloader development
- Bootloader/Application memory partitioning
- Firmware image validation
- Cryptographic hash implementation
- Digital signature verification
- Secure application handover
- Invalid/corrupted firmware handling
- Linker script and memory layout
- Firmware update considerations
- Security and failure-handling mechanisms
- Unit testing and verification
- Future extension toward ISO 26262 safety concepts

## Hardware & Software

- **MCU:** STM32
- **Architecture:** ARM Cortex-M
- **Language:** Embedded C
- **IDE:** STM32CubeIDE
- **Debugging:** ST-LINK / SWD
- **Version Control:** Git / GitHub

## Repository Structure

```text
Secure-Boot/
├── Bootloader/
├── Application/
├── Documentation/
├── Tests/
└── README.md
