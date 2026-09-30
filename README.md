# STM32-Project-05-Secure-Boot

A practical implementation of a Secure Boot and dual-slot firmware update architecture for an STM32F407 embedded system.

The project focuses on firmware integrity verification using SHA-256, firmware metadata validation, secure application handover, A/B firmware slot architecture, and FOTA-oriented update handling.

Digital-signature-based firmware authenticity is planned as a future security extension.

## Project Objective

The goal of this project is to design and implement a secure firmware boot and update mechanism that verifies firmware integrity before execution and supports safer firmware replacement using two independent application slots.

The firmware architecture separates:

* a download/staging area for incoming firmware
* two bootable firmware slots
* slot-specific firmware metadata
* the bootloader responsible for validation and boot selection

The implementation is being developed incrementally, with SHA-256-based firmware integrity verification as the current security foundation.

## Secure Boot Flow

1. MCU Reset
2. Bootloader starts execution
3. Read firmware slot metadata
4. Identify the firmware image selected for boot
5. Validate metadata and application image boundaries
6. Calculate the selected firmware SHA-256
7. Compare the calculated digest with the stored reference digest
8. Boot the selected firmware when integrity verification succeeds
9. Enter the defined failure-handling path when firmware integrity verification fails

## A/B Firmware Architecture

The firmware update architecture uses two independent application slots so that the currently active firmware is preserved while a new firmware image is prepared and validated.

```text
STM32F407 Internal Flash
|
+-- Sector 0-1 : Secure Boot / Bootloader
|
+-- Sector 2   : Slot A Metadata
|
+-- Sector 3   : Slot B Metadata
|
+-- Sector 4   : Download / Staging Area
|
+-- Sector 5   : Firmware Slot A
|
+-- Sector 6   : Firmware Slot B
|
+-- Remaining  : Reserved / available Flash
```

### Slot Roles

Slot A and Slot B are fixed physical firmware locations. Their runtime role can change between updates.

```text
Initial state

Slot A : Firmware V1  → ACTIVE
Slot B : Empty        → INACTIVE
```

After a successful update:

```text
Slot A : Firmware V1  → Previous / INACTIVE
Slot B : Firmware V2  → ACTIVE
```

The next update uses the other slot:

```text
Slot A : Firmware V3  → ACTIVE
Slot B : Firmware V2  → Previous / INACTIVE
```

The system therefore alternates between Slot A and Slot B rather than permanently assigning one region as "active" and the other as "candidate".

## Download and Promotion Architecture

New firmware is first placed in the dedicated download/staging region.

```text
New Firmware
     |
     v
Sector 4
Download / Staging
     |
     v
Bootloader validation
     |
     v
Inactive Firmware Slot
     |
     v
Post-program verification
     |
     v
Active-slot activation
     |
     v
Boot selected firmware
```

The application image is not permanently associated with the staging area. Sector 4 is used only as temporary storage for a firmware update before it is promoted into the inactive A/B slot.

The A/B promotion and persistent update-state handling are currently being implemented.

## Slot-Specific Firmware Builds

Because Slot A and Slot B are located at different Flash addresses, each slot uses a linker configuration corresponding to its execution address.

```text
Firmware_Slot_A
    FLASH origin = 0x08020000
    VTOR         = 0x08020000

Firmware_Slot_B
    FLASH origin = 0x08040000
    VTOR         = 0x08040000
```

Both slot firmware projects use the same application functionality while being linked for their respective execution locations.

## Firmware Metadata

Each firmware slot has its own metadata.

```c
typedef struct
{
    uint32_t magic;
    uint32_t image_size;
    uint32_t version;
    uint8_t  sha256[SHA256_DIGEST_SIZE];
    uint32_t update_state;
} firmware_metadata_t;
```

Metadata is associated with the physical slot rather than with a permanent "active" or "candidate" role.

For example:

```text
Sector 2 → Slot A Metadata
Sector 3 → Slot B Metadata
```

The metadata contains properties and update information for the firmware stored in that slot. Fixed slot start/end addresses remain part of the bootloader and linker memory architecture rather than being duplicated in each metadata record.

## Integrity Verification

SHA-256 is used to verify firmware integrity.

