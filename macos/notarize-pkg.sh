#!/bin/sh
set -eu

project_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
version=$(sed -n '1p' "$project_dir/VERSION")
pkg=${1:-"$project_dir/build/TSC-DA200-OpenDriver-$version-universal-signed.pkg"}
profile=${NOTARYTOOL_PROFILE:?Set NOTARYTOOL_PROFILE to an existing local notarytool keychain profile}

test -f "$pkg" || {
  echo "Package not found: $pkg" >&2
  exit 1
}

MACOS_REQUIRE_DISTRIBUTION_SIGNATURE=1 \
  "$project_dir/macos/audit-pkg.sh" "$pkg"

xcrun notarytool submit "$pkg" \
  --keychain-profile "$profile" \
  --wait --timeout 30m
xcrun stapler staple "$pkg"
xcrun stapler validate "$pkg"
spctl --assess --type install --verbose=2 "$pkg"

echo "Notarized and stapled package: $pkg"
