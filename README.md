# ePaper Display — Setup & Customisation Guide

- Seeed XIAO ESP32S3 
- Waveshare 2.13" ePaper V4
- ePaper Breakout Board for XIAO

---

## What You Have

The device already works out of the box. This guide is here in case you ever want to change the message, update the quotes, swap the team photo, or set it up again on a new board.

| File | What it does |
|---|---|
| `stephen_farewell.ino` | Main program — controls timing, screens, animation |
| `team_bitmap.h` | The team photo stored as pixel data |
| `quotes.h` | All 130 data science quotes |

---

## What You Need to Install


### 1. Arduino IDE

Download and install from: **https://www.arduino.cc/en/software**

Choose the version for your operating system (Windows / Mac / Linux).

---

### 2. Board Support — Seeed XIAO ESP32S3

This tells Arduino IDE how to talk to your specific board.

1. Open Arduino IDE
2. Go to **Arduino IDe → Settings**
3. Find **"Additional boards manager URLs"** and paste in:
   ```
   https://files.seeedstudio.com/arduino/package_seeeduino_boards_index.json
   ```
4. Click **OK**
5. Go to **Tools → Board → Boards Manager**
6. Search for **Seeed XIAO ESP32S3** and click **Install**

---

### 3. Two Arduino Libraries

1. Go to **Tools → Manage Libraries**
2. Search for **GxEPD2** — install the one by **ZinggJM**
3. Search for **Adafruit GFX Library** — install it

> **Tip:** When installing GxEPD2, Arduino IDE may ask if you want to install dependencies. Click **"Install All"** to include Adafruit GFX automatically.

---

## Uploading to the Board

Follow these steps whenever you want to upload new or edited code.

### Step 1 — Open the project

Open the folder called `stephen_farewell`. Inside you will see:

```
stephen_farewell/
├── stephen_farewell.ino   ← open this one
├── team_bitmap.h          (must stay in the same folder)
└── quotes.h               (must stay in the same folder)
```

Double-click `stephen_farewell.ino` and it will open in Arduino IDE.

### Step 2 — Select the board

Go to **Tools → Board → Seeed XIAO ESP32S3**

### Step 3 — Select the port

1. Plug the XIAO board into your computer via USB-C
2. Go to **Tools → Port**
3. Select the port that appears — on Windows it will say `COM` followed by a number, on Mac it starts with `/dev/cu.`

### Step 4 — Upload

1. Click the **→ Upload** button (the arrow at the top left of Arduino IDE)
2. Wait for the progress bar at the bottom to complete (~30 seconds)
3. The display starts immediately once upload is done

> **If the upload fails:** Hold the **BOOT** button on the XIAO board while clicking Upload. Release BOOT once you see `Connecting...` in the log. This forces the board into upload mode.

---

## Customising the Display

### A — Change the farewell message

Open `stephen_farewell.ino` and scroll to the function called `showFarewellMessage()`.

You will see lines like this:

```cpp
display.print("Thank you for");
display.print("everything, and best");
display.print("of luck in your new");
display.print("journey ahead!");

display.print("- By Akash");
```

Change the text inside the quotes to whatever you want. Keep each line to about **20 characters** — the display is narrow and longer lines will run off the edge.

> **After any change, re-upload using the steps above.**

---

### B — Add, remove, or edit quotes

Open `quotes.h`. It contains an array like this:

```cpp
const char* quotes[] = {
  "Data is the new oil. - Clive Humby",
  "Without data, you are just another person with an opinion. - W. Edwards Deming",
  // ...
};
```

Each quote is a string in double quotes followed by a comma. The author is separated from the quote using ` - ` (space, dash, space) — the display uses this to show the author name separately at the bottom.

- **To add a quote:** paste a new line inside the array
  ```cpp
  "Your quote here. - Author Name",   // ← comma is important
  ```
- **To remove a quote:** delete its line entirely
- **To edit a quote:** change the text between the double quotes

> The code counts quotes automatically — you can have as many or as few as you like.

---

### C — Replace the team photo

The photo needs to be converted to a bitmap format before the display can show it.

#### Step 1 — Install Python

Download from **https://www.python.org/downloads** and install.
On Windows, tick **"Add Python to PATH"** during install.

#### Step 2 — Install the Pillow library

Open a terminal (Command Prompt on Windows, Terminal on Mac) and run:

```bash
pip install Pillow
```

#### Step 3 — Run the conversion script

Save the script below as `convert_photo.py` in the same folder as your photo, then run it with `python convert_photo.py`.

```python
from PIL import Image, ImageEnhance

img = Image.open('your_photo.png')           # <-- change this filename
img = img.resize((250, 122), Image.LANCZOS)
img = ImageEnhance.Contrast(img.convert('L')).enhance(2.5)
img_bw = img.convert('1', dither=Image.FLOYDSTEINBERG)

pixels = list(img_bw.getdata())
byte_array = []
for y in range(122):
    for x in range(0, 250, 8):
        byte = 0
        for bit in range(8):
            if x + bit < 250:
                if not pixels[y * 250 + x + bit]:
                    byte |= (1 << (7 - bit))
        byte_array.append(byte)

with open('team_bitmap.h', 'w') as f:
    f.write('// Team photo 250x122\n')
    f.write('#define TEAM_WIDTH  250\n')
    f.write('#define TEAM_HEIGHT 122\n')
    f.write('static const unsigned char team_bitmap[] PROGMEM = {\n')
    row = (250 + 7) // 8
    for i, b in enumerate(byte_array):
        if i % row == 0: f.write('  ')
        f.write(f'0x{b:02X},')
        if (i + 1) % row == 0: f.write('\n')
    f.write('};\n')

print('Done! Replace team_bitmap.h in your project folder.')
```

Replace the old `team_bitmap.h` in your project folder with the newly generated one, then re-upload.

> **Photo tips:** Use a well-lit, high-contrast image. PNG or JPG both work. The script converts to black & white automatically.

---

### D — Change how long each screen is shown

At the top of `stephen_farewell.ino`:

```cpp
#define PHOTO_DURATION    10000   // Team photo — milliseconds
#define MESSAGE_DURATION  10000   // Farewell message — milliseconds
#define QUOTE_DURATION    30000   // Each quote — milliseconds
```

Change the number to however many milliseconds you want.
`1000` = 1 second, `10000` = 10 seconds, `60000` = 1 minute.

---


## Quick Reference

| I want to... | Where |
|---|---|
| Change the message | `stephen_farewell.ino` → `showFarewellMessage()` |
| Add or edit quotes | `quotes.h` → edit the string array |
| Replace the photo | Run `convert_photo.py`, replace `team_bitmap.h` |
| Change screen timing | `stephen_farewell.ino` → top 3 `#define` lines |
| Upload to board | Arduino IDE → select board + port → click Upload |

---

## Troubleshooting

| Problem | Fix |
|---|---|
| Upload fails / board not found | Hold **BOOT** on the XIAO while clicking Upload. Release once upload starts. |
| Display shows nothing | Check all 6 wires are connected. Check the correct port is selected. |
| Text runs off the edge | Shorten your line in `showFarewellMessage()` to under 20 characters. |
| Photo too dark or washed out | Change `.enhance(2.5)` in the script — lower = less contrast, higher = more. |
| Screen flickers on refresh | Normal behaviour — ePaper always flickers on a full-screen update. |
