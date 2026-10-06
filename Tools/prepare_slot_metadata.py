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
# Firmware image configuration
# ============================================================

# STM32F407 Slot A firmware start address.
# Used only for debug/address reporting.
SLOT_A_IMAGE_START = 0x08020000

# Slot B can be updated later if required.
SLOT_B_IMAGE_START = 0x08040000


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
# Temporary SHA-256 debug
#
# Calculates only the FIRST 512-bit SHA-256 block and prints
# working variables a-h after selected rounds.
#
# This is for debugging only.
# ============================================================

SHA256_K = [
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
    0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
    0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
    0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
    0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
    0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2,
]


def rotr32(x: int, n: int) -> int:
    return (
        (x >> n) |
        (x << (32 - n))
    ) & 0xFFFFFFFF


def sha256_debug_first_block(data: bytes) -> None:

    # --------------------------------------------------------
    # SHA-256 padding
    # --------------------------------------------------------

    padded = bytearray(data)

    bit_length = len(data) * 8

    padded.append(0x80)

    while len(padded) % 64 != 56:
        padded.append(0x00)

    padded += bit_length.to_bytes(
        8,
        byteorder="big"
    )

    # First 512-bit block
    block = padded[0:64]

    # --------------------------------------------------------
    # SHA-256 initial hash values
    # --------------------------------------------------------

    h0 = 0x6A09E667
    h1 = 0xBB67AE85
    h2 = 0x3C6EF372
    h3 = 0xA54FF53A
    h4 = 0x510E527F
    h5 = 0x9B05688C
    h6 = 0x1F83D9AB
    h7 = 0x5BE0CD19

    # --------------------------------------------------------
    # Message schedule W[0..63]
    # SHA-256 uses BIG-ENDIAN 32-bit words
    # --------------------------------------------------------

    w = []

    for i in range(16):
        word = int.from_bytes(
            block[i * 4:(i + 1) * 4],
            byteorder="big"
        )
        w.append(word)

    for i in range(16, 64):

        s0 = (
            rotr32(w[i - 15], 7)
            ^ rotr32(w[i - 15], 18)
            ^ (w[i - 15] >> 3)
        )

        s1 = (
            rotr32(w[i - 2], 17)
            ^ rotr32(w[i - 2], 19)
            ^ (w[i - 2] >> 10)
        )

        w.append(
            (
                w[i - 16]
                + s0
                + w[i - 7]
                + s1
            ) & 0xFFFFFFFF
        )

    # --------------------------------------------------------
    # Working variables
    # --------------------------------------------------------

    a = h0
    b = h1
    c = h2
    d = h3
    e = h4
    f = h5
    g = h6
    h = h7

    # --------------------------------------------------------
    # 64 SHA-256 rounds
    # --------------------------------------------------------

    for i in range(64):

        S1 = (
            rotr32(e, 6)
            ^ rotr32(e, 11)
            ^ rotr32(e, 25)
        )

        ch = (
            (e & f)
            ^ ((~e) & g)
        ) & 0xFFFFFFFF

        temp1 = (
            h
            + S1
            + ch
            + SHA256_K[i]
            + w[i]
        ) & 0xFFFFFFFF

        S0 = (
            rotr32(a, 2)
            ^ rotr32(a, 13)
            ^ rotr32(a, 22)
        )

        maj = (
            (a & b)
            ^ (a & c)
            ^ (b & c)
        ) & 0xFFFFFFFF

        temp2 = (
            S0 + maj
        ) & 0xFFFFFFFF

        h = g
        g = f
        f = e
        e = (d + temp1) & 0xFFFFFFFF
        d = c
        c = b
        b = a
        a = (temp1 + temp2) & 0xFFFFFFFF


    # --------------------------------------------------------
    # DEBUG OUTPUT
    # --------------------------------------------------------

    print()
    print("==============================================")
    print(" Python SHA-256 Debug - After Round 64")
    print(" First 512-bit block")
    print("==============================================")
    print(f"a = 0x{a:08x}")
    print(f"b = 0x{b:08x}")
    print(f"c = 0x{c:08x}")
    print(f"d = 0x{d:08x}")
    print(f"e = 0x{e:08x}")
    print(f"f = 0x{f:08x}")
    print(f"g = 0x{g:08x}")
    print(f"h = 0x{h:08x}")
    print("==============================================")
    print()


