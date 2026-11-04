#pragma once

#include <Arduino.h>

// Called with the new note text whenever a client updates it.
typedef void (*NoteCallback)(const String& text);

// Start the HTTP server. `initial` is the note to report until a client
// sends a new one; `onChange` is invoked for every accepted update.
void noteServerBegin(const String& initial, NoteCallback onChange);

// Service pending HTTP requests. Call from loop().
void noteServerHandle();
