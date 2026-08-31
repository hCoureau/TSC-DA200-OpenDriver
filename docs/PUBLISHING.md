# Release and publishing guide

## Release policy

Only tagged releases are published. Each release must include the source
archive, macOS installer, Debian packages, Fedora RPMs, `SHA256SUMS`, and
release notes. Publish only immutable versioned files; never replace a release
asset after publishing it.

## Preflight

1. Confirm the working tree contains no private keys, provisioning profiles,
   `.env` files, local logs, device serial numbers, full printer URIs, or test
   photos.
2. Run:

   ```sh
   make clean test check-release
   sh -n scripts/*.sh tools/*.sh macos/*.sh macos/pkg-scripts/* debian/*.sh rpm/*.sh
   cupstestppd -W all ppd/TSC-DA200.ppd
   ```

3. Build Debian/Raspberry Pi and Fedora packages in their documented native or
   containerized test environments.
4. Build the macOS package with Developer ID Application and Developer ID
   Installer identities. Submit it through a local Keychain notary profile;
   never store an App Store Connect private key in this repository or CI logs.
5. Validate the final macOS package with `pkgutil`, `xcrun stapler validate`,
   and `spctl --assess --type install --verbose=2`.

## Assembly

Create a clean `dist/VERSION` directory outside Git tracking. Copy only the
validated artifacts, then calculate checksums from the final copies:

```sh
find . -maxdepth 1 -type f ! -name SHA256SUMS -print | LC_ALL=C sort | \
  xargs shasum -a 256 > SHA256SUMS
shasum -a 256 -c SHA256SUMS
```

Inspect the source archive before publishing. It must not contain `build/`,
`dist/`, macOS metadata files, credentials, test output, or other generated
artifacts.

## GitHub and Homebrew

Open and merge a pull request before tagging. Create an annotated `vVERSION`
tag only from the merged commit, then publish a GitHub release from that tag.

The Homebrew cask lives in the separate `hCoureau/homebrew-tap` repository and
must reference the immutable GitHub release asset and its SHA-256. Update the
cask only after the GitHub release asset is live and verified.

## Post-release checks

Download every public release asset into a clean directory, verify its checksum,
and rerun the package-specific install/audit checks. Confirm the release page
does not disclose credentials, exact printer identifiers, customer content, or
local filesystem paths.
