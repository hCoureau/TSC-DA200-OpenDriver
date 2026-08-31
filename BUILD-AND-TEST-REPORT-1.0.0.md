# TSC DA200 Open Driver 1.0.0 — Build and Test Report

Date: 2026-08-29

## Release result

The 1.0.0 source and release packages pass the available automated, packaging,
and physical-print gates. Claims are deliberately limited to the tested device
and environments described below.

## Identified printer

- Product family: TSC DA200-compatible thermal label printer
- USB identity: `1203:0252`
- Printer-reported identity and firmware branch: intentionally omitted from
  this public report
- Test media: 4 by 6 inch gap labels

## macOS validation

- Built on Apple Silicon with Xcode 26.6 and the macOS 26.5 SDK.
- Universal filter contains native `arm64` and `x86_64` slices.
- Deployment target is macOS 11.0 with no maximum OS version.
- Finished-package PPD, architecture, deployment-target, payload, and script
  audits passed.
- Installed 1.0.0 candidate printed the complete numbered physical suite over
  USB on macOS 26.6.2.
- User-supplied photographs confirm intact alignment borders, correctly placed
  functional targets, readable QR and fine-line targets, progressive ordered
  halftones, exact-white and solid-black endpoints, mirror and negative
  transforms, and the expected collated and uncollated copy groups.
- The final security-hardened package was rebuilt and statically audited. Its
  renderer changes only add rejection/clamping for malformed numeric options;
  its diagnostics change only reduces default data collection. Reinstalling
  that final package on this Mac requires an administrator password.

## macOS 27 compatibility gate

- CI builds with GitHub's newest public macOS/Xcode image and records the SDK
  version used. It does not claim availability of a non-public macOS 27 runner.
- This is an SDK/compiler/package compatibility target. macOS 27 certification
  requires a final macOS 27 runtime installation and physical printer test.

## Linux packaging validation

- Debian packages were built, installed, and regression-tested in native
  `amd64`, `arm64`, and `armhf` userspaces.
- Fedora 42 RPMs were built, installed, and regression-tested in native
  `x86_64` and `aarch64` userspaces.
- Raspberry Pi packages are therefore architecture/package validated. Physical
  Pi USB and shared-network-printer certification remains an explicit field
  gate for each supported Raspberry Pi OS image.

## Automated validation

- Encoder unit tests and CUPS raster integration tests passed.
- Collated and uncollated multi-page copy ordering passed.
- Invalid copy counts, dimensions, resolutions, short rows, truncated raster
  data, non-finite numbers, and extreme numeric values are rejected or safely
  clamped.
- AddressSanitizer and UndefinedBehaviorSanitizer runs passed.
- PPD validation and release metadata synchronization passed.
- Shell syntax validation passed.
- Source-archive hygiene and release checksums are verified during final
  assembly.

## Security review

The pre-release engineering review remediated malformed numeric CUPS options
and reduced the default diagnostics bundle. The public-source security scan is
recorded separately with the public release review, so this report makes no
claim about a later revision of the repository.

## Distribution qualifications

- The project is independently MIT licensed and contains no vendor driver or
  firmware binaries.
- The public macOS installer is Developer ID signed, notarized by Apple, and
  stapled. Local source builds are ad-hoc signed unless distribution identities
  are explicitly supplied.
- Windows and macOS 27 are compatibility targets, not certified platforms, until
  their respective runtime and hardware testbeds are exercised.
- Firmware updates are intentionally separate from driver installation and must
  never be applied without exact hardware/firmware-branch validation.

## Apple distribution evidence

- Final notary-service status: `Accepted`.
- Stapler validation: passed (`The validate action worked!`).
- Gatekeeper installer assessment: accepted, with source `Notarized Developer ID`.
- `pkgutil` reports a Developer ID Installer signature, a trusted timestamp, and
  trust by the Apple notary service.
- The embedded universal filter is signed with Developer ID Application,
  hardened runtime, and a secure timestamp.
