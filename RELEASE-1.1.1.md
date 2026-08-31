# TSC DA200 Open Driver 1.1.1

This second release candidate retains the Raspberry Pi-ready PAPPL Printer
Application for AirPrint. It exposes one fixed, monochrome 4 × 6 inch label
queue at 203 dpi.

RC2 fixes the physical feed direction and replaces ordered dithering with a
deterministic threshold path in the AirPrint renderer. This keeps barcode bars
and monochrome logo edges crisp rather than turning gray edge pixels into a
row-dependent dot pattern.

The Debian package is versioned 1.1.1 so a host running the earlier 1.1.0 RC
upgrades normally through `apt`; its existing AirPrint configuration is kept
and the printer-application service is restarted on upgrade.

Validate an iPhone/iPad print and a physical DA200 label before promoting this
pre-release candidate to the final 1.1.1 release.
