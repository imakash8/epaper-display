#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMono12pt7b.h>
#include "team_bitmap.h"
#include "quotes.h"

#define EPD_CS    D1
#define EPD_DC    D3
#define EPD_RST   D0
#define EPD_BUSY  D5

GxEPD2_BW<GxEPD2_213_GDEY0213B74, GxEPD2_213_GDEY0213B74::HEIGHT> display(
  GxEPD2_213_GDEY0213B74(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY));

#define PHOTO_DURATION    10000
#define MESSAGE_DURATION  10000
#define QUOTE_DURATION    30000

int currentQuote = 0;
int totalQuotes   = 0;

// ── Word-wrap helper ──────────────────────────────────────────────────────
// Returns the Y position after last line (so caller knows how tall it drew)
int drawWrapped(const char* text, int x, int y, int maxWidth, int lineH) {
  char buf[200];
  strncpy(buf, text, sizeof(buf));
  buf[sizeof(buf)-1] = '\0';

  char line[80] = "";
  char word[40] = "";
  int  wi   = 0;
  int  curY = y;

  for (int i = 0; ; i++) {
    char c = buf[i];
    if (c == ' ' || c == '\0') {
      word[wi] = '\0';
      wi = 0;

      char test[80];
      if (strlen(line) == 0)
        snprintf(test, sizeof(test), "%s", word);
      else
        snprintf(test, sizeof(test), "%s %s", line, word);

      int16_t x1, y1; uint16_t w, h;
      display.getTextBounds(test, x, curY, &x1, &y1, &w, &h);

      if ((int)w > maxWidth && strlen(line) > 0) {
        display.setCursor(x, curY);
        display.print(line);
        curY += lineH;
        snprintf(line, sizeof(line), "%s", word);
      } else {
        snprintf(line, sizeof(line), "%s", test);
      }

      if (c == '\0') {
        display.setCursor(x, curY);
        display.print(line);
        break;
      }
    } else {
      word[wi++] = c;
    }
  }
  return curY;
}

// ── Auto font size picker ─────────────────────────────────────────────────
// Sets g_font and g_lineH globals to the best fitting font
const GFXfont* g_font = &FreeMonoBold12pt7b;
int g_lineH = 18;

void pickFont(const char* text, int x, int startY, int maxWidth, int maxHeight) {
  display.setFont(&FreeMonoBold12pt7b);
  char buf[200];
  strncpy(buf, text, sizeof(buf));

  int lineH12 = 18;
  int lines = 1;
  char line[80] = "";
  char word[40] = "";
  int wi = 0;

  for (int i = 0; ; i++) {
    char c = buf[i];
    if (c == ' ' || c == '\0') {
      word[wi] = '\0'; wi = 0;
      char test[80];
      if (strlen(line) == 0) snprintf(test, sizeof(test), "%s", word);
      else snprintf(test, sizeof(test), "%s %s", line, word);

      int16_t x1, y1; uint16_t w, h;
      display.getTextBounds(test, x, startY, &x1, &y1, &w, &h);
      if ((int)w > maxWidth && strlen(line) > 0) {
        lines++;
        snprintf(line, sizeof(line), "%s", word);
      } else {
        snprintf(line, sizeof(line), "%s", test);
      }
      if (c == '\0') break;
    } else { word[wi++] = c; }
  }

  if (lines * lineH12 <= maxHeight) {
    g_font  = &FreeMonoBold12pt7b;
    g_lineH = lineH12;
  } else {
    g_font  = &FreeMonoBold9pt7b;
    g_lineH = 14;
  }
}