The bootloader independently calculates the SHA-256 digest of the firmware image stored in the selected slot and compares it with the reference digest stored in the corresponding metadata.

```text
Firmware Image
      |
      v
   SHA-256
      |
      v
Calculated Digest
      |
      +------ Compare ------+
                             |
                    Stored Metadata Digest
```

A digest mismatch causes the image to be rejected rather than executed.

## Implemented Security Features

* STM32 Secure Boot bootloader
* Bootloader/application memory partitioning
* A/B firmware slot architecture
* Slot-specific linker configurations
* Slot-specific vector table configuration
* Firmware metadata management
* Application image boundary validation
* Firmware version validation
* SHA-256 implementation
* SHA-256 standard test-vector validation
* SHA-256 validation of firmware stored in STM32 Flash
* Firmware integrity verification
* Secure application handover
* Invalid metadata handling
* Invalid/corrupted firmware handling
* Download/staging area for FOTA architecture

## Current Development Focus

The current development phase is focused on completing the production-oriented A/B firmware update mechanism.

Planned implementation steps include:

1. Define persistent firmware update states
2. Determine active and inactive firmware slots
3. Validate firmware stored in the download/staging area
4. Program the validated firmware into the inactive slot
5. Verify the programmed destination image
6. Safely commit the new active slot
7. Handle reset and power-loss conditions during update processing
8. Implement rollback/recovery behavior

## Planned Security Extension

Digital signature verification will be added after the A/B firmware update architecture and update-state handling are completed.

SHA-256 provides firmware integrity verification by detecting changes to the firmware image. Digital signatures will extend the design by providing firmware authenticity and allowing the bootloader to verify that an image was produced by a trusted signing source.

## Verification & Testing

Validation is performed incrementally during implementation.

Current verification areas include:

* SHA-256 standard test vectors
* Empty and single-block inputs
* Padding-boundary test cases
* Multi-block SHA-256 inputs
* STM32 Flash image hashing
* Firmware integrity verification
* Firmware metadata validation
* Firmware version comparison
* Application memory boundary validation
* Slot-specific firmware execution
* Vector table configuration
* Invalid/corrupted firmware handling

Future validation will cover:

* Active/inactive slot selection
* Firmware promotion
* Persistent update-state transitions
* Interrupted update recovery
* Power-loss handling
* Rollback behavior
* Post-promotion verification
* Digital signature verification

Test evidence is maintained separately using validation test cases, debugger checkpoints, LED status indication, and screenshots.

## Hardware & Software

* **MCU:** STM32F407
* **Architecture:** ARM Cortex-M4
* **Language:** Embedded C
* **IDE:** STM32CubeIDE
* **Debugging:** ST-LINK / SWD / GDB
* **Build / Image Handling:** STM32CubeIDE build and post-build processing
* **Version Control:** Git / GitHub

## Repository Structure

```text
STM32-Project-05-Secure-Boot/
├── Bootloader/
│   └── Secure_Boot/
│       └── STM32F407 Secure Boot bootloader
├── Firmware_Slot_A/
│   └── Firmware linked for Slot A
├── Firmware_Slot_B/
│   └── Firmware linked for Slot B
├── Docs/
│   └── Project documentation and design notes
├── Validation/
│   └── Test cases, validation results, and evidence
├── LICENSE
└── README.md
```

## Memory Architecture

| Region     |           Address | Purpose            |
| ---------- | ----------------: | ------------------ |
| Sector 0-1 | Bootloader region | Secure Boot        |
| Sector 2   |      `0x08008000` | Slot A metadata    |
| Sector 3   |      `0x0800C000` | Slot B metadata    |
| Sector 4   |      `0x08010000` | Download / staging |
| Sector 5   |      `0x08020000` | Firmware Slot A    |
| Sector 6   |      `0x08040000` | Firmware Slot B    |

## Future Enhancements

* Production-grade persistent update-state handling
* Candidate validation and firmware promotion
* Power-loss-resilient update processing
* Automatic rollback and recovery
* Digital signature verification
* Authenticated firmware updates
* Automated firmware image generation and validation
* ISO 26262-oriented verification
* Increased unit-test coverage
* Tool-based coverage analysis
