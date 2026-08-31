#!/bin/sh
set -eu

project_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
version=$(sed -n '1p' "$project_dir/VERSION")
pkg=${1:-"$project_dir/build/TSC-DA200-OpenDriver-$version-universal.pkg"}
audit_root=$(mktemp -d /private/tmp/tsc-da200-audit.XXXXXX)
expanded="$audit_root/expanded"

cleanup() {
  case "$audit_root" in
    /private/tmp/tsc-da200-audit.*) rm -rf "$audit_root" ;;
    *) echo "Refusing unsafe cleanup path: $audit_root" >&2 ;;
  esac
}
trap cleanup EXIT HUP INT TERM

test -f "$pkg" || {
  echo "Package not found: $pkg" >&2
  exit 1
}

"$project_dir/scripts/check-release.sh"
pkgutil --expand-full "$pkg" "$expanded"

distribution="$expanded/Distribution"
component="$expanded/clean-component.pkg"
payload="$component/Payload"
filter="$payload/Library/Printers/TSC/OpenDA200/Filter/rastertotspl"
ppd="$payload/Library/Printers/PPDs/Contents/Resources/TSC-DA200-Open.ppd"
fixed_ppd="$payload/Library/Printers/PPDs/Contents/Resources/TSC-DA200-Open-4x6.ppd"
postinstall="$component/Scripts/postinstall"

test -f "$distribution"
test -d "$component"
test -x "$filter"
test -r "$ppd"
test -r "$fixed_ppd"
test -x "$postinstall"

grep -Fq 'hostArchitectures="arm64,x86_64"' "$distribution"
grep -Fq '<os-version min="11.0"/>' "$distribution"
! grep -Fq 'max=' "$distribution"
grep -Fq "version=\"$version\"" "$distribution"
grep -Fq 'id="org.openlabeldrivers.tsc-da200"' "$distribution"

lipo "$filter" -verify_arch arm64 x86_64
codesign --verify --strict "$filter"
if [ "${MACOS_REQUIRE_DISTRIBUTION_SIGNATURE:-0}" = "1" ]; then
  signature_details=$(codesign -dvvv "$filter" 2>&1)
  echo "$signature_details" | grep -Fq 'Authority=Developer ID Application:'
  echo "$signature_details" | grep -Eq 'flags=.*runtime|Runtime Version='
  echo "$signature_details" | grep -Fq 'Timestamp='

  package_signature=$(pkgutil --check-signature "$pkg" 2>&1)
  echo "$package_signature" | grep -Fq 'Developer ID Installer:'
  echo "$package_signature" | grep -Fq 'trusted timestamp'
fi
otool -l "$filter" | awk '
  $1 == "cmd" && $2 == "LC_BUILD_VERSION" { in_build = 1; next }
  in_build && $1 == "platform" { if ($2 != "1") exit 1; platform = 1 }
  in_build && $1 == "minos" { if ($2 != "11.0") exit 1; minos = 1; in_build = 0 }
  END { if (!platform || !minos) exit 1 }
'

cupstestppd -W all "$ppd"
cupstestppd -W all "$fixed_ppd"
grep -Fq "*FileVersion: \"$version\"" "$ppd"
grep -Fq '/Library/Printers/TSC/OpenDA200/Filter/rastertotspl' "$ppd"
test "$(sed -n '1p' "$postinstall")" = '#!/bin/sh'

if find "$expanded" -name '._*' -type f | grep -q .; then
  echo "Package contains forbidden AppleDouble files" >&2
  exit 1
fi

for required in LICENSE.txt CHANGELOG.md THIRD_PARTY_NOTICES.md README.txt \
  "RELEASE-$version.md" SUPPORT.md FUTURE-PROOFING.md; do
  test -r "$payload/Library/Printers/TSC/OpenDA200/$required" || {
    echo "Package is missing $required" >&2
    exit 1
  }
done

echo "macOS package audit passed: $pkg"
echo "  architectures: arm64 x86_64"
echo "  deployment target: macOS 11.0 (no upper bound)"
echo "  macOS 27 guardrails: explicit host architectures, native arm64 filter and scripts"
if [ "${MACOS_REQUIRE_DISTRIBUTION_SIGNATURE:-0}" = "1" ]; then
  echo "  distribution signatures: Developer ID Application + Developer ID Installer"
fi
