# STM32F407 FOTA Tools

Host-side tools used to prepare firmware metadata and support manual FOTA testing for the STM32F407 Secure Boot project.

## Tools

### `prepare_slot_metadata.py`

Used for firmware that represents the **final image in Slot A or Slot B**.

It uses the ELF to generate a flash-equivalent image and fills linker gaps with `0xFF` before calculating the SHA-256 reference.

### `prepare_staging_metadata.py`

Used for firmware that is **downloaded to the staging area (Sector 4)**.

It calculates SHA-256 directly from the raw `.bin` because the bootloader validates the exact bytes stored in the staging area.

## Test Data

`Test_Data/` contains generated metadata and firmware reference files used during manual validation.

- `Slot_A/` — Slot A metadata/test data
- `Slot_B/` — Slot B metadata/test data

The generated files are test artifacts and are used to reproduce and verify the FOTA flow.

For the detailed preparation, GDB programming, and validation procedure, see:

`Slot_Metadata_Preparation_Procedure.md`
