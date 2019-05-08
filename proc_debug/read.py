import sys
from pathlib import Path
data = Path(sys.argv[1]).read_bytes()  # Python 3.5+

for byte in data:
    print("0x{0}, ".format(hex(byte).replace("x", "0")[-2:]), end = "")