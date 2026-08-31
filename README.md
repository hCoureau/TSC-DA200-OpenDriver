# TSC DA200 Open Driver

[![CI](https://github.com/hCoureau/TSC-DA200-OpenDriver/actions/workflows/ci.yml/badge.svg)](https://github.com/hCoureau/TSC-DA200-OpenDriver/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

An independent, open-source CUPS raster driver for 203 dpi TSC DA200 label
printers. It converts CUPS raster output into native TSPL `BITMAP` jobs for USB
and TCP port 9100 connections.

> This project is not affiliated with or endorsed by TSC Auto ID Technology
> Co., Ltd. `TSC` and `DA200` are used only to identify printer compatibility.

## What works in v1

- Native macOS filter for Apple Silicon and Intel, distributed in a Developer
  ID-signed, notarized installer.
- Raspberry Pi OS / Debian packages for ARM64 and ARMHF.
- Fedora RPM packages for x86_64 and AArch64.
- USB and LAN printer connections; a fixed-4x6 Raspberry Pi AirPrint service
  for trusted local networks.
- Gap, black-mark, and continuous media; speed, density, offsets, direction,
  tear/peel; threshold and ordered halftone; mirror, negative, copies, and
  CUPS collation.
- 4 × 6 inch (102 × 152 mm) is the default. `--fixed-4x6` creates a queue
  backed by a one-size PPD for installations that must never select another
  label size.

See [platform support](docs/SUPPORT.md) for evidence-based support status and
[features](docs/FEATURES.md) for the full capability matrix.

## Installation

Choose the matching path in the [installation guide](docs/INSTALLATION.md):

| Platform | Recommended path |
| --- | --- |
| Apple Silicon or Intel macOS | Download the notarized `.pkg` from the GitHub release |
| Raspberry Pi OS / Debian | Install the matching `.deb` (`arm64` or `armhf`) |
| Fedora | Install the matching `.rpm` (`x86_64` or `aarch64`) |
| Other CUPS 2.x Linux | Build from source |
| Windows | Print through a Raspberry Pi IPP share; direct Windows USB is not implemented |

Never expose CUPS (TCP 631) or a printer's raw port 9100 directly to the
internet. Use a trusted LAN/VPN for remote printing.

## Quick start on Raspberry Pi

```sh
sudo apt install ./tsc-da200-cups_1.1.0_arm64.deb
lpinfo -v | grep '^usb://'
sudo tsc-da200-setup --share-lan 'usb://...'
```

The discovery URI is specific to the host and printer. Copy it exactly from
`lpinfo -v`; do not guess a URI from a product name.

For a fixed 4 × 6 AirPrint queue, use the separate Printer Application:

```sh
sudo tsc-da200-airprint-setup 'usb://...'
```

See the [AirPrint Pi guide](docs/AIRPRINT.md) for the iOS acceptance test and
the one-queue safety model.

## Build from source

```sh
sudo apt install build-essential libcups2-dev cups cups-filters
make test
sudo make install
sudo systemctl restart cups
```

Run `make check-release` before packaging, and consult
[hardware acceptance](docs/HARDWARE-TEST.md) before claiming a new platform or
printer configuration as certified.

## Safety, privacy, and firmware

- Diagnostics redact printer URIs, device IDs, job metadata, and logs by
  default. Review the output before sharing anything generated with
  `--include-sensitive`.
- Firmware updates are deliberately out of scope for this driver. Follow the
  model- and firmware-branch-specific process in [the firmware policy](docs/FIRMWARE.md).
- Security reports belong in [private GitHub Security Advisories](SECURITY.md),
  not public issues.

## Contributing and license

Contributions are welcome. Read [CONTRIBUTING.md](CONTRIBUTING.md),
[CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md), and [SECURITY.md](SECURITY.md) before
opening a pull request. This project is released under the [MIT License](LICENSE).
