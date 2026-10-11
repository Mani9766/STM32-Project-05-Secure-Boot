# Threat Modeling & Security Hardening

## 1. Scope Definition

### Project Overview
- **Target:** STM32F407 microcontroller.
- **Architecture:** Secure Boot with SHA-256 firmware integrity verification and A/B FOTA.
- **Primary objective:** Detect invalid or corrupted firmware and prevent its execution through bootloader validation and recovery logic.
- **Bootloader responsibilities:**
  - Validate firmware metadata.
  - Calculate and compare SHA-256 digests.
  - Check firmware version and image parameters.
  - Select the appropriate firmware image.
  - Handle validation failures through fallback or fail-safe logic.
  - Validate application handover before transferring control.

### Security Objectives
- Detect firmware modifications through SHA-256 verification.
- Validate metadata before using it for firmware verification or selection.
- Prevent invalid image sizes and addresses from bypassing validation.
- Handle corrupted metadata and invalid firmware safely.
- Review interrupted updates and flash programming failures.
- Evaluate version checks and rollback-related behavior.
- Validate the application vector table, stack pointer, and reset-handler address.
- Maintain consistency between host-side metadata generation and bootloader verification.

### In Scope
- Bootloader validation and boot decision logic.
- Slot A, Slot B, and staging-area firmware.
- Active and candidate firmware metadata.
- SHA-256 calculation and digest comparison.
- Firmware image sizes, addresses, and flash boundaries.
- Metadata corruption and manipulation.
- Firmware version checks and rollback-related scenarios.
- Flash programming, erase operations, and interrupted updates.
- Candidate rejection, active-image fallback, and fail-safe handling.
- Application handover and vector-table configuration.
- Host-side firmware binary and metadata preparation.
- Update-protocol inputs and error handling as the protocol is integrated.

### Out of Scope
- Claims that SHA-256 verification proves the firmware source is trusted.
- Formal security certification or compliance claims.

### Key Limitation
SHA-256 comparison verifies that the calculated digest matches the expected digest stored in metadata. If both firmware and its expected digest can be modified, SHA-256 comparison alone cannot establish firmware authenticity. This project focuses on firmware integrity verification; authenticity is outside its implementation scope.

## 2. Security Asset Identification

### Asset Inventory

| Asset ID&nbsp;&nbsp;&nbsp; | Asset | Location | Protection Objective |
|---|---|---|---|
| AST-01 | Bootloader firmware | Sectors 0–1 | Preserve validation, boot selection, and recovery logic. |
| AST-02 | Active firmware metadata | Sector 2: `0x08008000` | Detect invalid or corrupted metadata before use. |
| AST-03 | Candidate firmware metadata | Sector 3: `0x0800C000` | Validate candidate metadata before accepting an image. |
| AST-04 | Staging firmware | Sector 4: `0x08010000` | Prevent incomplete or invalid staged firmware from being accepted. |
| AST-05 | Slot A firmware | Sector 5: `0x08020000` | Verify image integrity before execution when selected. |
| AST-06 | Slot B firmware | Sector 6: `0x08040000` | Verify image integrity before execution when selected. |
| AST-07 | SHA-256 verification logic | Bootloader | Calculate the digest over the intended image region and compare it correctly. |
| AST-08 | Firmware metadata fields | Metadata structures | Validate magic value, image size, version, digest, and applicable sequence fields. |
| AST-09 | Flash layout and boundaries | Bootloader constants and linker scripts | Prevent invalid memory ranges and incorrect image-region selection. |
| AST-10 | Flash programming and erase logic | Firmware update implementation | Prevent unintended flash modification and unsafe acceptance after interrupted writes. |
| AST-11 | Boot selection and recovery logic | Bootloader | Reject invalid candidates and follow the defined fallback or fail-safe path. |
| AST-12 | Application handover data | Application vector table and bootloader | Validate stack pointer, reset-handler address, and vector-table configuration. |
| AST-13 | Host-side metadata generator | `Tools/prepare_slot_metadata.py` | Generate metadata consistent with the firmware binary and verification format. |
| AST-14 | Firmware image representation | Build output and programmed flash | Ensure host-side hashing and device-side verification use the intended bytes and image size. |
| AST-15 | Update inputs | Staging and update mechanism | Handle corrupted, malformed, or incomplete firmware inputs safely. |

### Asset Identification Notes
- The bootloader, firmware images, metadata, and boot decision logic are critical
  assets because they directly affect whether firmware is accepted and executed.
- Firmware metadata and image contents must be evaluated together.
- Image size, slot boundaries, and digest calculation range must remain consistent
  across firmware packaging and bootloader verification.
- Application handover requires appropriate address and vector-table validation
  in addition to firmware digest verification.
- Asset protection effectiveness will be evaluated through threat scenarios,
  source-code review, security hardening, and relevant test evidence.
