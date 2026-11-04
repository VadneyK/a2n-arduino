#pragma once

#include <Arduino.h>

// Initialise the TFT and clear it.
void displayBegin();

// Draw a note. Lines are split on '\n' and word-wrapped to the screen width.
// Short notes are drawn in a larger font; up to MAX_LINES lines are shown.
void displayNote(const String& text);

// Replace the one-line footer at the bottom of the screen (address, status).
void displayFooter(const String& text);

// Show a full-screen status message, e.g. while connecting to Wi-Fi.
void displayStatus(const String& title, const String& detail);
