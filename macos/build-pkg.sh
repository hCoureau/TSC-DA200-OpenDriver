#!/bin/sh
set -eu

project_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
version=${1:-$(sed -n '1p' "$project_dir/VERSION")}
build_dir=$(mktemp -d /private/tmp/tsc-da200-pkg.XXXXXX)
payload_dir="$build_dir/payload"
filter_dir="$payload_dir/Library/Printers/TSC/OpenDA200/Filter"
resource_dir="$payload_dir/Library/Printers/PPDs/Contents/Resources"
support_dir="$payload_dir/Library/Printers/TSC/OpenDA200"
output_pkg="$project_dir/build/TSC-DA200-OpenDriver-$version-universal.pkg"
component_pkg="$build_dir/component.pkg"
clean_component_pkg="$build_dir/clean-component.pkg"
distribution="$build_dir/Distribution.xml"
filter_path=/Library/Printers/TSC/OpenDA200/Filter/rastertotspl

cleanup() {
  case "$build_dir" in
    /private/tmp/tsc-da200-pkg.*) rm -rf "$build_dir" ;;
    *) echo "Refusing unsafe cleanup path: $build_dir" >&2 ;;
  esac
}
trap cleanup EXIT HUP INT TERM

mkdir -p "$filter_dir" "$resource_dir" "$support_dir"
mkdir -p "$project_dir/build"

cc -O2 -Wall -Wextra -Werror -arch arm64 -arch x86_64 \
  -mmacosx-version-min=11.0 -I"$project_dir/src" \
  -o "$filter_dir/rastertotspl" \
  "$project_dir/src/rastertotspl.c" "$project_dir/src/tspl.c" -lcups
lipo "$filter_dir/rastertotspl" -verify_arch arm64 x86_64
if [ -n "${MACOS_APPLICATION_IDENTITY:-}" ]; then
  codesign --force --sign "$MACOS_APPLICATION_IDENTITY" \
    --options runtime --timestamp "$filter_dir/rastertotspl"
else
  codesign --force --sign - --timestamp=none "$filter_dir/rastertotspl"
fi

sed "s| 0 rastertotspl\"| 0 $filter_path\"|g" \
  "$project_dir/ppd/TSC-DA200.ppd" \
  > "$resource_dir/TSC-DA200-Open.ppd"
sed "s| 0 rastertotspl\"| 0 $filter_path\"|g" \
  "$project_dir/ppd/TSC-DA200-4x6.ppd" \
  > "$resource_dir/TSC-DA200-Open-4x6.ppd"
install -m 0755 "$project_dir/macos/setup-printer.sh" \
  "$support_dir/setup-printer.sh"
install -m 0755 "$project_dir/tools/calibrate.sh" \
  "$support_dir/calibrate.sh"
install -m 0755 "$project_dir/tools/diagnose.sh" \
  "$support_dir/diagnose.sh"
install -m 0644 "$project_dir/macos/README-macOS.txt" \
  "$support_dir/README.txt"
install -m 0644 "$project_dir/LICENSE" "$support_dir/LICENSE.txt"
install -m 0644 "$project_dir/CHANGELOG.md" "$support_dir/CHANGELOG.md"
install -m 0644 "$project_dir/THIRD_PARTY_NOTICES.md" \
  "$support_dir/THIRD_PARTY_NOTICES.md"
install -m 0644 "$project_dir/RELEASE-$version.md" \
  "$support_dir/RELEASE-$version.md"
install -m 0644 "$project_dir/docs/SUPPORT.md" \
  "$support_dir/SUPPORT.md"
install -m 0644 "$project_dir/docs/FUTURE-PROOFING.md" \
  "$support_dir/FUTURE-PROOFING.md"
/usr/bin/xattr -cr "$payload_dir"
find "$payload_dir" -name '._*' -type f -delete

COPYFILE_DISABLE=1 pkgbuild --root "$payload_dir" \
  --scripts "$project_dir/macos/pkg-scripts" \
  --identifier org.openlabeldrivers.tsc-da200 \
  --version "$version" \
  --install-location / \
  "$component_pkg"

# macOS can force the com.apple.provenance xattr back onto build inputs after
# xattr removal. pkgbuild serializes that metadata as AppleDouble `._` files.
# Rebuild the two cpio archives explicitly so those host-only files can never
# be installed under /Library on another Mac.
expanded_dir="$build_dir/expanded"
clean_payload="$build_dir/clean-payload"
clean_scripts="$build_dir/clean-scripts"
pkgutil --expand "$component_pkg" "$expanded_dir"
mkdir -p "$clean_payload"
(cd "$clean_payload" && gzip -dc "$expanded_dir/Payload" | cpio -idm --quiet)
find "$clean_payload" -name '._*' -type f -delete
mv "$expanded_dir/Scripts" "$clean_scripts"
find "$clean_scripts" -name '._*' -type f -delete
rm "$expanded_dir/Payload"
(cd "$clean_payload" && find . -print | cpio -o -H odc --owner 0:80 2>/dev/null | gzip -9) \
  > "$expanded_dir/Payload"
mv "$clean_scripts" "$expanded_dir/Scripts"
mkbom "$clean_payload" "$expanded_dir/Bom"
payload_files=$(find "$clean_payload" -mindepth 1 | wc -l | tr -d ' ')
payload_kbytes=$(du -sk "$clean_payload" | awk '{print $1}')
sed -E "s/numberOfFiles=\"[0-9]+\" installKBytes=\"[0-9]+\"/numberOfFiles=\"$payload_files\" installKBytes=\"$payload_kbytes\"/" \
  "$expanded_dir/PackageInfo" > "$expanded_dir/PackageInfo.clean"
mv "$expanded_dir/PackageInfo.clean" "$expanded_dir/PackageInfo"
pkgutil --flatten "$expanded_dir" "$clean_component_pkg"
sed "s|@VERSION@|$version|g" "$project_dir/macos/Distribution.xml.in" \
  > "$distribution"
productbuild --distribution "$distribution" --package-path "$build_dir" \
  "$output_pkg"

if [ -n "${MACOS_INSTALLER_IDENTITY:-}" ]; then
  signed_pkg="$project_dir/build/TSC-DA200-OpenDriver-$version-universal-signed.pkg"
  productsign --sign "$MACOS_INSTALLER_IDENTITY" "$output_pkg" "$signed_pkg"
  output_pkg=$signed_pkg
fi

if [ -n "${MACOS_APPLICATION_IDENTITY:-}" ] || \
   [ -n "${MACOS_INSTALLER_IDENTITY:-}" ]; then
  test -n "${MACOS_APPLICATION_IDENTITY:-}" || {
    echo "MACOS_APPLICATION_IDENTITY is required for a distribution build" >&2
    exit 1
  }
  test -n "${MACOS_INSTALLER_IDENTITY:-}" || {
    echo "MACOS_INSTALLER_IDENTITY is required for a distribution build" >&2
    exit 1
  }
  MACOS_REQUIRE_DISTRIBUTION_SIGNATURE=1 \
    "$project_dir/macos/audit-pkg.sh" "$output_pkg"
else
  "$project_dir/macos/audit-pkg.sh" "$output_pkg"
fi

echo "$output_pkg"
