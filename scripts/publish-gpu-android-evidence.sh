#!/system/bin/sh
# Invoked by instrumentation through UiAutomation as shell, not the app UID.
# The host creates this exclusive directory and supplies this reviewed script.
set -eu
umask 077
set -C
case "$#" in 1) ;; *) exit 1 ;; esac
case "$1" in ''|*[!A-Za-z0-9+/=]*) exit 1 ;; esac
[ "${#1}" -le 21848 ]
directory=${0%/*}
printf '%s' "$1" | base64 -d > "$directory/evidence.json"
printf PIXAURA_WRITTEN
