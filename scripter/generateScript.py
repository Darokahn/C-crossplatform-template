import os
import cv2
import time
import numpy as np

spriteWidth = 48
stageWidth = 512

alignOfImage = 2
alignOfPixel = 1

mode = "desktop"

files = [["stages", file] for file in sorted(os.listdir("assets/stages"))] + [["sprites", file] for file in sorted(os.listdir("assets/sprites"))]
imageBlob = []

def intToBytes(num, length):
    b = bytearray(length)
    for i in range(length):
        b[i] = (num >> (8 * i) & 0xff)
    return b

def compileImages(files):
    for idnum, file in enumerate(files):
        while(len(imageBlob) & (~(alignOfImage - 1)) != len(imageBlob)):
            imageBlob.append(0)
        imageType, filename = file
        img = cv2.imread(f"assets/{imageType}/{filename}", cv2.IMREAD_UNCHANGED)
        hasAlpha = False
        alpha = None
        if (img.shape[2] == 4):
            hasAlpha = True
        height, width = img.shape[:2]
        scale = stageWidth / width
        if (imageType == "sprites"):
            scale = spriteWidth / width
        width, height = (int(width * scale), int(height * scale))
        img = cv2.resize(img, (width, height), interpolation=cv2.INTER_AREA)
        imageBlob.extend(intToBytes(idnum, 2))
        imageBlob.extend(intToBytes(width, 2))
        imageBlob.extend(intToBytes(height, 2))
        name = bytearray(256)
        for i, char in enumerate(bytes(filename, "utf-8")):
            name[i] = char
        imageBlob.extend(name)
        buffer = img.tobytes()
        bufferIter = iter(buffer)
        for i in range(len(buffer) // (3 + hasAlpha)):
            b, g, r = next(bufferIter), next(bufferIter), next(bufferIter)
            a = 255
            if (hasAlpha):
                a = next(bufferIter)
            color = bytearray([r, g, b, a])
            imageBlob.extend(color)
        imageBlob.extend(intToBytes(1, 2))
    imageBlob.extend(intToBytes(-1, 2))
    array = "const unsigned char imageBlob[] = {" + ", ".join((str(l) for l in imageBlob)) + "};"
    return array

CELL = 24

def cell_brightness(cell):
    # Compute brightness from RGB or RGBA
    if cell.shape[2] == 4:
        b,g,r,a = cv2.split(cell)
    else:
        b,g,r = cv2.split(cell)
    # Standard luminance approximation
    return np.mean(0.299*r + 0.587*g + 0.114*b)

def image_to_1bit_cells(path):
    img = cv2.imread(path, cv2.IMREAD_UNCHANGED)

    h, w = img.shape[:2]
    cells_y = h // CELL
    cells_x = w // CELL

    bitmap = np.zeros((cells_y, cells_x), dtype=np.uint8)

    for cy in range(cells_y):
        for cx in range(cells_x):
            y0 = cy * CELL
            x0 = cx * CELL
            cell = img[y0:y0+CELL, x0:x0+CELL]

            brightness = cell_brightness(cell)

            # 1-bit threshold: >50% = filled
            bitmap[cy, cx] = 1 if brightness > 127 else 0

    return bitmap

def compileStage():
    bitmap = image_to_1bit_cells("assets/stage.png")
    points = []

    for y in range(20):
        for x in range(200):
            if bitmap[y, x] == 1:
                points.append((x * 24, y * 24))
    cArray =  "const point_t points[] = {"
    for point in points:
        cArray += "{" + f"{point[0]}, {point[1]}" + "},"
    cArray += "};\n"
    cArray += f"int pointCount = {len(points)};\n"
    return cArray

print("#include \"gameObjects.h\"")
print(compileImages(files))
