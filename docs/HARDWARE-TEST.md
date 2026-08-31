# Hardware acceptance

## Privacy boundary

Do not put printer serial numbers, full USB URIs, customer label content,
network addresses, job names, photographs containing personal information, or
diagnostic bundles in issues, pull requests, or release notes. Use generic
values such as `usb://...` and keep raw evidence private unless it has been
reviewed and redacted.

## Existing macOS evidence

The v1.0.0 release candidate was physically exercised over direct USB on a
contributor-owned TSC DA200 using 4 by 6 inch gap labels. The test device
identified with USB vendor/product `1203:0252`; its serial number and complete
discovery URI are intentionally not recorded here.

The numbered suite verified baseline geometry, fine lines, text, QR targets,
threshold/monochrome/ordered-halftone rendering, mirror and negative
transforms, and collated/uncollated two-page copy ordering. The published
macOS package was also signed, notarized, stapled, and accepted by Gatekeeper.

This evidence establishes compatibility for that tested path only. It does not
certify every firmware revision, label stock, macOS version, or DA200 unit.

## Raspberry Pi acceptance procedure

1. Capture non-sensitive platform facts:

   ```sh
   cat /etc/os-release
   dpkg --print-architecture
   lsusb -d 1203:0252
   lpinfo -v
   ```

2. Install the matching package and create the queue as described in
   [INSTALLATION.md](INSTALLATION.md).

3. Confirm the queue and available options:

   ```sh
   lpstat -t
   lpoptions -p TSC_DA200 -l
   ```

4. Print a non-sensitive 4 by 6 inch test label:

   ```sh
   lp -d TSC_DA200 -o media=4x6.Fullbleed sample-label.pdf
   ```

5. Check that exactly one label advances, content is neither inverted nor
   mirrored, all edges align, the next label stops at the tear position, and
   any barcode scans successfully.

6. For shared printing, submit one test job from a second trusted LAN client.
   Confirm that remote CUPS administration remains disabled and that the
   firewall permits TCP 631 only from the intended LAN/VPN.

If output is blank, inverted, clipped, or misaligned, retain only redacted
evidence. `tsc-da200-diagnose QUEUE` is safe by default; use
`--include-sensitive` only for a private support exchange after reviewing the
output.
