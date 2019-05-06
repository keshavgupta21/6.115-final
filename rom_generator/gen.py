from pathlib import Path
import sys
rom = Path(sys.argv[1]).read_bytes()

with open("rom.csv", "w") as f:
    f.write("Version, 1\n")
    for i in range(0, len(rom), 16):
        strs = []
        for j in range(min(16, len(rom) - i)):
            byte = rom[i + j]
            strs.append("0x{0}".format(hex(byte).replace("x", "0").upper()[-2:]))
        f.write(", ".join(strs) + "\n")
