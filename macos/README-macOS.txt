TSC DA200 Open Driver for macOS
================================

This package installs a universal CUPS filter containing native arm64 and
x86_64 code. It does not require Rosetta on Apple Silicon.

This is an independent community project and is not affiliated with or
endorsed by TSC Auto ID Technology Co., Ltd. License and third-party notices
are installed beside this file.

Installed files:

  /Library/Printers/TSC/OpenDA200/Filter/rastertotspl
  /Library/Printers/PPDs/Contents/Resources/TSC-DA200-Open.ppd
  /Library/Printers/TSC/OpenDA200/setup-printer.sh
  /Library/Printers/TSC/OpenDA200/calibrate.sh
  /Library/Printers/TSC/OpenDA200/diagnose.sh

After installation, add the printer in System Settings > Printers & Scanners.
When macOS asks which software to use, select:

  TSC DA200 203dpi (Open TSPL)

Alternatively, discover the printer URI with:

  lpinfo -v

Then create the queue with:

  sudo /Library/Printers/TSC/OpenDA200/setup-printer.sh 'usb://...'

For a location that must only ever use 4 x 6 inch labels, use the dedicated
fixed-media PPD:

  sudo /Library/Printers/TSC/OpenDA200/setup-printer.sh --fixed-4x6 'usb://...'

For a network-equipped DA200, use:

  sudo /Library/Printers/TSC/OpenDA200/setup-printer.sh \
    'socket://PRINTER_IP:9100'

Official release packages are signed with Developer ID Application and
Developer ID Installer certificates, notarized by Apple, and stapled. A local
source build remains ad-hoc signed unless both signing identities are supplied
to macos/build-pkg.sh. Install with:

  sudo installer -pkg TSC-DA200-OpenDriver-1.0.0-universal.pkg -target /

Installing an update automatically refreshes queues that already use the open
driver while retaining their URI and saved printer options.

The default diagnostic report omits device identifiers, URIs, job metadata,
and logs. If support specifically needs those details, use:

  /Library/Printers/TSC/OpenDA200/diagnose.sh --include-sensitive QUEUE

Review that output before sharing it.
