import serial, sys
from pathlib import Path

try:
    rom = Path(sys.argv[1]).read_bytes()

    with serial.Serial('/dev/ttyS8', 57600, timeout=1) as ser:
        ser.write([0, 1])
        tbuf = ser.read(2)
        assert(list(tbuf) == [2, 3])
        print("Connection Established...")
        n = len(rom)
        ser.write([n//255, n%255])
        tbuf = ser.read(2)
        assert(list(tbuf) == [n//255, n%255])
        print("Size verified...")
        for i in range(len(rom)):
            print(str(int(100*i/len(rom))) + "% transmitted..")
            ser.write(rom[1])
        tbuf = ser.read(2)
        assert(list(tbuf) == [4, 5])
        print("Transmission Complete!")
except Exception as e:
    print(e)
    print("ROM Transmission Failed :( Reset Target and try again!")
    exit(1)
print("ROM Transmitted Successfully!")
    