# Feature parity and extensions

## Parity with the installed TSC macOS driver

| Capability | Open driver |
|---|---|
| 203 dpi raster printing | Supported |
| Custom label sizes | Supported, up to the DA200's 108 mm print width |
| Speed | 2 through 6 inches/second |
| Density | Every level from 0 through 15 |
| Gap, black-mark, and continuous media | Supported |
| Gap/mark height and extra feed | Supported |
| Horizontal and vertical reference | Supported |
| Signed horizontal and vertical image shift | Supported |
| Tear/peel/cutter stop offset | Supported, +/-25.4 mm |
| Tear and peel | Supported; peel is explicitly labelled as requiring the optional peeler |
| Full/partial cut | Not exposed: TSC does not list a cutter as compatible with the DA200 |
| Direction, mirror, and negative image | Supported |
| Monochrome, threshold, and ordered halftone | Supported |
| Multiple copies and CUPS collation | Supported |

The TSC PPD also exposes thermal-transfer mode. The DA200 is a direct-thermal
printer, so the open driver intentionally keeps `SET RIBBON OFF`. Its
`DirectBuffer=AUTO` and `StoredGraphics=AUTO` entries offer no selectable
behavior and are not duplicated.

The official generic PPD advertises page lengths beyond this model's physical
specification. The open driver uses the DA200 limit instead.

## Additional operational features

- `tools/calibrate.sh` sends explicit gap, black-mark, or automatic media
  calibration commands. It warns before feeding labels.
- `tools/diagnose.sh` collects redacted OS, architecture, USB-presence, CUPS,
  and queue-option information without collecting printed document contents.
  Device identifiers, URIs, job metadata, and logs require the explicit
  `--include-sensitive` switch and a review-before-sharing warning.
- Both USB and TCP port 9100 printer connections are supported.
- The Pi can share the rendered queue through IPP so clients do not need the
  driver locally.
- Ordered halftoning uses an independently reproducible mathematical Bayer
  screen. It keeps exact-white areas clean, produces complete solid-black
  coverage, and avoids carrying any vendor-derived lookup data.
- Firmware installation is deliberately outside the driver; see
  `docs/FIRMWARE.md` for the tested unit's cross-branch safety finding.

## Future architecture

A PAPPL-based Printer Application is the planned replacement for legacy PPD
drivers and will offer a driverless IPP endpoint on future CUPS versions. The
current CUPS filter remains the tested v1 path for Raspberry Pi OS and current
macOS releases. The architecture and release gates are defined in
`docs/FUTURE-PROOFING.md`.
