# Installation guide

Release packages are available from the project's GitHub Releases page. Verify
the downloaded SHA-256 value against `SHA256SUMS` in the same release before
installing.

## macOS 11 or later

The release package is universal: it contains native Apple Silicon (`arm64`)
and Intel (`x86_64`) code. It is Developer ID signed, notarized, and stapled.

1. Download `TSC-DA200-OpenDriver-1.0.0-universal.pkg` and `SHA256SUMS`.
2. Verify the download:

   ```sh
   shasum -a 256 TSC-DA200-OpenDriver-1.0.0-universal.pkg
   spctl --assess --type install --verbose=2 TSC-DA200-OpenDriver-1.0.0-universal.pkg
   ```

   Gatekeeper should report `source=Notarized Developer ID`.

3. Install by double-clicking the package or running:

   ```sh
   sudo installer -pkg TSC-DA200-OpenDriver-1.0.0-universal.pkg -target /
   ```

4. Open **System Settings → Printers & Scanners**, add the printer, and choose
   **TSC DA200 203dpi (Open TSPL)** when macOS asks for driver software.

For a network-connected printer, use port 9100 only on a trusted LAN:

```sh
sudo /Library/Printers/TSC/OpenDA200/setup-printer.sh 'socket://PRINTER_IP:9100'
```

## Raspberry Pi OS / Debian (signed APT repository)

Confirm the architecture before choosing a package:

```sh
dpkg --print-architecture
```

Add the project's signed APT repository, then install the package. The
repository supports `arm64`, `armhf`, and `amd64`; `apt update` followed by
your normal upgrade command will install future driver releases automatically.
The signing key is scoped to this repository with `signed-by`, rather than
being trusted globally:

```sh
sudo apt update
sudo apt install -y curl gnupg
curl -fsSL https://hcoureau.github.io/TSC-DA200-OpenDriver/apt/public.gpg | \
  sudo gpg --dearmor -o /usr/share/keyrings/tsc-da200-open-driver.gpg
echo 'deb [signed-by=/usr/share/keyrings/tsc-da200-open-driver.gpg] https://hcoureau.github.io/TSC-DA200-OpenDriver/apt stable main' | \
  sudo tee /etc/apt/sources.list.d/tsc-da200-open-driver.list >/dev/null
sudo apt update
sudo apt install tsc-da200-cups
```

Upgrade just this package with `sudo apt update && sudo apt install --only-upgrade
tsc-da200-cups`, or upgrade all configured system packages with your usual
`sudo apt upgrade`. To inspect the available version, use `apt policy
tsc-da200-cups`.

For an offline installation, download the architecture-matched `.deb` and
`SHA256SUMS` from the GitHub release, verify the checksum, then run `sudo apt
install ./tsc-da200-cups_VERSION_ARCH.deb`.

For a USB printer, discover its exact URI and create the queue:

```sh
lpinfo -v | grep '^usb://'
sudo tsc-da200-setup 'usb://...'
```

To share the rendered queue with other devices on a trusted LAN:

```sh
sudo tsc-da200-setup --share-lan 'usb://...'
```

If a location must only ever print 4 x 6 inch labels, create a fixed-media
queue instead. Its dedicated PPD advertises just that one size, which prevents
clients from selecting a mismatched label size:

```sh
sudo tsc-da200-setup --fixed-4x6 'usb://...'
```

Allow TCP 631 only from your LAN or VPN. Do not expose CUPS or raw printer port
9100 to the public internet. Clients can add the resulting IPP queue using a
URL like `ipp://RASPBERRY_PI.local:631/printers/TSC_DA200`.

## Fedora

Choose the RPM matching `uname -m`: `x86_64` for Intel/AMD systems or `aarch64`
for 64-bit ARM systems.

```sh
sudo dnf install ./tsc-da200-cups-1.0.0-1.fc42.x86_64.rpm
lpinfo -v | grep '^usb://'
sudo tsc-da200-setup 'usb://...'
```

The release RPM is validated on Fedora 42. Other Fedora versions may require a
source build until they are tested.

## Other CUPS 2.x Linux distributions

Install the equivalent compiler and CUPS development packages, then build:

```sh
make test
sudo make install
sudo tsc-da200-setup 'usb://...'
```

`cups-config`, the CUPS headers, and a running CUPS service are required.
Classic PPD/filter support is not a CUPS 3 solution; see
[FUTURE-PROOFING.md](FUTURE-PROOFING.md).

## Windows

Direct Windows USB support is not included in v1. The supported route is to
connect the printer to the Raspberry Pi, use `--share-lan`, then add the Pi's
IPP printer in Windows. This path is designed but not yet certified on a
Windows testbed; see [SUPPORT.md](SUPPORT.md).

## iPhone and iPad through the Raspberry Pi

An Apple device sends a normal document/PDF-style job to the shared queue; the
Pi's CUPS PPD and raster filter perform the TSPL conversion. The queue
advertises monochrome media and these label sizes: 4 x 6 inch, 100 x 150 mm,
102 x 152 mm, 4 x 4 inch, 2 x 4 inch, 2 x 1 inch, and custom sizes within the
printer limits. Document orientation is selected in the client application;
the queue defaults to normal printer direction. For an iPhone/iPad installation
that always uses 4 x 6 inch labels, create the Pi queue with `--fixed-4x6`
before sharing it. The shared queue then advertises only 4 x 6 inch media to
clients that read its legacy PPD attributes.

The legacy v1 CUPS share is useful for IPP clients but is not yet certified as
an IPP Everywhere/AirPrint Printer Application. iOS will expose only options
the shared queue advertises, and some iOS apps do not offer printing at all.
Do not promise universal iOS printing until the Printer Application gates in
[FUTURE-PROOFING.md](FUTURE-PROOFING.md) are complete.

## First label and diagnostics

Before production use, print a non-sensitive test label and follow
[HARDWARE-TEST.md](HARDWARE-TEST.md). To calibrate gap media:

```sh
tsc-da200-calibrate TSC_DA200 gap
```

Use `tsc-da200-diagnose TSC_DA200` for a redacted support bundle. Review any
`--include-sensitive` output before sharing it.
