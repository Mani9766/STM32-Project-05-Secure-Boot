# Memory Architecture

## 1. Overview

The Secure Boot project uses dedicated Flash regions for the Bootloader, firmware metadata, candidate firmware, and active firmware.

The Bootloader executes first after MCU reset. It validates firmware metadata and image boundaries, performs SHA-256 integrity verification, and transfers control to a validated application image.

The memory architecture also provides separate candidate and active firmware regions for the current FOTA-oriented validation flow.

---

## 2. Flash Memory Layout

The current STM32F407 internal Flash organization is:

```text
STM32F407 Internal Flash

┌──────────────┬──────────────┬────────────────────────────┐
│   ADDRESS    │    SECTOR    │           ROLE             │
├──────────────┼──────────────┼────────────────────────────┤
│ 0x08000000   │   Sector 0   │ Bootloader                 │
│ 0x08004000   │   Sector 1   │ Bootloader                 │
│ 0x08008000   │   Sector 2   │ Active Image Metadata      │
│ 0x0800C000   │   Sector 3   │ Candidate Image Metadata  │
│ 0x08010000   │   Sector 4   │ Candidate Firmware        │
│ 0x08020000   │   Sector 5   │ Active Firmware           │
│ 0x08040000   │   Sector 6   │ Active Firmware           │
│ 0x08060000   │   Sector 7   │ Active Firmware           │
│ 0x08080000   │   Sector 8   │ Available / Reserved      │
│ 0x080A0000   │   Sector 9   │ Available / Reserved      │
│ 0x080C0000   │   Sector 10  │ Available / Reserved      │
│ 0x080E0000   │   Sector 11  │ Available / Reserved      │
├──────────────┴──────────────┴────────────────────────────┤
│ 0x080FFFFF   │            Flash End Address              │
└─────────────────────────────────────────────────────────┘
```

### Key Memory Boundaries

* **Bootloader:** `0x08000000 – 0x08007FFF`
* **Active Metadata:** `0x08008000 – 0x0800BFFF`
* **Candidate Metadata:** `0x0800C000 – 0x0800FFFF`
* **Candidate Firmware:** `0x08010000 – 0x0801FFFF`
* **Active Firmware:** starts at `0x08020000`
* **Flash End:** `0x080FFFFF`

The candidate and active firmware images are intentionally stored at different Flash locations so that the active firmware remains available while the candidate image is being validated.

---

## 3. Bootloader Region

The Bootloader occupies dedicated Flash memory in Sectors 0 and 1.

### Responsibilities

* MCU startup handling
* Firmware metadata handling
* Application image validation
* Firmware version validation
* SHA-256 integrity verification
* Candidate image validation
* Active-image fallback
* Failure handling
* Application handover

The Bootloader region is separated from the firmware image regions and is not part of the application integrity calculation.

---

## 4. Metadata Regions

Metadata is stored separately from the firmware images.

### Active Image Metadata

* **Flash sector:** Sector 2
* **Base address:** `0x08008000`
* **Purpose:** Stores metadata describing the active firmware image

### Candidate Image Metadata

* **Flash sector:** Sector 3
* **Base address:** `0x0800C000`
* **Purpose:** Stores metadata describing the candidate firmware image

The metadata contains information required by the Bootloader for image validation, including:

* Firmware metadata magic value
* Firmware image size
* Firmware version
* SHA-256 reference digest

Metadata is validated before the corresponding firmware image is processed.

---

## 5. Candidate Firmware Region

The candidate firmware is stored separately from the active firmware.

* **Start address:** `0x08010000`
* **Region:** Sector 4
* **Purpose:** Stores a candidate firmware image for FOTA validation

The Bootloader validates the candidate metadata, verifies the image boundaries, checks the firmware version, independently calculates the candidate SHA-256 digest, and compares it with the digest stored in candidate metadata.

A valid candidate is currently executed directly from the candidate Flash location.

Candidate-to-active image programming is not part of the current implementation and is planned as a future FOTA enhancement.

---

## 6. Active Firmware Region

The active firmware is stored starting from:

* **Start address:** `0x08020000`
* **Region:** Sector 5 and subsequent available firmware sectors

The active firmware is the primary fallback image used by the Bootloader.

Before execution, the Bootloader validates the active metadata and independently calculates the SHA-256 digest of the active firmware.

The active application is executed only when the required validation and integrity checks succeed.

---

## 7. Vector Table

The Bootloader and application firmware each have their own vector tables.

The Bootloader starts execution using its own vector table.

Before transferring control to an application, the Bootloader configures the Vector Table Offset Register (VTOR) to the application's vector table address.

Conceptually:

```text
Bootloader Vector Table
        |
        v
Bootloader Execution
        |
        | Validate Image
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

For the current firmware layout:

```text
Candidate Vector Table → 0x08010000
Active Vector Table    → 0x08020000
```

The Bootloader also validates the application's initial stack pointer and reset-handler address before performing the application handover.

---

## 8. Linker Script Configuration

Separate linker configurations are used for the Bootloader, Candidate Application, and Active Application.

The linker scripts define:

* Flash origin
* Flash length
* RAM origin
* RAM length
* Code sections
* Read-only data
* Initialized data
* Uninitialized data
* Stack
* Heap

The Candidate Application and Active Application use different Flash origins because they are executed from different memory locations.

The application linker configuration must ensure that the application image remains within its assigned Flash region and cannot overwrite the Bootloader or metadata regions.

---

## 9. Memory Protection and Validation

The memory architecture ensures that:

* Bootloader and application regions are separated.
* Active and candidate firmware are stored independently.
* Metadata is stored in dedicated Flash sectors.
* Firmware images are processed using known start addresses.
* Image sizes are validated against the configured memory boundaries.
* The Bootloader validates the application reset-handler address before handover.
* Firmware integrity verification operates on the intended Flash image region.

Additional hardware memory-protection mechanisms may be considered during future security hardening.

---

## 10. Current Design Considerations

The current implementation intentionally separates:

* Bootloader
* Active firmware metadata
* Candidate firmware metadata
* Candidate firmware
* Active firmware

The following mechanisms are outside the current memory-management scope and are planned for future enhancement:

* Candidate-to-active image programming
* Persistent update-state storage
* Pending/confirmed image state
* Rollback storage or swap area
* Power-loss-resilient update handling
* Digital signature metadata and key management

These mechanisms can be added later without changing the fundamental separation between the Bootloader, candidate image, and active image regions.

---

## 11. Implementation Status

**Status: Implemented and Verified**

The current memory architecture has been implemented and validated with the STM32F407 firmware projects.

### Current Addresses

* **Bootloader start:** `0x08000000`
* **Active metadata:** `0x08008000`
* **Candidate metadata:** `0x0800C000`
* **Candidate firmware:** `0x08010000`
* **Active firmware:** `0x08020000`
* **Flash end:** `0x080FFFFF`

The linker configurations and application vector-table locations have been configured accordingly.

The current implementation supports independent candidate and active firmware validation using metadata and SHA-256 integrity verification.