# ============================================================
# SHA-256 debug for all blocks
# ============================================================

def sha256_debug_all_blocks(data: bytes) -> bytes:

    original_data = data

    # --------------------------------------------------------
    # SHA-256 padding
    # --------------------------------------------------------

    padded = bytearray(data)

    bit_length = len(data) * 8

    padded.append(0x80)

    while len(padded) % 64 != 56:
        padded.append(0x00)

    padded += bit_length.to_bytes(
        8,
        byteorder="big"
    )

    total_blocks = len(padded) // 64

    # --------------------------------------------------------
    # Initial SHA-256 state
    # --------------------------------------------------------

    h0 = 0x6A09E667
    h1 = 0xBB67AE85
    h2 = 0x3C6EF372
    h3 = 0xA54FF53A
    h4 = 0x510E527F
    h5 = 0x9B05688C
    h6 = 0x1F83D9AB
    h7 = 0x5BE0CD19

    # --------------------------------------------------------
    # Process every 512-bit block
    # --------------------------------------------------------

    for block_number in range(total_blocks):

        block_start = block_number * 64

        block = padded[
            block_start:block_start + 64
        ]

        # ----------------------------------------------------
        # Message schedule W[0..63]
        # ----------------------------------------------------

        w = []

        for i in range(16):
            word = int.from_bytes(
                block[i * 4:(i + 1) * 4],
                byteorder="big"
            )
            w.append(word)

        for i in range(16, 64):

            s0 = (
                rotr32(
                    w[i - 15],
                    7
                )
                ^ rotr32(
                    w[i - 15],
                    18
                )
                ^ (
                    w[i - 15] >> 3
                )
            )

            s1 = (
                rotr32(
                    w[i - 2],
                    17
                )
                ^ rotr32(
                    w[i - 2],
                    19
                )
                ^ (
                    w[i - 2] >> 10
                )
            )

            w.append(
                (
                    w[i - 16]
                    + s0
                    + w[i - 7]
                    + s1
                ) & 0xFFFFFFFF
            )

        # ----------------------------------------------------
        # Working variables
        # ----------------------------------------------------

        a = h0
        b = h1
        c = h2
        d = h3
        e = h4
        f = h5
        g = h6
        h = h7

        # ----------------------------------------------------
        # 64 SHA-256 rounds
        # ----------------------------------------------------

        for i in range(64):

            S1 = (
                rotr32(e, 6)
                ^ rotr32(e, 11)
                ^ rotr32(e, 25)
            )

            ch = (
                (e & f)
                ^ ((~e) & g)
            ) & 0xFFFFFFFF

            temp1 = (
                h
                + S1
                + ch
                + SHA256_K[i]
                + w[i]
            ) & 0xFFFFFFFF

            S0 = (
                rotr32(a, 2)
                ^ rotr32(a, 13)
                ^ rotr32(a, 22)
            )

            maj = (
                (a & b)
                ^ (a & c)
                ^ (b & c)
            ) & 0xFFFFFFFF

            temp2 = (
                S0 + maj
            ) & 0xFFFFFFFF

            h = g
            g = f
            f = e
            e = (d + temp1) & 0xFFFFFFFF
            d = c
            c = b
            b = a
            a = (temp1 + temp2) & 0xFFFFFFFF

        # ----------------------------------------------------
        # Final accumulation for this block
        # ----------------------------------------------------

        h0 = (h0 + a) & 0xFFFFFFFF
        h1 = (h1 + b) & 0xFFFFFFFF
        h2 = (h2 + c) & 0xFFFFFFFF
        h3 = (h3 + d) & 0xFFFFFFFF
        h4 = (h4 + e) & 0xFFFFFFFF
        h5 = (h5 + f) & 0xFFFFFFFF
        h6 = (h6 + g) & 0xFFFFFFFF
        h7 = (h7 + h) & 0xFFFFFFFF

    # --------------------------------------------------------
    # Construct final digest
    # --------------------------------------------------------

    digest = b"".join(
        x.to_bytes(
            4,
            byteorder="big"
        )
        for x in [
            h0,
            h1,
            h2,
            h3,
            h4,
            h5,
            h6,
            h7,
        ]
    )

    # --------------------------------------------------------
    # Compare with hashlib
    # --------------------------------------------------------

    reference = hashlib.sha256(
        original_data
    ).digest()

    print()
    print("==============================================")
    print(" Python SHA-256 Full Debug")
    print("==============================================")
    print(
        f"Firmware size : "
        f"{len(original_data)} bytes"
    )
    print(
        f"Total blocks  : "
        f"{total_blocks}"
    )
    print()
    print(
        f"Debug SHA-256 : "
        f"{digest.hex()}"
    )
    print(
        f"hashlib       : "
        f"{reference.hex()}"
    )

    if digest == reference:
        print("RESULT        : MATCH")
    else:
        print("RESULT        : MISMATCH")

    print("==============================================")
    print()

    return digest


