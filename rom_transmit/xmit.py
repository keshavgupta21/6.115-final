import serial, sys
from pathlib import Path

rom = Path(sys.argv[1]).read_bytes()
with serial.Serial('/dev/ttyS6', 57600, timeout=1) as ser:
    print("Connection Established...")
    ser.write([0, 1])
    n = len(rom)
    size_2ple = [n//256, n%256]
    ser.write(size_2ple)
    ser.write(rom)
    tbuf = ser.read(2)
    if(list(tbuf) == size_2ple):
        ser.write([0x04])
        print("Transmission Complete!")
        print("ROM Transmitted Successfully!")
    else:
        ser.write([0x00])
        print("ROM Transmission Failed :( Reset Target and try again!")

    
