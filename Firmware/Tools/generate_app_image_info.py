import subprocess
import sys
from pathlib import Path

def normalize_path(path_string):
    """
    Convert a Cygwin-style path to a Windows path when required.
    Leave normal Windows paths unchanged.
    """
    if path_string.startswith("/cygdrive/"):
        parts = path_string.split("/", 3)

        if len(parts) == 4:
            drive = parts[2].upper()
            path = parts[3].replace("/", "\\")
            return Path(f"{drive}:\\{path}")

    return Path(path_string)
    

def main():
    if len(sys.argv) != 3:
        print("Usage: generate_app_image_info.py <elf> <output_header>")
        return 1

    elf_path = normalize_path(sys.argv[1])
    header_path = normalize_path(sys.argv[2])

    if not elf_path.is_file():
        print(f"ERROR: ELF file not found: {elf_path}")
        return 1

    nm_exe = Path(
    "/cygdrive/c/ST/STM32CubeIDE_1.19.0/STM32CubeIDE/plugins/"
    "com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32."
    "13.3.rel1.win32_1.0.0.202411081344/tools/bin/arm-none-eabi-nm.exe"
    )

    if not nm_exe.is_file():
        print(f"ERROR: arm-none-eabi-nm.exe not found: {nm_exe}")
        return 1

    try:
        result = subprocess.run(
            [str(nm_exe), "--defined-only", str(elf_path)],
            capture_output=True,
            text=True,
            check=True
        )
    except subprocess.CalledProcessError as exc:
        print("ERROR: nm failed.")
        print(exc.stderr)
        return 1

    symbols = {}

    for line in result.stdout.splitlines():
        parts = line.split()

        if len(parts) >= 3 and parts[2] in {
            "_app_image_start",
            "_app_image_end",
        }:
            symbols[parts[2]] = int(parts[0], 16)

    if "_app_image_start" not in symbols:
        print("ERROR: _app_image_start not found.")
        return 1

    if "_app_image_end" not in symbols:
        print("ERROR: _app_image_end not found.")
        return 1

    image_start = symbols["_app_image_start"]
    image_end = symbols["_app_image_end"]

    if image_end <= image_start:
        print("ERROR: Invalid application image boundaries.")
        return 1

    image_size = image_end - image_start

    header_path.parent.mkdir(parents=True, exist_ok=True)

    header_content = f"""#ifndef APP_IMAGE_INFO_H
#define APP_IMAGE_INFO_H

#define APP_IMAGE_START  0x{image_start:08X}U
#define APP_IMAGE_END    0x{image_end:08X}U
#define APP_IMAGE_SIZE   (APP_IMAGE_END - APP_IMAGE_START)

#endif
"""

    header_path.write_text(header_content, encoding="utf-8")

    print(f"Application image start : 0x{image_start:08X}")
    print(f"Application image end   : 0x{image_end:08X}")
    print(f"Application image size  : {image_size} bytes")
    print(f"Generated               : {header_path}")

    return 0


if __name__ == "__main__":
    sys.exit(main())