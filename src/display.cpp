#include "display.h"

#include <TFT_eSPI.h>

#include "config.h"

namespace {

TFT_eSPI tft;

constexpr uint8_t kNoteFont = 4;    // TFT_eSPI built-in 26 px font
constexpr uint8_t kFooterFont = 2;  // TFT_eSPI built-in 16 px font
constexpr int kFooterHeight = 20;
constexpr int kMargin = 6;
constexpr int kWrapCapacity = 2 * MAX_LINES;  // scratch space while wrapping

int screenWidth() { return tft.width(); }
int noteAreaHeight() { return tft.height() - kFooterHeight; }

// Word-wrap `text` at the current text size into `out`. Returns the number of
// lines produced, which may exceed `capacity` (extra lines are counted but
// not stored) so the caller can decide whether the note fits.
int wrapText(const String& text, int maxWidth, String* out, int capacity) {
  int count = 0;
  auto push = [&](const String& line) {
    if (count < capacity) out[count] = line;
    count++;
  };

  int start = 0;
  while (start <= (int)text.length()) {
    int end = text.indexOf('\n', start);
    if (end < 0) end = text.length();
    String paragraph = text.substring(start, end);
    paragraph.replace("\r", "");
    start = end + 1;

    String line;
    int wordStart = 0;
    while (wordStart <= (int)paragraph.length()) {
      int wordEnd = paragraph.indexOf(' ', wordStart);
      if (wordEnd < 0) wordEnd = paragraph.length();
      String word = paragraph.substring(wordStart, wordEnd);
      wordStart = wordEnd + 1;
      if (word.isEmpty()) continue;

      String candidate = line.isEmpty() ? word : line + " " + word;
      if (tft.textWidth(candidate, kNoteFont) <= maxWidth) {
        line = candidate;
        continue;
      }
      if (!line.isEmpty()) push(line);

      // A single word wider than the screen is broken between characters.
      while (tft.textWidth(word, kNoteFont) > maxWidth && word.length() > 1) {
        int cut = word.length() - 1;
        while (cut > 1 && tft.textWidth(word.substring(0, cut), kNoteFont) > maxWidth) cut--;
        push(word.substring(0, cut));
        word = word.substring(cut);
      }
      line = word;
    }
    push(line);  // an empty paragraph keeps its blank line
  }
  return count;
}

}  // namespace

void displayBegin() {
  tft.init();
  tft.setRotation(1);  // landscape, USB connector on the right
  tft.fillScreen(TFT_BLACK);
  tft.setTextWrap(false);
}

void displayNote(const String& text) {
  const int maxWidth = screenWidth() - 2 * kMargin;
  String lines[kWrapCapacity];

  // Prefer the double-size font; fall back to single size for longer notes.
  uint8_t size = 2;
  tft.setTextSize(size);
  int count = wrapText(text, maxWidth, lines, kWrapCapacity);
  int lineHeight = tft.fontHeight(kNoteFont);
  if (count * lineHeight > noteAreaHeight()) {
    size = 1;
    tft.setTextSize(size);
    count = wrapText(text, maxWidth, lines, kWrapCapacity);
    lineHeight = tft.fontHeight(kNoteFont);
  }
  if (count > MAX_LINES) count = MAX_LINES;

  tft.fillRect(0, 0, screenWidth(), noteAreaHeight(), TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(TC_DATUM);
  int y = (noteAreaHeight() - count * lineHeight) / 2;
  for (int i = 0; i < count; i++) {
    tft.drawString(lines[i], screenWidth() / 2, y, kNoteFont);
    y += lineHeight;
  }
}

void displayFooter(const String& text) {
  const int top = tft.height() - kFooterHeight;
  tft.fillRect(0, top, screenWidth(), kFooterHeight, TFT_BLACK);
  tft.setTextSize(1);
  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.setTextDatum(BC_DATUM);
  tft.drawString(text, screenWidth() / 2, tft.height() - 2, kFooterFont);
}

void displayStatus(const String& title, const String& detail) {
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(title, screenWidth() / 2, tft.height() / 2 - 16, kNoteFont);
  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.drawString(detail, screenWidth() / 2, tft.height() / 2 + 16, kFooterFont);
}