# ============================================================
# Create flash-equivalent firmware image from ELF
#
# The raw .bin produced by objcopy may contain 0x00 bytes in
# linker-generated gaps.
#
# The actual STM32 flash is erased to 0xFF.
#
# Therefore the reference image used for SHA-256 must represent
# the same bytes that exist in physical flash.
#
# GNU objcopy --gap-fill=0xFF handles linker gaps generically.
# ============================================================

def create_flash_image_from_elf(
    elf_path: Path,
    objcopy_path: Path,
    output_path: Path,
) -> bytes:

    if not elf_path.exists():
        raise FileNotFoundError(
            f"ELF file not found: {elf_path}"
        )

    if not objcopy_path.exists():
        raise FileNotFoundError(
            f"objcopy not found: {objcopy_path}"
        )

    command = [
        str(objcopy_path),
        "-O",
        "binary",
        "--gap-fill=0xFF",
        str(elf_path),
        str(output_path),
    ]

    print()
    print(
        "Creating flash-equivalent firmware image..."
    )

    print(
        " ".join(
            f'"{x}"' if " " in x else x
            for x in command
        )
    )

    print()

    result = subprocess.run(
        command,
        check=False,
        capture_output=True,
        text=True,
    )

    if result.returncode != 0:

        stderr = result.stderr.strip()

        raise RuntimeError(
            "objcopy failed while creating "
            "flash-equivalent image. "
            f"{stderr}"
        )

    if not output_path.exists():

        raise RuntimeError(
            "objcopy completed but flash-equivalent "
            f"image was not created: {output_path}"
        )

    flash_image = output_path.read_bytes()

    if len(flash_image) == 0:

        raise ValueError(
            "Generated flash-equivalent firmware "
            "image is empty."
        )

    print(
        "Flash-equivalent image created:"
        f" {output_path}"
    )

    print(
        "Flash-equivalent image size:"
        f" {len(flash_image)} bytes"
    )

    return flash_image


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
        raise ValueError(
            "Firmware image is empty."
        )

    if version == 0:
        raise ValueError(
            "Version must be greater than zero."
        )

    if sequence == 0:
        raise ValueError(
            "Sequence must be greater than zero."
        )

    # --------------------------------------------------------
    # IMPORTANT:
    #
    # firmware is now the flash-equivalent image.
    #
    # SHA is therefore calculated over the same representation
    # that is expected to exist in STM32 flash.
    # --------------------------------------------------------

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

    metadata_crc = stm32_crc32_words(
        record_data
    )

    record = (
        record_data
        + struct.pack(
            "<I",
            metadata_crc
        )
        + struct.pack(
            "<I",
            METADATA_COMMIT_MARKER
        )
    )

    if len(record) != METADATA_RECORD_SIZE:

        raise RuntimeError(
            f"Expected {METADATA_RECORD_SIZE}-byte record, "
            f"got {len(record)}."
        )

    return record


# ============================================================
# Create complete metadata sector
#
# 1. Read the normal build-generated .bin.
# 2. Generate a flash-equivalent image from the ELF.
# 3. Fill linker gaps with 0xFF.
# 4. Use the flash-equivalent image for SHA calculation.
# 5. Preserve the original .bin for comparison/debugging.
# ============================================================

