from PIL import Image
import sys
from pathlib import Path
from dotenv import load_dotenv
import os

OLED_HEIGHT = 64
OLED_WIDTH = 128
OLED_PAGE_HEIGHT = 8

ICON_WIDTH = 4
ICON_HEIGHT = 3

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


icons_start_index = [(x, y) for y in range(0, img_height, ICON_HEIGHT) for x in range(0, img_width, ICON_WIDTH)]
print(icons_start_index)

buffer = []
for x, y in icons_start_index:
    for j in range(x, x+ICON_WIDTH):
        out_byte = 0
        for k in range(ICON_HEIGHT):
            out_byte |= pixels[(y+k)*img_width + j] << k

        buffer.append(f'{out_byte:#04x}')

print(buffer)

icons_names = ["ON", "OFF"]
buffer_str = "\n"
for i, name in enumerate(icons_names):
    buffer_str += 4 * " " + f"//ICON {name}:\n"
    buffer_str += 4 * " "
    for k in range(ICON_WIDTH): # 0 .. 2
        index = (ICON_WIDTH * i) + k
        buffer_str += f"{buffer[index]}, "
    buffer_str += "\n"

buffer_hex = f'static uint8_t {img_name}[] = {{{buffer_str}}};\n'

with open(f'on_off_icons.h', 'wt') as file:
    file.write(buffer_hex)

