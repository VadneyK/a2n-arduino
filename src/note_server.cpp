#include "note_server.h"

#include <WebServer.h>
#include <WiFi.h>

#include "config.h"
#include "page.h"

namespace {

WebServer server(HTTP_PORT);
String currentNote;
NoteCallback changeCallback = nullptr;

void setNote(const String& text) {
  currentNote = text;
  if (changeCallback) changeCallback(currentNote);
}

void handleIndex() {
  server.send_P(200, "text/html; charset=utf-8", kIndexHtml);
}

void handleGetNote() {
  server.send(200, "text/plain; charset=utf-8", currentNote);
}

// Accepts either a form field named "text" (application/x-www-form-urlencoded)
// or a raw request body with any other content type, which the ESP32
// WebServer exposes as the "plain" argument.
void handlePostNote() {
  String text;
  if (server.hasArg("text")) {
    text = server.arg("text");
  } else if (server.hasArg("plain")) {
    text = server.arg("plain");
  } else {
    server.send(400, "text/plain", "missing note text\n");
    return;
  }
  if (text.length() > MAX_NOTE_LENGTH) {
    server.send(413, "text/plain", "note too long\n");
    return;
  }
  setNote(text);
  server.send(200, "text/plain", "ok\n");
}

void handleClear() {
  setNote("");
  server.send(200, "text/plain", "ok\n");
}

void handleStatus() {
  String json = "{";
  json += "\"hostname\":\"" HOSTNAME "\",";
  json += "\"ip\":\"" + (WiFi.getMode() == WIFI_AP ? WiFi.softAPIP() : WiFi.localIP()).toString() + "\",";
  json += "\"rssi\":" + String(WiFi.RSSI()) + ",";
  json += "\"uptime_s\":" + String(millis() / 1000) + ",";
  json += "\"note_length\":" + String(currentNote.length());
  json += "}\n";
  server.send(200, "application/json", json);
}

void handleNotFound() {
  server.send(404, "text/plain", "not found\n");
}

}  // namespace

void noteServerBegin(const String& initial, NoteCallback onChange) {
  currentNote = initial;
  changeCallback = onChange;
  server.on("/", HTTP_GET, handleIndex);
  server.on("/note", HTTP_GET, handleGetNote);
  server.on("/note", HTTP_POST, handlePostNote);
  server.on("/clear", HTTP_POST, handleClear);
  server.on("/status", HTTP_GET, handleStatus);
  server.onNotFound(handleNotFound);
  server.begin();
}

void noteServerHandle() {
  server.handleClient();
}
