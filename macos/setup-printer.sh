#!/bin/sh
set -eu

queue=TSC_DA200_Open
fixed_4x6=0
while [ "$#" -gt 0 ]; do
  case "$1" in
    --queue) [ "$#" -ge 2 ] || { echo "--queue requires a name" >&2; exit 2; }; queue=$2; shift 2 ;;
    --fixed-4x6) fixed_4x6=1; shift ;;
    --help|-h) echo "Usage: sudo $0 [--queue NAME] [--fixed-4x6] 'usb://...'"; exit 0 ;;
    --*) echo "Unknown option: $1" >&2; exit 2 ;;
    *) break ;;
  esac
done
[ "$#" -eq 1 ] || {
  echo "Usage: sudo $0 [--queue NAME] [--fixed-4x6] 'usb://...'" >&2
  echo "   or: sudo $0 [--queue NAME] [--fixed-4x6] 'socket://PRINTER_IP:9100'" >&2
  echo "Find attached printers with: lpinfo -v" >&2
  exit 2
}
printer_uri=$1
ppd_path=/Library/Printers/PPDs/Contents/Resources/TSC-DA200-Open.ppd
if [ "$fixed_4x6" -eq 1 ]; then
  ppd_path=/Library/Printers/PPDs/Contents/Resources/TSC-DA200-Open-4x6.ppd
fi

if [ "$(id -u)" -ne 0 ]; then
  echo "Run this setup script with sudo." >&2
  exit 1
fi
if [ ! -f "$ppd_path" ]; then
  echo "The open DA200 driver is not installed." >&2
  exit 1
fi
case "$queue" in
  ''|*[!A-Za-z0-9_.-]*) echo "Queue name contains unsupported characters" >&2; exit 2 ;;
esac
case "$printer_uri" in
  usb://*|socket://*) ;;
  *) echo "Only usb:// and socket:// printer URIs are accepted" >&2; exit 2 ;;
esac

lpadmin -p "$queue" -E -v "$printer_uri" -P "$ppd_path" \
  -o PageSize=4x6.Fullbleed -o TscMediaType=Gap -o TscGap=3 \
  -o TscSpeed=4 -o TscDensity=7 -o printer-is-shared=false
cupsaccept "$queue"
cupsenable "$queue"

echo "Queue created: $queue"
if [ "$fixed_4x6" -eq 1 ]; then
  echo "Fixed-media mode: only 4 x 6 inch labels are advertised by this queue."
fi
