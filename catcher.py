import serial
import sys
import time

PORT = 'socket://192.168.6.236:8888' # <-- Change to your ser2net IP/Port
BAUD = 115200
BLOCK_SIZE = 4096
TOTAL_BLOCKS = 1024 # 1024 * 4096 = 4MB

def calculate_crc16(data: bytes) -> int:
    crc = 0xFFFF
    for byte in data:
        crc ^= (byte << 8)
        for _ in range(8):
            if crc & 0x8000:
                crc = ((crc << 1) ^ 0x1021) & 0xFFFF
            else:
                crc = (crc << 1) & 0xFFFF
    return crc

print(f"Connecting to {PORT}...")

try:
    with serial.serial_for_url(PORT, baudrate=BAUD, timeout=5) as ser:
        ser.reset_input_buffer()
        ser.reset_output_buffer()

        # Phase 1: Handshake
        board_ready = False
        print("Synchronizing...")
        while not board_ready:
            ser.write(b"HELLO_DUMPER\n")
            response = ser.readline().decode(errors='ignore').strip()
            if "ACK_READY" in response:
                print("Handshake successful!")
                board_ready = True
            else:
                time.sleep(0.5)

        # Phase 2: Start Dump
        ser.write(b"START_DUMP\n")

        dump_data = bytearray()
        start_time = time.time()
        retries = 0

        print(f"Downloading 4MB with CRC16 Verification...\n")

        # Phase 3: Receive 1024 Blocks
        for block in range(TOTAL_BLOCKS):
            block_accepted = False

            while not block_accepted:
                # We expect exactly 4096 bytes of data + 2 bytes of CRC
                raw_chunk = ser.read(BLOCK_SIZE + 2)

                if len(raw_chunk) != (BLOCK_SIZE + 2):
                    print(f"\n[!] Network timeout or byte drop on Block {block}. Sending NACK...")
                    ser.reset_input_buffer()
                    ser.write(b"NACK\n")
                    retries += 1
                    continue

                data = raw_chunk[:BLOCK_SIZE]
                received_crc = (raw_chunk[BLOCK_SIZE] << 8) | raw_chunk[BLOCK_SIZE + 1]

                calculated_crc = calculate_crc16(data)

                if received_crc == calculated_crc:
                    ser.write(b"ACK\n")
                    dump_data.extend(data)
                    block_accepted = True
                else:
                    print(f"\n[!] CRC Mismatch on Block {block} (Got {hex(received_crc)}, Expected {hex(calculated_crc)}). Sending NACK...")
                    ser.reset_input_buffer()
                    ser.write(b"NACK\n")
                    retries += 1

            # Print progress
            received_bytes = (block + 1) * BLOCK_SIZE
            percent = (received_bytes / (TOTAL_BLOCKS * BLOCK_SIZE)) * 100
            sys.stdout.write(f"\rReceived: {received_bytes:,} / 4,194,304 bytes [{percent:.1f}%] (Retries: {retries})")
            sys.stdout.flush()

        elapsed = time.time() - start_time
        print(f"\n\nTransfer complete in {elapsed:.1f} seconds!")

        if retries > 0:
            print(f"Protocol successfully caught and fixed {retries} corrupted network packets.")

        with open("hazk03_flash_crc.bin", "wb") as f:
            f.write(dump_data)

        print("Flawless 4MB dump saved to hazk03_flash_crc.bin.")

except Exception as e:
    print(f"\nFatal Error: {e}")
