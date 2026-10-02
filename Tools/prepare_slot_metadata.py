import argparse
import hashlib
import struct
import subprocess
from pathlib import Path


# ============================================================
# STM32F407 metadata sector configuration
# ============================================================

METADATA_SECTOR_SIZE = 0x4000       # 16 KB

SLOT_A_METADATA_ADDRESS = 0x08008000
SLOT_B_METADATA_ADDRESS = 0x0800C000

METADATA_RECORD_SIZE = 60


# ============================================================
# Metadata definitions
# Must match metadata.h
# ============================================================

FIRMWARE_METADATA_MAGIC = 0xDEADBEEF
METADATA_COMMIT_MARKER = 0xA5A55A5A


STATE_VALUES = {
    "EMPTY": 0,
    "PENDING_VALIDATION": 1,
    "VALIDATED": 2,
    "BOOT_PENDING": 3,
    "CONFIRMED": 4,
    "INVALID": 5,
    "ROLLBACK": 6,
}


# ============================================================
# STM32F4 CRC peripheral
#
# Polynomial : 0x04C11DB7
# Initial    : 0xFFFFFFFF
# Input      : 32-bit words
# ============================================================

CRC_POLYNOMIAL = 0x04C11DB7
CRC_INITIAL = 0xFFFFFFFF


def stm32_crc32_words(data: bytes) -> int:
    """
    Calculate the CRC produced by the STM32F4 CRC peripheral
    for 32-bit word input.

    The C implementation calculates CRC over:
        sequence + firmware_metadata_t

    which is 52 bytes = 13 words.
    """

    if len(data) % 4 != 0:
        raise ValueError(
            "CRC input length must be a multiple of 4 bytes."
        )

    crc = CRC_INITIAL

    for offset in range(0, len(data), 4):

        # Same uint32_t interpretation used by the C code.
        word = struct.unpack_from(
            "<I",
            data,
            offset
        )[0]

        crc ^= word

        for _ in range(32):

            if crc & 0x80000000:
                crc = (
                    (crc << 1) ^ CRC_POLYNOMIAL
                ) & 0xFFFFFFFF
            else:
                crc = (
                    crc << 1
                ) & 0xFFFFFFFF

    return crc


# ============================================================
# Metadata record
#
# firmware_metadata_t = 48 bytes
#
# sequence               4
# magic                  4
# image_size             4
# version                4
# sha256                32
# update_state           4
# -------------------------
#                       52
#
# metadata_crc            4
# commit_marker           4
# -------------------------
#                       60
# ============================================================

def create_metadata_record(
    firmware: bytes,
    version: int,
    sequence: int,
    update_state: int,
) -> bytes:

    image_size = len(firmware)

    if image_size == 0:
        raise ValueError("Firmware image is empty.")

    if version == 0:
        raise ValueError("Version must be greater than zero.")

    if sequence == 0:
        raise ValueError("Sequence must be greater than zero.")

    sha256_digest = hashlib.sha256(
        firmware
    ).digest()

    # sequence + firmware_metadata_t
    record_data = struct.pack(
        "<IIII32sI",
        sequence,
        FIRMWARE_METADATA_MAGIC,
        image_size,
        version,
        sha256_digest,
        update_state,
    )

    if len(record_data) != 52:
        raise RuntimeError(
            f"Expected 52 bytes before CRC, "
            f"got {len(record_data)}."
        )

    metadata_crc = stm32_crc32_words(record_data)

    record = (
        record_data
        + struct.pack("<I", metadata_crc)
        + struct.pack("<I", METADATA_COMMIT_MARKER)
    )

    if len(record) != METADATA_RECORD_SIZE:
        raise RuntimeError(
            f"Expected {METADATA_RECORD_SIZE}-byte record, "
            f"got {len(record)}."
        )

    return record


# ============================================================
# Create complete metadata sector
# ============================================================

