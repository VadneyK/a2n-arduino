#pragma once

// Network name used for mDNS. The display answers at http://HOSTNAME.local
#define HOSTNAME "sticky-note"

// Name of the open access point the display starts when it cannot join
// the configured Wi-Fi network (or when include/secrets.h is missing).
#define FALLBACK_AP_SSID "sticky-note-setup"

// How long to wait for the configured Wi-Fi network before falling back.
#define WIFI_CONNECT_TIMEOUT_MS 20000

// Longest note accepted over HTTP, in bytes.
#define MAX_NOTE_LENGTH 512

// Most lines drawn on the screen. Extra lines are dropped.
#define MAX_LINES 8

// HTTP port of the web server.
#define HTTP_PORT 80
