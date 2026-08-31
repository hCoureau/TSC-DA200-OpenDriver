# Contributing

Contributions are welcome under the MIT License. By submitting a contribution,
you confirm that you have the right to provide it under that license and agree
to follow the [Code of Conduct](CODE_OF_CONDUCT.md).

Do not submit vendor binaries, firmware, PPDs, logos, decompiled code, or data
copied from proprietary drivers. Tests and implementations must come from
public specifications, independently generated test data, or observations of
hardware you are authorized to use.

Before opening a change:

```sh
make clean test
make check-release
sh -n scripts/*.sh tools/*.sh macos/*.sh macos/pkg-scripts/* debian/*.sh
cupstestppd -W all ppd/TSC-DA200.ppd
```

Changes to raster polarity, media motion, density, speed, label dimensions,
copy ordering, or transforms require the numbered physical acceptance suite.
Record only redacted hardware evidence: host OS/architecture, driver version,
and non-sensitive observations. Do not commit or post printer serial numbers,
full USB/network URIs, job identifiers, document contents, raw logs, or
unreviewed photographs.

Do not claim platform support beyond the evidence rules in `docs/SUPPORT.md`.

For a release, follow [docs/PUBLISHING.md](docs/PUBLISHING.md). Releases must
contain source and checksums, never credentials, signing keys, local build
paths, or generated diagnostic bundles.
