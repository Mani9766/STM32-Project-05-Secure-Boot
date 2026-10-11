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

  ## 3. Attack Surface Identification

### Attack Surface Inventory

| Attack Surface ID&nbsp;&nbsp; | Attack Surface | Potential Security Concern | Related Assets |
|---|---|---|---|
| AS-01 | Firmware images in staging, Slot A, and Slot B | Firmware corruption or modification may go undetected if validation is missing, incorrect, or performed on the wrong image region. | AST-02, AST-03, AST-04 |
| AS-02 | Firmware metadata | Corrupted or manipulated magic values, image sizes, versions, digests, sequence numbers, or state fields may affect validation and boot decisions. | AST-05, AST-06, AST-08 |
| AS-03 | Image address and size validation | Invalid addresses, oversized images, or arithmetic overflow may cause incorrect memory-range checks or verification of unintended flash contents. | AST-07, AST-08, AST-09 |
| AS-04 | SHA-256 calculation and comparison | Incorrect digest length, image boundaries, or error handling may cause corrupted firmware to be accepted or valid firmware to be rejected. | AST-07, AST-14 |
| AS-05 | Staging-to-slot firmware transfer | An interrupted or incomplete copy from staging to the destination slot may leave an invalid candidate image. | AST-03, AST-04, AST-10 |
| AS-06 | Flash erase and programming operations | Incorrect sector selection, programming failures, or interrupted operations may corrupt firmware or metadata and leave inconsistent update state. | AST-05, AST-06, AST-10 |
| AS-07 | Metadata state transitions and boot selection | Invalid or inconsistent state transitions may cause the bootloader to select an unvalidated image or mishandle recovery. | AST-05, AST-06, AST-11 |
| AS-08 | Application handover | Invalid stack-pointer or reset-handler values, incorrect vector-table configuration, or address mismatches may cause an unsafe jump to an application. | AST-02, AST-03, AST-11, AST-12 |
| AS-09 | Host-side firmware and metadata preparation | Mismatches between binary size, padding, digest calculation, and device-side verification may cause incorrect verification results. | AST-13, AST-14 |
| AS-10 | Firmware update inputs and protocol | Malformed, oversized, corrupted, or incomplete update data may reach the staging or flash-programming logic without adequate validation. | AST-04, AST-10, AST-15 |
| AS-11 | Debug and programming interface | Programming access may permit unintended firmware or metadata modification if access is available and not appropriately restricted. | AST-01, AST-02, AST-03, AST-05, AST-06 |

### Attack Surface Assessment Notes

- **Flash contents:** Firmware images and metadata can be affected by corruption, unintended writes, or unauthorized modification.
- **Metadata processing:** Fields used for image validation and boot selection must be validated before use.
- **Image verification:** The image address, size, and exact bytes used for SHA-256 calculation must be consistent across host-side and bootloader implementations.
- **Update operations:** Firmware copying, flash erase/program operations, and metadata state transitions must handle failures and interruptions safely.
- **Boot decision and handover:** Only an image that satisfies the implemented validation requirements should be selected for execution. Its execution addresses must also be checked.
- **External inputs:** Protocol-specific attack surfaces will be assessed against the implemented update protocol once integration is complete.
- **Debug access:** Security restrictions on SWD/ST-LINK access depend on the device configuration and deployment assumptions; they are not presumed to be enabled.

The attack surfaces listed above are potential points of influence, not confirmed vulnerabilities. Their actual exploitability and the effectiveness of existing controls will be evaluated through threat scenario analysis, source-code review, and testing.
