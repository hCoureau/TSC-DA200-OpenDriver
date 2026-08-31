# TSC DA200 Open Driver 1.0.0

## Status

Version 1.0.0 is the production-release target for the tested TSC DA200 unit.
The macOS package is native on Apple Silicon and Intel. Raspberry Pi packages
target both 64-bit ARM and 32-bit ARM. Platform claims in this document are
limited to the environments actually exercised before release.

## Supported platforms

- macOS 11 through macOS 26, native universal `arm64` and `x86_64` filter.
- macOS 27 Golden Gate compatibility target: explicit ARM64/Intel installer
  metadata and CI against GitHub's newest public macOS/Xcode toolchain; final
  certification still requires a macOS 27 runtime testbed.
- Raspberry Pi OS / Debian ARM64 and ARMHF packages.
- Fedora 42 native RPM packaging and validation.
- Source builds on compatible CUPS 2.x Linux distributions.

## v1 capabilities

- 203 dpi raster printing over USB or TCP port 9100.
- Label sizes through the DA200's 108 mm print width and 90 inch length.
- Gap, black-mark, and continuous media.
- Speed, density, reference, signed shift, stop offset, direction, tear, and
  optional peel controls.
- Monochrome, threshold, and independently generated ordered-halftone modes.
- Mirror, negative, combined transforms, multiple copies, and correct CUPS
  collated and uncollated page ordering.
- Exact-white halftone output and complete solid-black coverage.
- Explicit opt-in Raspberry Pi LAN sharing with a firewall warning.
- Calibration and privacy-safe-by-default diagnostics, with sensitive support
  data available only through an explicit opt-in.

## Release gates

- Unit, raster integration, malformed-input, sanitizer, and PPD validation.
- Universal macOS binary and installer-structure validation.
- Physical macOS printing on the identified TSC DA200 over USB.
- Photographic acceptance of the complete numbered 1.0.0 physical suite.
- Native Debian ARM64 and ARMHF build, install, and regression validation.
- Artifact checksums and archive hygiene checks.
- Version-synchronization and finished-package architecture/deployment audits.

## Distribution qualifications

The public macOS package is built with Developer ID Application and Developer
ID Installer identities, submitted to Apple's notary service, and stapled.
Local source builds remain ad-hoc signed unless both identities are supplied.
The published 1.0.0 installer was accepted by Apple's notary service and passes
both stapler validation and Gatekeeper assessment as `Notarized Developer ID`.

Physical Raspberry Pi USB and IPP-over-LAN deployment remains required before
claiming hardware certification for a particular Pi image. Windows support is
tracked separately and must not be described as certified without a Windows
testbed and appropriate Microsoft package signing.

Classic CUPS PPD/filter support is deprecated upstream. The maintained
long-term path and IPP Everywhere conformance gates are documented in
`docs/FUTURE-PROOFING.md`; the v1 shared CUPS queue is not claimed as a complete
Printer Application.

This is an independent community project and is not affiliated with or
endorsed by TSC Auto ID Technology Co., Ltd.
