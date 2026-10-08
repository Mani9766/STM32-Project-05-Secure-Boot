"""
Prepare candidate metadata for the FOTA staging area.

This script is intentionally different from prepare_slot_metadata.py.
It hashes the RAW .bin exactly as it will be written to Sector 4.

Use this for:
    Firmware_Staging.bin -> Sector 4 (0x08010000)

The ELF is NOT used here, because the staging image is copied as raw
bytes by the bootloader.
"""

import argparse
import hashlib
from pathlib import Path

from prepare_slot_metadata import (
    METADATA_COMMIT_MARKER,
    METADATA_RECORD_SIZE,
    METADATA_SECTOR_SIZE,
    create_metadata_record,
)


STATE_VALUES = {
    "EMPTY": 0,
    "PENDING_VALIDATION": 1,
    "VALIDATED": 2,
    "BOOT_PENDING": 3,
    "CONFIRMED": 4,
    "INVALID": 5,
    "ROLLBACK": 6,
}


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Create metadata for a firmware image stored in FOTA staging area."
    )

    parser.add_argument(
        "--firmware",
        required=True,
        type=Path,
        help="Raw firmware .bin that will be written to Sector 4",
    )
    parser.add_argument(
        "--version",
        required=True,
        type=int,
        help="Firmware version",
    )
    parser.add_argument(
        "--sequence",
        required=True,
        type=int,
        help="Metadata sequence number",
    )
    parser.add_argument(
        "--state",
        choices=list(STATE_VALUES.keys()),
        default="PENDING_VALIDATION",
        help="Metadata update state",
    )
    parser.add_argument(
        "--output",
        required=True,
        type=Path,
        help="Output metadata sector binary",
    )

    args = parser.parse_args()

    if not args.firmware.exists():
        raise FileNotFoundError(f"Firmware file not found: {args.firmware}")

    raw_firmware = args.firmware.read_bytes()

    if not raw_firmware:
        raise ValueError("Firmware image is empty.")

    update_state = STATE_VALUES[args.state]

    # IMPORTANT:
    # Hash exactly the bytes contained in the raw .bin.
    firmware_sha = hashlib.sha256(raw_firmware).hexdigest()

    record = create_metadata_record(
        firmware=raw_firmware,
        version=args.version,
        sequence=args.sequence,
        update_state=update_state,
    )

    sector_image = bytearray(b"\xFF" * METADATA_SECTOR_SIZE)
    sector_image[:METADATA_RECORD_SIZE] = record

    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_bytes(sector_image)

    print()
    print("==============================================")
    print(" STM32F407 Staging Metadata Generator")
    print("==============================================")
    print(f"Firmware .bin    : {args.firmware}")
    print(f"Firmware size    : {len(raw_firmware)} bytes")
    print(f"Version          : {args.version}")
    print(f"Sequence         : {args.sequence}")
    print(f"State            : {args.state}")
    print(f"Raw .bin SHA-256 : {firmware_sha}")
    print(f"Metadata output  : {args.output}")
    print(f"Metadata size    : {len(sector_image)} bytes")
    print(f"Commit marker    : 0x{METADATA_COMMIT_MARKER:08X}")
    print("==============================================")
    print()

    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, ValueError) as exc:
        print(f"ERROR: {exc}")
        raise SystemExit(1)
