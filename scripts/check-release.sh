#!/bin/sh
set -eu

project_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
version=$(sed -n '1p' "$project_dir/VERSION")
ppd="$project_dir/ppd/TSC-DA200.ppd"
release_notes="$project_dir/RELEASE-$version.md"

case "$version" in
  ''|*[!0-9.]*|.*|*.|*..*)
    echo "VERSION is not a dotted numeric release: $version" >&2
    exit 1
    ;;
esac

test -f "$release_notes" || {
  echo "Missing release notes: $release_notes" >&2
  exit 1
}

grep -Fq "*FileVersion: \"$version\"" "$ppd" || {
  echo "PPD FileVersion does not match VERSION $version" >&2
  exit 1
}
grep -Fq "Open TSPL $version" "$ppd" || {
  echo "PPD NickName does not match VERSION $version" >&2
  exit 1
}
grep -Fq "TSC DA200 Open Driver $version" "$release_notes" || {
  echo "Release-notes title does not match VERSION $version" >&2
  exit 1
}
grep -Fq 'hostArchitectures="arm64,x86_64"' \
  "$project_dir/macos/Distribution.xml.in" || {
  echo "macOS Distribution must explicitly allow arm64 and x86_64" >&2
  exit 1
}
grep -Fq '<os-version min="11.0"/>' \
  "$project_dir/macos/Distribution.xml.in" || {
  echo "macOS Distribution minimum-version contract changed" >&2
  exit 1
}
if grep -Fq 'max=' "$project_dir/macos/Distribution.xml.in"; then
  echo "macOS Distribution must not impose an upper OS-version limit" >&2
  exit 1
fi
grep -Fq 'Version:        @VERSION@' \
  "$project_dir/rpm/tsc-da200-cups.spec.in" || {
  echo "RPM spec must derive its version from VERSION" >&2
  exit 1
}

echo "Release metadata check passed for $version"
