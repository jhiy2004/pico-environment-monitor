from PIL import Image
import sys
from pathlib import Path
from dotenv import load_dotenv
import os

OLED_HEIGHT = 64
OLED_WIDTH = 128
OLED_PAGE_HEIGHT = 8

FONT_WIDTH = 3
FONT_HEIGHT = 5

if len(sys.argv) < 2:
    print("No image name provided.")
    sys.exit()


load_dotenv()

img_path = Path(os.getenv('IMAGE_PATH'))

try:
    im = Image.open(img_path / sys.argv[1])
except OSError:
    raise Exception("Oops! The image could not be opened.")

img_width = im.size[0]
img_height = im.size[1]

# black or white
out = im.convert("1")

img_name = Path(im.filename).stem

# `pixels` is a flattened array with the top left pixel at index 0
# and bottom right pixel at the width*height-1
pixels = list(out.getdata())

# swap white for black and swap (255, 0) for (1, 0)
pixels = [0 if x == 255 else 1 for x in pixels]


fonts_start_index = [(x, y) for y in range(0, img_height, FONT_HEIGHT) for x in range(0, img_width, FONT_WIDTH)]
print(fonts_start_index)

buffer = []
for x, y in fonts_start_index:
    for j in range(x, x+FONT_WIDTH):
        out_byte = 0
        for k in range(FONT_HEIGHT):
            out_byte |= pixels[(y+k)*img_width + j] << k

        buffer.append(f'{out_byte:#04x}')

print(buffer)

buffer_str = "\n"
for i in range(10):
    buffer_str += 4 * " " + f"//Number {i}:\n"
    buffer_str += 4 * " "
    for k in range(FONT_WIDTH): # 0 .. 3
        index = (FONT_WIDTH * i) + k
        buffer_str += f"{buffer[index]}, "
    buffer_str += "\n"

buffer_hex = f'static uint8_t {img_name}[] = {{{buffer_str}}};\n'

with open(f'small_numbers.h', 'wt') as file:
    file.write(buffer_hex)

