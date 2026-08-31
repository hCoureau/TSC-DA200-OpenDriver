# Future-proofing and driverless transition

## macOS 27 Golden Gate

Version 1.0.0 is designed to avoid the compatibility failures Apple has
identified for macOS 27:

- the filter contains native `arm64` and `x86_64` slices;
- the Distribution file explicitly declares both host architectures;
- every installer script is a POSIX shell script that runs natively on ARM;
- the deployment target is macOS 11.0 with no upper-version limit;
- the package contains no Intel-only plug-in, loader, or bundled runtime;
- `macos/audit-pkg.sh` opens the finished package and enforces these properties.
- CI compiles, tests, and audits the package with GitHub's newest public
  macOS/Xcode image, recording the SDK version used for every run.

This is a compatibility design target, not a macOS 27 certification. Release
certification requires installing and printing on the final public macOS 27
build. The Xcode 27 SDK CI gate catches source and packaging regressions but is
not a runtime or physical-printer test. Until that final test exists, project
documentation must say “macOS 27 compatibility target”, never “certified”.

The native ARM slice avoids Intel-only compatibility risk. It does not guarantee
that Apple will retain the classic PPD/filter printing interface indefinitely.

## CUPS 3 and Printer Applications

OpenPrinting deprecated classic PPD files and printer-model-specific filters
in CUPS 2.3, with removal planned for CUPS 3. The DA200 itself cannot become an
IPP Everywhere printer through a firmware setting, so long-term support needs
a software bridge.

The durable architecture is a PAPPL-based Printer Application running on the
Raspberry Pi:

```text
macOS / Windows / Linux / iOS / Android
                  |
        driverless IPP Everywhere
                  |
       DA200 Printer Application on Pi
                  |
      tested raster-to-TSPL rendering core
                  |
        USB or TCP/9100 to the DA200
```

This keeps the hardware-specific rendering code in one place and makes the Pi
look like a modern network printer. Clients use their built-in IPP/AirPrint/
Mopria path instead of installing a DA200-specific driver.

The current v1 CUPS queue can be shared through IPP and is useful on present
CUPS 2.x systems. Its PPD advertises common label media, but that does not make
it a complete IPP Everywhere/AirPrint Printer Application: a client exposes
only the attributes it understands, and some iOS apps provide no print action.
The stock CUPS 2.x queue does not expose every attribute required by the IPP
Everywhere conformance suite. Full cross-platform “no driver on the client”
certification is therefore a separate, explicit release gate.

## Required Printer Application gates

Before the Printer Application replaces the v1 CUPS path, it must pass:

1. The same renderer unit, malformed-input, and physical 4×6 label suite as
   the direct macOS driver.
2. OpenPrinting's complete `ipp-everywhere.test`, including actual raster jobs.
3. DNS-SD discovery from macOS and iOS, Windows 11 IPP/Mopria discovery, and a
   current Linux driverless client.
4. USB disconnect/reconnect, printer power-cycle, Pi reboot, paper-out, cover-
   open, cancelled-job, and network-interruption recovery.
5. Authentication, TLS, firewall, and untrusted-document resource-limit tests.
6. ARM64 and ARMHF package/image tests on supported Raspberry Pi OS releases.

## Platform commitments

- **macOS direct USB:** native package remains maintained while Apple ships the
  classic filter interface; macOS 27 requires an actual release test.
- **Raspberry Pi:** v1 CUPS packages are available now; the Printer Application
  becomes the primary long-term product once its gates pass.
- **Other Linux:** CUPS 2.x source/package support now, then driverless access
  to the Pi bridge for CUPS 3.x environments.
- **Windows:** driverless network access through the certified Pi bridge is the
  primary design. A direct Windows USB driver is a separate signed product and
  must not be claimed without Windows testbeds and Microsoft signing.
- **Firmware:** never coupled to driver installation; branch-specific firmware
  updates remain the separate safety process in `docs/FIRMWARE.md`.