def create_metadata_sector(
    firmware_path: Path,
    elf_path: Path,
    objcopy_path: Path,
    output_path: Path,
    flash_image_output: Path,
    slot: str,
    version: int,
    sequence: int,
    state_name: str,
) -> None:

    # --------------------------------------------------------
    # Read normal .bin
    # --------------------------------------------------------

    raw_firmware = firmware_path.read_bytes()

    if len(raw_firmware) == 0:

        raise ValueError(
            "Firmware image is empty."
        )

    # --------------------------------------------------------
    # Generate flash-equivalent image
    # --------------------------------------------------------

    firmware = create_flash_image_from_elf(
        elf_path=elf_path,
        objcopy_path=objcopy_path,
        output_path=flash_image_output,
    )

    # --------------------------------------------------------
    # Verify image sizes
    # --------------------------------------------------------

    if len(raw_firmware) != len(firmware):

        raise RuntimeError(
            "Firmware size mismatch:\n"
            f"  Raw .bin size       : "
            f"{len(raw_firmware)} bytes\n"
            f"  Flash image size    : "
            f"{len(firmware)} bytes"
        )

    # --------------------------------------------------------
    # Compare raw .bin and flash-equivalent image
    #
    # This is intentionally retained for debugging.
    # --------------------------------------------------------

    if raw_firmware != firmware:

        print()
        print(
            "NOTE: Raw .bin differs from "
            "flash-equivalent image."
        )
        print(
            "This is expected when linker-generated "
            "gaps exist."
        )
        print()

        difference_count = 0

        for offset, (
            raw_byte,
            flash_byte
        ) in enumerate(
            zip(
                raw_firmware,
                firmware
            )
        ):

            if raw_byte != flash_byte:

                difference_count += 1

                if difference_count <= 20:

                    address = (
                        SLOT_A_IMAGE_START
                        + offset
                    )

                    print(
                        f"  Offset "
                        f"0x{offset:08X} "
                        f"(address "
                        f"0x{address:08X}): "
                        f".bin=0x{raw_byte:02X}, "
                        f"flash=0x{flash_byte:02X}"
                    )

        print(
            f"Total differing bytes: "
            f"{difference_count}"
        )
        print()

    else:

        print()
        print(
            "Raw .bin and flash-equivalent "
            "image are identical."
        )
        print()

    # --------------------------------------------------------
    # SHA debugging
    #
    # IMPORTANT:
    # Debug now operates on the flash-equivalent image.
    # --------------------------------------------------------

    sha256_debug_all_blocks(
        firmware
    )

    # sha256_debug_first_block(firmware)

    # --------------------------------------------------------
    # Metadata state
    # --------------------------------------------------------

    state_name = state_name.upper()

    if state_name not in STATE_VALUES:

        raise ValueError(
            f"Invalid state '{state_name}'. "
            f"Valid states: "
            f"{', '.join(STATE_VALUES)}"
        )

    update_state = STATE_VALUES[
        state_name
    ]

    # --------------------------------------------------------
    # Create metadata record
    # --------------------------------------------------------

    record = create_metadata_record(
        firmware=firmware,
        version=version,
        sequence=sequence,
        update_state=update_state,
    )

    # --------------------------------------------------------
    # Entire metadata sector is erased state
    # --------------------------------------------------------

    sector_image = bytearray(
        b"\xFF" * METADATA_SECTOR_SIZE
    )

    # First metadata record starts at sector beginning.

    sector_image[
        0:METADATA_RECORD_SIZE
    ] = record

    output_path.write_bytes(
        sector_image
    )

    # --------------------------------------------------------
    # Final SHA values
    # --------------------------------------------------------

    raw_firmware_sha = hashlib.sha256(
        raw_firmware
    ).hexdigest()

    firmware_sha = hashlib.sha256(
        firmware
    ).hexdigest()

    # --------------------------------------------------------
    # Metadata CRC
    # --------------------------------------------------------

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

    # --------------------------------------------------------
    # Output
    # --------------------------------------------------------

    print()
    print("==============================================")
    print(" STM32F407 Slot Metadata Generator")
    print("==============================================")
    print(
        f"Slot                 : "
        f"{slot}"
    )
    print(
        f"Firmware .bin        : "
        f"{firmware_path}"
    )
    print(
        f"Firmware ELF         : "
        f"{elf_path}"
    )
    print()
    print(
        f"Raw .bin size        : "
        f"{len(raw_firmware)} bytes"
    )
    print(
        f"Flash image size     : "
        f"{len(firmware)} bytes"
    )
    print(
        f"Version              : "
        f"{version}"
    )
    print(
        f"Sequence             : "
        f"{sequence}"
    )
    print(
        f"State                : "
        f"{state_name}"
    )
    print(
        f"Metadata address     : "
        f"0x{metadata_address:08X}"
    )
    print(
        f"Record size          : "
        f"{METADATA_RECORD_SIZE} bytes"
    )
    print()
    print(
        f"Raw .bin SHA-256     : "
        f"{raw_firmware_sha}"
    )
    print(
        f"Flash image SHA-256  : "
        f"{firmware_sha}"
    )
    print()
    print(
        f"Metadata CRC         : "
        f"0x{metadata_crc:08X}"
    )
    print(
        f"Commit marker        : "
        f"0x{METADATA_COMMIT_MARKER:08X}"
    )
    print(
        f"Metadata output size : "
        f"{len(sector_image)} bytes"
    )
    print(
        f"Metadata output      : "
        f"{output_path}"
    )
    print(
        f"Flash image output   : "
        f"{flash_image_output}"
    )
    print(
        "=============================================="
    )
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

    print(
        "Programming metadata sector..."
    )

    print(
        " ".join(
            f'"{x}"' if " " in x else x
            for x in command
        )
    )

    print()

    result = subprocess.run(
        command,
        check=False,
    )

    if result.returncode != 0:

        raise RuntimeError(
            "STM32CubeProgrammer failed "
            f"with exit code "
            f"{result.returncode}."
        )

    print(
        "Metadata programming completed."
    )

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
        "--elf",
        required=True,
        type=Path,
        help=(
            "Firmware ELF file used to generate "
            "the flash-equivalent image"
        ),
    )

    parser.add_argument(
        "--objcopy",
        required=True,
        type=Path,
        help=(
            "Path to arm-none-eabi-objcopy.exe"
        ),
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
        "--flash-image-output",
        type=Path,
        help=(
            "Output path for flash-equivalent "
            "firmware image"
        ),
    )

    parser.add_argument(
        "--program",
        action="store_true",
        help=(
            "Program metadata into STM32 flash"
        ),
    )

    parser.add_argument(
        "--cubeprogrammer",
        type=Path,
        help=(
            "Path to STM32_Programmer_CLI.exe"
        ),
    )

    args = parser.parse_args()

    try:

        # ----------------------------------------------------
        # Validate firmware
        # ----------------------------------------------------

        if not args.firmware.exists():

            raise FileNotFoundError(
                f"Firmware file not found: "
                f"{args.firmware}"
            )

        # ----------------------------------------------------
        # Validate ELF
        # ----------------------------------------------------

        if not args.elf.exists():

            raise FileNotFoundError(
                f"ELF file not found: "
                f"{args.elf}"
            )

        # ----------------------------------------------------
        # Validate objcopy
        # ----------------------------------------------------

        if not args.objcopy.exists():

            raise FileNotFoundError(
                f"objcopy not found: "
                f"{args.objcopy}"
            )

        # ----------------------------------------------------
        # Default metadata output
        # ----------------------------------------------------

        if args.output is None:

            args.output = Path(
                f"Slot_{args.slot}_Metadata.bin"
            )

        # ----------------------------------------------------
        # Default flash-equivalent image output
        # ----------------------------------------------------

        if args.flash_image_output is None:

            args.flash_image_output = (
                args.firmware.parent
                / (
                    f"{args.firmware.stem}"
                    f"_FlashReference.bin"
                )
            )

        # ----------------------------------------------------
        # Create metadata
        # ----------------------------------------------------

        create_metadata_sector(
            firmware_path=args.firmware,
            elf_path=args.elf,
            objcopy_path=args.objcopy,
            output_path=args.output,
            flash_image_output=args.flash_image_output,
            slot=args.slot,
            version=args.version,
            sequence=args.sequence,
            state_name=args.state,
        )

        # ----------------------------------------------------
        # Optional metadata programming
        # ----------------------------------------------------

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

        print(
            f"ERROR: {exc}"
        )

        return 1

    return 0


if __name__ == "__main__":
    raise SystemExit(
        main()
    )