// ── Glitter transition — fast, 3 rounds ──────────────────────────────────
void glitterTransition() {
  for (int round = 0; round < 3; round++) {  // ← 3 rounds instead of 5
    display.setPartialWindow(0, 0, 250, 122);
    display.firstPage();
    do {
      display.fillScreen(GxEPD_WHITE);
      for (int i = 0; i < 60; i++) {          // ← 60 dots instead of 80
        int sx = random(2, 248);
        int sy = random(2, 120);
        int sz = random(1, 4);
        display.fillRect(sx, sy, sz, sz, GxEPD_BLACK);
        if (sz == 3) {
          display.drawPixel(sx - 1, sy + 1, GxEPD_BLACK);
          display.drawPixel(sx + 2, sy + 1, GxEPD_BLACK);
          display.drawPixel(sx + 1, sy - 1, GxEPD_BLACK);
          display.drawPixel(sx + 1, sy + 2, GxEPD_BLACK);
        }
      }
    } while (display.nextPage());
    delay(80);  // ← 80ms instead of 120ms
  }
}

// ── Screen 1: Team Photo ──────────────────────────────────────────────────
void showTeamPhoto() {
  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_BLACK);
    display.drawBitmap(0, 0, team_bitmap, TEAM_WIDTH, TEAM_HEIGHT, GxEPD_WHITE);
  } while (display.nextPage());
}

// ── Screen 2: Farewell Message ────────────────────────────────────────────
void showFarewellMessage() {
  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    display.drawRect(2, 2, 246, 118, GxEPD_BLACK);

    display.setFont(&FreeMonoBold9pt7b);
    display.setTextColor(GxEPD_BLACK);
    display.setCursor(8, 28);
    display.print("Thank you for");
    display.setCursor(8, 46);
    display.print("everything, and best");
    display.setCursor(8, 64);
    display.print("of luck in your new");
    display.setCursor(8, 82);
    display.print("journey ahead!");

    display.drawLine(8, 90, 242, 90, GxEPD_BLACK);
    display.setFont(&FreeMono9pt7b);
    display.setCursor(138, 108);
    display.print("- By Akash");

  } while (display.nextPage());
}

// ── Screen 3: Quote with auto font sizing ────────────────────────────────
void showQuote(int index) {
  const char* q = quotes[index];

  char quotePart[200] = "";
  char authorPart[80] = "";
  const char* sep = strstr(q, " - ");

  if (sep) {
    int qLen = sep - q;
    strncpy(quotePart, q, qLen);
    quotePart[qLen] = '\0';
    strncpy(authorPart, sep + 3, sizeof(authorPart));
  } else {
    strncpy(quotePart, q, sizeof(quotePart));
  }

  // Available height for quote text:
  // Top y=22, bottom y=100 (leaves room for author line at 104-116)
  int availableH = 78; // 100 - 22

  // Pick best fitting font — sets g_font and g_lineH
  pickFont(quotePart, 8, 22, 234, availableH);

  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    display.drawRect(2, 2, 246, 118, GxEPD_BLACK);

    // Draw quote with chosen font
    display.setFont(g_font);
    display.setTextColor(GxEPD_BLACK);
    drawWrapped(quotePart, 8, 22, 234, g_lineH);

    // Author
    if (strlen(authorPart) > 0) {
      display.drawLine(8, 104, 242, 104, GxEPD_BLACK);
      display.setFont(&FreeMono9pt7b);
      int16_t x1, y1; uint16_t w, h;
      display.getTextBounds(authorPart, 0, 0, &x1, &y1, &w, &h);
      int ax = 244 - (int)w - 4;
      if (ax < 8) ax = 8;
      display.setCursor(ax, 116);
      display.print(authorPart);
    }

  } while (display.nextPage());
}

// ── Setup ─────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  Serial.println("Starting...");

  display.init(115200, true, 2, false);
  display.setRotation(1);

  totalQuotes = sizeof(quotes) / sizeof(quotes[0]);
  randomSeed(analogRead(0));
  currentQuote = random(totalQuotes);

  showTeamPhoto();
  delay(PHOTO_DURATION);

  showFarewellMessage();
  delay(MESSAGE_DURATION);

  glitterTransition();
  showQuote(currentQuote);
}

// ── Loop ──────────────────────────────────────────────────────────────────
void loop() {
  delay(QUOTE_DURATION);
  currentQuote = (currentQuote + 1) % totalQuotes;
  glitterTransition();
  showQuote(currentQuote);
}
