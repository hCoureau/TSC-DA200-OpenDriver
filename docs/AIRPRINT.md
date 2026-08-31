# Raspberry Pi AirPrint (fixed 4 × 6)

The `v1.1.0-rc.1` pre-release includes a PAPPL IPP Everywhere printer application. It is a
separate AirPrint endpoint, not a shared legacy CUPS/PPD queue. The endpoint
advertises only thermal monochrome 4 × 6 inch labels at 203 dpi, deliberately
preventing clients from selecting incompatible paper sizes.

## Install

On a 64-bit Raspberry Pi OS or Debian host, install the ARM64 `.deb` release
asset, then point the service at the DA200:

```sh
sudo apt install ./tsc-da200-cups_1.1.0_arm64.deb
sudo tsc-da200-airprint-setup socket://PRINTER_HOST:9100
```

For a USB-connected printer, use the URI reported by `lpinfo -v`, for example
`usb://TSC/DA200?...`. Do not copy a URI containing credentials into a public
issue or log.

The helper creates `/etc/tsc-da200-printer-app.conf`, enables
`tsc-da200-printer-app.service`, and publishes a queue called **TSC DA200 4x6**.
Avahi/mDNS must be installed and running for iPhone/iPad discovery:

```sh
sudo apt install avahi-daemon
systemctl status tsc-da200-printer-app.service avahi-daemon
```

Do not also share the legacy CUPS DA200 queue over AirPrint; advertise only the
new queue to avoid clients choosing the incompatible endpoint.

## RC acceptance test

1. From iOS, choose **TSC DA200 4x6** in the system print sheet.
2. Print a normal document/PDF with one page and confirm it prints upright,
   complete, and aligned on a 4 × 6 label.
3. Print two pages and verify both labels emerge in source order.
4. Verify the iOS sheet offers no conflicting paper-size choice.
5. Capture service evidence with `journalctl -u tsc-da200-printer-app -b` if a
   job fails; redact device URIs before sharing it.
