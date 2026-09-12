# Memory Architecture

## 1. Overview

The Secure Boot project uses separate memory regions for the Bootloader and Application.

The Bootloader executes first after MCU reset and is responsible for validating the Application before transferring control to it.

## 2. Flash Memory Layout

The initial memory layout is planned as:

```text
STM32F407 Internal Flash
┌──────────────┬──────────────┬──────────────────────┐
│   ADDRESS    │    SECTOR    │        ROLE          │
├──────────────┼──────────────┼──────────────────────┤
│ 0x08000000   │   Sector 0   │ Bootloader           │
│ 0x08004000   │   Sector 1   │ Bootloader           │
│ 0x08008000   │   Sector 2   │ Firmware Metadata    │
│ 0x0800C000   │   Sector 3   │ Application          │
│ 0x08010000   │   Sector 4   │ Application          │
│ 0x08020000   │   Sector 5   │ Application          │
│ 0x08040000   │   Sector 6   │ Application          │
│ 0x08060000   │   Sector 7   │ Application          │
├──────────────┼──────────────┼──────────────────────┤
│ 0x08080000   │    End       │                      │
└──────────────┴──────────────┴──────────────────────┘
```
And the key boundaries are:

- Bootloader  : 0x08000000 – 0x08007FFF
- Metadata    : 0x08008000 – 0x0800BFFF
- Application : 0x0800C000 – 0x0807FFFF

## 3. Bootloader Region

The Bootloader occupies a dedicated region of Flash memory.

Responsibilities include:

- MCU startup handling
- Application detection
- Firmware validation
- Integrity verification
- Authenticity verification
- Application handover
- Failure/recovery handling

The Bootloader must remain available even when the Application firmware is updated.

## 4. Application Region

The Application is placed at a different Flash address from the Bootloader.

The Application linker script must therefore be configured with the correct Flash origin and available Flash size.

The Application must not overwrite the Bootloader region.

## 5. Vector Table

Both the Bootloader and Application have their own vector tables.

The Bootloader starts from its configured vector table.

Before jumping to the Application, the Bootloader must configure the Vector Table Offset Register (VTOR) to point to the Application's vector table.

Conceptually:
```text
Bootloader Vector Table
        |
        v
Bootloader Execution
        |
        | Validate Application
        |
        v
Application Vector Table
        |
        v
Application Reset Handler
        |
        v
Application Execution
```
## 6. Linker Script

Separate linker configurations are required for the Bootloader and Application.

The linker scripts define:

- Flash origin
- Flash size
- RAM origin
- RAM size
- Code sections
- Read-only data
- Initialized data
- Uninitialized data
- Stack
- Heap

The Application linker configuration must account for the Flash space occupied by the Bootloader.

## 7. Memory Protection

The memory architecture must ensure that:

- Application code cannot overwrite the Bootloader region.
- Bootloader and Application regions are clearly separated.
- Firmware images are stored at known addresses.
- Verification operates on the intended memory region.

Additional hardware memory protection mechanisms may be considered during security hardening.

## 8. Design Considerations

The following items will be finalized during implementation:

- Bootloader start address
- Application start address
- Application maximum size
- Metadata location
- Firmware image format
- Signature/hash storage
- Reserved Flash regions
- RAM requirements
- Update/recovery area
## 9. Implementation Status

Status: Implemented and Verified

The memory addresses and linker configurations are as follows:
- Bootloader start address: 0x08000000
- Application start address: 0x0800C000
- Application maximum region: 0x0800C000–0x0807FFFF
- Metadata location: Sector 2, 0x08008000–0x0800BFFF
- Signature/hash storage: Metadata region
- Reserved Flash regions: Sector 2 for metadata
