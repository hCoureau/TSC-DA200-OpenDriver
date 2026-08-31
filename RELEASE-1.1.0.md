# TSC DA200 Open Driver 1.1.0

This pre-release candidate adds a Raspberry Pi-ready PAPPL Printer Application
for AirPrint. It exposes one fixed, monochrome 4 × 6 inch label queue at 203 dpi.

Highlights:

- fixes the invalid zero-resolution IPP defaults that caused iOS to reject the queue;
- advertises a standards-compliant single 4 × 6 inch media database and URF record;
- renders incoming PWG raster jobs to native TSPL; and
- retains the legacy CUPS filter and PPDs for conventional desktop queues.

Validate an iPhone/iPad print and a physical DA200 label before promoting this
pre-release candidate to the final 1.1.0 release.
