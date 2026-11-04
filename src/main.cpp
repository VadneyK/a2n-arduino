// ESP32 Cheap Yellow Display sticky note.
//
// Boot sequence:
//   1. Initialise the TFT and show "Connecting".
//   2. Join the Wi-Fi network from include/secrets.h. If that file is missing
//      or the network is unreachable, start an open access point instead.
//   3. Advertise HOSTNAME.local over mDNS and start the HTTP server.
//   4. Restore the last note from flash and draw it.
//
// The loop services HTTP requests and keeps the footer showing the address.

#include <Arduino.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include <WiFi.h>

#include "config.h"
#include "display.h"
#include "note_server.h"

#if __has_include("secrets.h")
#include "secrets.h"
#endif

namespace {

Preferences prefs;
String address;  // what to print in the footer, e.g. "sticky-note.local  192.168.1.20"

void onNoteChanged(const String& text) {
  prefs.putString("text", text);
  displayNote(text);
  Serial.printf("note updated (%u bytes)\n", text.length());
}

bool connectToWifi() {
#if defined(WIFI_SSID) && defined(WIFI_PASSWORD)
  displayStatus("Connecting", String("Wi-Fi: ") + WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.setHostname(HOSTNAME);
  WiFi.setAutoReconnect(true);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  const unsigned long started = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - started < WIFI_CONNECT_TIMEOUT_MS) {
    delay(250);
  }
  if (WiFi.status() == WL_CONNECTED) {
    address = String(HOSTNAME) + ".local  " + WiFi.localIP().toString();
    Serial.printf("connected to %s as %s\n", WIFI_SSID, WiFi.localIP().toString().c_str());
    return true;
  }
  Serial.printf("could not join %s\n", WIFI_SSID);
#else
  Serial.println("include/secrets.h not found, skipping Wi-Fi client mode");
#endif
  return false;
}

void startFallbackAp() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(FALLBACK_AP_SSID);
  address = String("AP ") + FALLBACK_AP_SSID + "  " + WiFi.softAPIP().toString();
  Serial.printf("started access point %s at %s\n", FALLBACK_AP_SSID, WiFi.softAPIP().toString().c_str());
}

}  // namespace

void setup() {
  Serial.begin(115200);
  displayBegin();

  if (!connectToWifi()) startFallbackAp();

  if (MDNS.begin(HOSTNAME)) {
    MDNS.addService("http", "tcp", HTTP_PORT);
  } else {
    Serial.println("mDNS failed to start");
  }

  prefs.begin("note", false);
  const String saved = prefs.getString("text", "");

  noteServerBegin(saved, onNoteChanged);
  displayNote(saved);
  displayFooter(address);
  Serial.printf("ready at http://%s.local/\n", HOSTNAME);
}

void loop() {
  noteServerHandle();
  delay(2);
}
