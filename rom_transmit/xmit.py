import serial, sys
import struct
from pathlib import Path

def send_byte(ser, ch_s):
    byte = struct.pack(">B", ch_s)
    ser.write(byte)
    resp = ser.read()
    ch_r = struct.unpack(">B", resp)
    assert len(ch_r) == 1
    assert ch_r[0] == (ch_s + 1) & 0xff

if __name__ == "__main__":
    data = [0xf0, 0x90, 0x90, 0x90, 0xf0, 0x20, 0x60, 0x20,\
            0x20, 0x70, 0xf0, 0x10, 0xf0, 0x80, 0xf0, 0xf0,\
            0x10, 0xf0, 0x10, 0xf0, 0x90, 0x90, 0xf0, 0x10,\
            0x10, 0xf0, 0x80, 0xf0, 0x10, 0xf0, 0xf0, 0x80,\
            0xf0, 0x90, 0xf0, 0xf0, 0x10, 0x20, 0x40, 0x40,\
            0xf0, 0x90, 0xf0, 0x90, 0xf0, 0xf0, 0x90, 0xf0,\
            0x10, 0xf0, 0xf0, 0x90, 0xf0, 0x90, 0x90, 0xe0,\
            0x90, 0xe0, 0x90, 0xe0, 0xf0, 0x80, 0x80, 0x80,\
            0xf0, 0xe0, 0x90, 0x90, 0x90, 0xe0, 0xf0, 0x80,\
            0xf0, 0x80, 0xf0, 0xf0, 0x80, 0xf0, 0x80, 0x80]
            
    while len(data) < 0x200:
        data.append(0x00)

    rom = Path(sys.argv[2]).read_bytes()
    data += rom

    magic_start = ord('S') & 0xff
    size_hb = (len(data) & 0xff00) >> 8
    size_lb = len(data) & 0xff
    magic_end = ord('E') & 0xff

    try:
        with serial.Serial(sys.argv[1], 9600, timeout=1) as ser:
            print("Connection Established...")
            send_byte(ser, magic_start)
            send_byte(ser, size_hb)
            send_byte(ser, size_lb)
            for byte in data:
                send_byte(ser, byte)
            send_byte(ser, magic_end)
            print("Successful Transmission")
    except:
        print("Failed Transmission")
    

    
