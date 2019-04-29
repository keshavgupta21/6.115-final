from pathlib import Path
import sys
rom = Path(sys.argv[1]).read_bytes()  # Python 3.5+

font = [0xF0, 0x90, 0x90, 0x90, 0xF0,\
        0x20, 0x60, 0x20, 0x20, 0x70,\
        0xF0, 0x10, 0xF0, 0x80, 0xF0,\
        0xF0, 0x10, 0xF0, 0x10, 0xF0,\
        0x90, 0x90, 0xF0, 0x10, 0x10,\
        0xF0, 0x80, 0xF0, 0x10, 0xF0,\
        0xF0, 0x80, 0xF0, 0x90, 0xF0,\
        0xF0, 0x10, 0x20, 0x40, 0x40,\
        0xF0, 0x90, 0xF0, 0x90, 0xF0,\
        0xF0, 0x90, 0xF0, 0x10, 0xF0,\
        0xF0, 0x90, 0xF0, 0x90, 0x90,\
        0xE0, 0x90, 0xE0, 0x90, 0xE0,\
        0xF0, 0x80, 0x80, 0x80, 0xF0,\
        0xE0, 0x90, 0x90, 0x90, 0xE0,\
        0xF0, 0x80, 0xF0, 0x80, 0xF0,\
        0xF0, 0x80, 0xF0, 0x80, 0x80]

data = font
while len(data) < 0x200:
    data.append(0x00)

data += rom

with open("ram_init_data.h", "w") as f:
    f.write("#define RAM_INITIAL_DATA {\\\n")
    for i in range(0, len(data), 8):
        strs = []
        for j in range(min(8, len(data) - i)):
            byte = data[i + j]
            strs.append("0x{0}".format(hex(byte).replace("x", "0")[-2:]))
        f.write("    " + ", ".join(strs) + ",\\\n")
    f.write("}\n")

    