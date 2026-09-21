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

The metadata contains info
