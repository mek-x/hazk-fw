import zlib

# Change this to match whatever you named your dump file!
FILENAME = "hazk03_flash_crc.bin"

print(f"Opening {FILENAME} for verification...")

try:
    with open(FILENAME, "rb") as f:
        data = f.read()

    if len(data) != 4194304:
        print(f"WARNING: File size is {len(data)} bytes. Expected exactly 4,194,304 bytes (4MB).")

    # Calculate CRC32 using zlib (matches standard IEEE 802.3)
    # The & 0xFFFFFFFF ensures it remains an unsigned 32-bit integer across all Python versions
    file_crc = zlib.crc32(data) & 0xFFFFFFFF

    print(f"\nLocal File CRC32: 0x{file_crc:08X}")
    print("\nCompare this output to the Arduino Serial Monitor!")

except FileNotFoundError:
    print(f"Error: Could not find '{FILENAME}'.")
except Exception as e:
    print(f"Error: {e}")
