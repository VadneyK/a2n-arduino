#!/usr/bin/env bash
# Send a note to the ESP32 sticky note display over HTTP.
#
# Usage:
#   tools/send-note.sh "Amazing Grace" "Key: G"      # each argument is one line
#   echo "Amazing Grace" | tools/send-note.sh         # or pipe the text in
#   tools/send-note.sh --clear                        # blank the display
#   tools/send-note.sh --get                          # print the current note
#   tools/send-note.sh --status                       # print the device status JSON
#
# Options:
#   -H, --host HOST   display hostname or IP (default: $STICKY_NOTE_HOST
#                     or sticky-note.local)
#   -h, --help        show this help
#
# Only curl is required.

set -euo pipefail

host="${STICKY_NOTE_HOST:-sticky-note.local}"
action="send"

usage() { sed -n '2,19p' "$0" | sed 's/^# \{0,1\}//'; }

while [ $# -gt 0 ]; do
  case "$1" in
    -H|--host) host="$2"; shift 2 ;;
    --clear) action="clear"; shift ;;
    --get) action="get"; shift ;;
    --status) action="status"; shift ;;
    -h|--help) usage; exit 0 ;;
    --) shift; break ;;
    -*) echo "unknown option: $1" >&2; usage >&2; exit 2 ;;
    *) break ;;
  esac
done

base="http://${host}"

case "$action" in
  clear)
    curl -fsS -X POST "${base}/clear"
    ;;
  get)
    curl -fsS "${base}/note"
    echo
    ;;
  status)
    curl -fsS "${base}/status"
    ;;
  send)
    if [ $# -gt 0 ]; then
      text="$(printf '%s\n' "$@")"
      text="${text%$'\n'}"
    else
      text="$(cat)"
    fi
    curl -fsS -X POST --data-urlencode "text=${text}" "${base}/note"
    ;;
esac
