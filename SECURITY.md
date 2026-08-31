# Security policy

## Reporting

Do not publish exploit details for an unpatched issue. Use
[GitHub Security Advisories](https://github.com/hCoureau/TSC-DA200-OpenDriver/security/advisories/new)
to report privately. Include affected versions, safe reproduction steps,
impact, and any proposed fix. Do not include printer serial numbers, full
printer URIs, customer label data, or unredacted diagnostic bundles.

## Scope

Security-sensitive areas include malformed raster input, integer and memory
safety, temporary files, installer privilege boundaries, queue setup, network
exposure, diagnostics privacy, and raw printer command injection.

The driver does not install firmware and must never treat a print job as an
authorized firmware update. TCP 631 and 9100 must not be exposed directly to
the public internet. Raspberry Pi sharing is opt-in and remote CUPS
administration remains disabled by default.

## Supported versions

The latest published stable release receives security fixes. Pre-release and
older versions may be used to reproduce a report but are not supported for
production deployment.
