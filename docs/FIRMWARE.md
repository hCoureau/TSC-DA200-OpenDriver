# Firmware policy

Firmware is not part of this driver and is never installed by a print job,
package, setup script, or diagnostic tool.

Some DA200-compatible units report firmware or USB discovery identities that
differ from the retail DA200 documentation. A version number from one firmware
branch is not evidence that an image from another branch is safe. Do not flash
a firmware image merely because its filename includes `DA200`.

## Safe procedure

1. Obtain a firmware image explicitly approved by TSC support for the exact
   physical model, current firmware branch, and region.
2. Record the vendor download URL, published version, SHA-256 digest, and
   release notes privately.
3. Save the current configuration and use stable power and a direct USB
   connection.
4. Stop CUPS jobs. Use only the vendor's documented firmware utility; never
   send a firmware image through IPP, an `lp` command, port 9100, a Raspberry
   Pi share, or this driver.
5. Do not disconnect power or USB until the vendor tool reports success and the
   printer has restarted.
6. Print a self-test, recalibrate media, and rerun the non-sensitive hardware
   acceptance suite.

If vendor support cannot confirm the exact branch compatibility, do not update
the firmware.
