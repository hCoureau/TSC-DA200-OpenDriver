#!/bin/sh
set -eu

project_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
version=${1:-$(sed -n '1p' "$project_dir/VERSION")}
release_notes="$project_dir/RELEASE-$version.md"
architecture=${DEB_ARCHITECTURE:-$(dpkg --print-architecture)}
package=tsc-da200-cups
build_dir="$project_dir/build/debian"
root="$build_dir/root"
output="$project_dir/build/${package}_${version}_${architecture}.deb"

"$project_dir/scripts/check-release.sh"

case "$build_dir" in
  "$project_dir"/build/debian) ;;
  *) echo "Refusing unsafe build path: $build_dir" >&2; exit 1 ;;
esac

make -C "$project_dir" clean all
rm -rf "$build_dir"
mkdir -p "$root/DEBIAN" "$root/usr/lib/cups/filter" \
  "$root/usr/share/ppd/tsc" "$root/usr/local/bin" \
  "$root/usr/share/doc/$package"

install -m 0755 "$project_dir/build/rastertotspl" \
  "$root/usr/lib/cups/filter/rastertotspl"
install -m 0644 "$project_dir/ppd/TSC-DA200.ppd" \
  "$root/usr/share/ppd/tsc/TSC-DA200.ppd"
install -m 0644 "$project_dir/ppd/TSC-DA200-4x6.ppd" \
  "$root/usr/share/ppd/tsc/TSC-DA200-4x6.ppd"
install -m 0755 "$project_dir/tools/calibrate.sh" \
  "$root/usr/local/bin/tsc-da200-calibrate"
install -m 0755 "$project_dir/tools/diagnose.sh" \
  "$root/usr/local/bin/tsc-da200-diagnose"
install -m 0755 "$project_dir/scripts/setup-printer.sh" \
  "$root/usr/local/bin/tsc-da200-setup"
install -m 0644 "$project_dir/README.md" "$project_dir/LICENSE" \
  "$project_dir/CHANGELOG.md" \
  "$project_dir/CONTRIBUTING.md" "$project_dir/SECURITY.md" \
  "$project_dir/THIRD_PARTY_NOTICES.md" \
  "$release_notes" \
  "$project_dir/docs/FEATURES.md" "$project_dir/docs/HARDWARE-TEST.md" \
  "$project_dir/docs/FIRMWARE.md" "$project_dir/docs/FUTURE-PROOFING.md" \
  "$project_dir/docs/SUPPORT.md" "$root/usr/share/doc/$package/"

cat > "$root/DEBIAN/control" <<EOF
Package: $package
Version: $version
Section: text
Priority: optional
Architecture: $architecture
Depends: cups, cups-filters
Maintainer: TSC DA200 Open Driver Project
Description: Open CUPS raster driver for the 203 dpi TSC DA200
 Converts CUPS raster jobs to native TSPL for USB and TCP/9100 connections.
 Includes queue setup, calibration, and privacy-safe diagnostic utilities.
EOF

cat > "$root/DEBIAN/postinst" <<'EOF'
#!/bin/sh
set -e
if command -v systemctl >/dev/null 2>&1; then
  systemctl try-restart cups.service >/dev/null 2>&1 || true
elif command -v service >/dev/null 2>&1; then
  service cups restart >/dev/null 2>&1 || true
fi
exit 0
EOF
chmod 0755 "$root/DEBIAN/postinst"

find "$root" -type d -exec chmod 0755 {} \;
dpkg-deb --root-owner-group --build "$root" "$output"
echo "$output"
