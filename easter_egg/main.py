from PIL import Image

img = Image.open("nggyunglud.png")
pixs = list(img.getdata())

def getpix(x, y):
    return 0 if pixs[x * 200 + y][0] > 127 else 1
    
x1 = 16
y1 = 16
x2 = 184
y2 = 184

qr = [0]
for i in range(29):
    row = 0
    for j in range(29):
        pix = getpix(16 + int((184.0-16.0)*i/28), 16 + int((184.0-16.0)*j/28))
        row = 2*row + pix            
    qr.append(row)
qr.append(0)
qr.append(0)
        
for i in range(len(qr)):
    qr[i] *= 2**17

print(",\n".join([hex(t) for t in qr]))
    