def create_metadata_sector(
    firmware_path: Path,
    output_path: Path,
    slot: str,
    version: int,
    sequence: int,
    state_name: str,
) -> None:

    firmware = firmware_path.read_bytes()

    state_name = state_name.upper()

    if state_name not in STATE_VALUES:
        raise ValueError(
            f"Invalid state '{state_name}'. "
            f"Valid states: {', '.join(STATE_VALUES)}"
        )

    update_state = STATE_VALUES[state_name]

    record = create_metadata_record(
        firmware=firmware,
        version=version,
        sequence=sequence,
        update_state=update_state,
    )

    # Entire metadata sector is erased state.
    sector_image = bytearray(
        b"\xFF" * METADATA_SECTOR_SIZE
    )

    # First metadata record starts at sector beginning.
    sector_image[
        0:METADATA_RECORD_SIZE
    ] = record

    output_path.write_bytes(sector_image)

    firmware_sha = hashlib.sha256(
        firmware
    ).hexdigest()

    metadata_crc = struct.unpack_from(
        "<I",
        record,
        52
    )[0]

    metadata_address = (
        SLOT_A_METADATA_ADDRESS
        if slot == "A"
        else SLOT_B_METADATA_ADDRESS
    )

    print()
    print("==============================================")
    print(" STM32F407 Slot Metadata Generator")
    print("==============================================")
    print(f"Slot                 : {slot}")
    print(f"Firmware             : {firmware_path}")
    print(f"Firmware size        : {len(firmware)} bytes")
    print(f"Version              : {version}")
    print(f"Sequence             : {sequence}")
    print(f"State                : {state_name}")
    print(f"Metadata address     : 0x{metadata_address:08X}")
    print(f"Record size          : {METADATA_RECORD_SIZE} bytes")
    print(f"SHA-256              : {firmware_sha}")
    print(f"Metadata CRC         : 0x{metadata_crc:08X}")
    print(
        f"Commit marker        : "
        f"0x{METADATA_COMMIT_MARKER:08X}"
    )
    print(f"Output size          : {len(sector_image)} bytes")
    print(f"Output               : {output_path}")
    print("==============================================")
    print()


# ============================================================
# Program metadata sector using STM32CubeProgrammer
# ============================================================

def program_metadata(
    cube_programmer: Path,
    metadata_file: Path,
    slot: str,
) -> None:

    metadata_address = (
        SLOT_A_METADATA_ADDRESS
        if slot == "A"
        else SLOT_B_METADATA_ADDRESS
    )

    command = [
        str(cube_programmer),
        "-c",
        "port=SWD",
        "-w",
        str(metadata_file),
        f"0x{metadata_address:08X}",
        "-v",
    ]

    print("Programming metadata sector...")
    print(" ".join(f'"{x}"' if " " in x else x for x in command))
    print()

    result = subprocess.run(
        command,
        check=False,
    )

    if result.returncode != 0:
        raise RuntimeError(
            f"STM32CubeProgrammer failed "
            f"with exit code {result.returncode}."
        )

    print("Metadata programming completed.")
    print()


# ============================================================
# Main
# ============================================================

def main() -> int:

    parser = argparse.ArgumentParser(
        description=(
            "Generate metadata for STM32F407 Slot A/B "
            "and optionally program it using ST-LINK."
        )
    )

    parser.add_argument(
        "--slot",
        required=True,
        choices=["A", "B"],
        help="Firmware slot",
    )

    parser.add_argument(
        "--firmware",
        required=True,
        type=Path,
        help="Firmware .bin file",
    )

    parser.add_argument(
        "--version",
        required=True,
        type=int,
        help="Firmware version",
    )

    parser.add_argument(
        "--sequence",
        type=int,
        default=1,
        help="Metadata sequence number",
    )

    parser.add_argument(
        "--state",
        choices=list(STATE_VALUES.keys()),
        default="CONFIRMED",
        help="Metadata update state",
    )

    parser.add_argument(
        "--output",
        type=Path,
        help="Output metadata sector binary",
    )

    parser.add_argument(
        "--program",
        action="store_true",
        help="Program metadata into STM32 flash",
    )

    parser.add_argument(
        "--cubeprogrammer",
        type=Path,
        help="Path to STM32_Programmer_CLI.exe",
    )

    args = parser.parse_args()

    try:

        if not args.firmware.exists():
            raise FileNotFoundError(
                f"Firmware file not found: {args.firmware}"
            )

        if args.output is None:

            args.output = Path(
                f"Slot_{args.slot}_Metadata.bin"
            )

        create_metadata_sector(
            firmware_path=args.firmware,
            output_path=args.output,
            slot=args.slot,
            version=args.version,
            sequence=args.sequence,
            state_name=args.state,
        )

        if args.program:

            if args.cubeprogrammer is None:
                raise ValueError(
                    "--cubeprogrammer is required "
                    "when --program is used."
                )

            if not args.cubeprogrammer.exists():
                raise FileNotFoundError(
                    "STM32CubeProgrammer CLI not found: "
                    f"{args.cubeprogrammer}"
                )

            program_metadata(
                cube_programmer=args.cubeprogrammer,
                metadata_file=args.output,
                slot=args.slot,
            )

    except (
        OSError,
        ValueError,
        RuntimeError,
        struct.error,
    ) as exc:

        print(f"ERROR: {exc}")
        return 1

    return 0


if __name__ == "__main__":
    raise SystemExit(main())