#!/bin/sh
set -u

include_sensitive=0
if [ "${1:-}" = "--include-sensitive" ]; then
  include_sensitive=1
  shift
fi
[ "$#" -le 1 ] || {
  echo "Usage: $0 [--include-sensitive] [QUEUE]" >&2
  exit 2
}
queue=${1:-TSC_DA200}
case "$queue" in
  ''|*[!A-Za-z0-9_.-]*)
    echo "Queue name contains unsupported characters" >&2
    exit 2
    ;;
esac

echo "TSC DA200 diagnostic report"
date -u '+UTC: %Y-%m-%dT%H:%M:%SZ'
uname -srm

if command -v sw_vers >/dev/null 2>&1; then sw_vers; fi
if [ -r /etc/os-release ]; then cat /etc/os-release; fi
if command -v dpkg >/dev/null 2>&1; then dpkg --print-architecture; fi

echo "CUPS"
cups-config --version 2>/dev/null || true
lpstat -r 2>&1 || true
lpstat -p "$queue" 2>&1 || true
lpstat -a "$queue" 2>&1 || true
lpoptions -p "$queue" -l 2>&1 || true

echo "USB"
if command -v lsusb >/dev/null 2>&1; then
  if lsusb -d 1203:0252 >/dev/null 2>&1; then
    echo "USB 1203:0252 present"
  else
    echo "USB 1203:0252 not detected"
  fi
elif command -v ioreg >/dev/null 2>&1; then
  if ioreg -p IOService -r -c IOUSBHostInterface -l -w 0 2>/dev/null |
      awk 'BEGIN { RS="" } /"idVendor" = 4611/ && /"idProduct" = 594/ { found=1 } END { exit found ? 0 : 1 }'; then
    echo "USB 1203:0252 present"
  else
    echo "USB 1203:0252 not detected"
  fi
else
  echo "USB presence check unavailable"
fi

if [ "$include_sensitive" -eq 1 ]; then
  echo "WARNING: sensitive diagnostics follow. Review before sharing."
  echo "They may contain hostnames, usernames, job titles, device URIs, serial numbers, IP addresses, and log content."
  uname -a
  echo "CUPS devices and jobs"
  lpstat -t 2>&1 || true
  lpinfo -v 2>&1 || true
  echo "USB details"
  if command -v lsusb >/dev/null 2>&1; then
    lsusb -d 1203:0252 -v 2>/dev/null || true
  elif command -v ioreg >/dev/null 2>&1; then
    ioreg -p IOService -r -c IOUSBHostInterface -l -w 0 2>/dev/null |
      awk 'BEGIN { RS="" } /"idVendor" = 4611/ && /"idProduct" = 594/ { print }' || true
  fi
  echo "Recent CUPS errors"
  if command -v journalctl >/dev/null 2>&1; then
    journalctl -u cups --since '30 minutes ago' -p warning --no-pager 2>/dev/null || true
  elif [ -r /var/log/cups/error_log ]; then
    tail -n 80 /var/log/cups/error_log
  fi
else
  echo "Sensitive device details, job metadata, URIs, and logs omitted."
  echo "Re-run with --include-sensitive only when needed, then review before sharing."
fi
