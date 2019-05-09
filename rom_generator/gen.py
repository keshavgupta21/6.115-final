from pathlib import Path
import sys
rom = Path(sys.argv[1]).read_bytes()

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
    
data += rom

with open("rom.csv", "w") as f:
    f.write("Version, 1\n")
    for i in range(0, len(data), 16):
        strs = []
        for j in range(min(16, len(data) - i)):
            byte = data[i + j]
            strs.append("0x{0}".format(hex(byte).replace("x", "0").upper()[-2:]))
        f.write(", ".join(strs) + "\n")
