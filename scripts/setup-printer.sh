#!/bin/sh
set -eu

queue=TSC_DA200
share_lan=0
fixed_4x6=0
while [ "$#" -gt 0 ]; do
  case "$1" in
    --queue)
      [ "$#" -ge 2 ] || { echo "--queue requires a name" >&2; exit 2; }
      queue=$2
      shift 2
      ;;
    --share-lan) share_lan=1; shift ;;
    --fixed-4x6) fixed_4x6=1; shift ;;
    --help|-h)
      echo "Usage: sudo $0 [--queue NAME] [--share-lan] [--fixed-4x6] 'usb://...'"
      echo "   or: sudo $0 [--queue NAME] [--share-lan] [--fixed-4x6] 'socket://PRINTER_IP:9100'"
      exit 0
      ;;
    --*) echo "Unknown option: $1" >&2; exit 2 ;;
    *) break ;;
  esac
done
[ "$#" -eq 1 ] || { echo "Exactly one printer URI is required" >&2; exit 2; }
printer_uri=$1
ppd_path=/usr/share/ppd/tsc/TSC-DA200.ppd
if [ "$fixed_4x6" -eq 1 ]; then
  ppd_path=/usr/share/ppd/tsc/TSC-DA200-4x6.ppd
fi

if [ "$(id -u)" -ne 0 ]; then
  echo "Run this setup script with sudo." >&2
  exit 1
fi
if [ ! -f "$ppd_path" ]; then
  echo "Driver is not installed at $ppd_path; run 'sudo make install' first." >&2
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
  -o TscSpeed=4 -o TscDensity=7 -o printer-is-shared="$([ "$share_lan" -eq 1 ] && echo true || echo false)"
cupsaccept "$queue"
cupsenable "$queue"

echo "Queue created: $queue"
if [ "$fixed_4x6" -eq 1 ]; then
  echo "Fixed-media mode: only 4 x 6 inch labels are advertised by this queue."
fi
if [ "$share_lan" -eq 1 ]; then
  # Keep administration local and retain CUPS' normal local-network access
  # policy. `--remote-any` would unnecessarily admit arbitrary source networks.
  cupsctl --share-printers --no-remote-admin
  echo "LAN sharing enabled for CUPS; remote administration remains disabled."
  echo "Restrict TCP 631 to your trusted LAN/VPN in the host firewall."
  echo "LAN URL: ipp://$(hostname).local:631/printers/$queue"
else
  echo "Queue is local-only. Re-run with --share-lan to expose it through IPP."
fi
