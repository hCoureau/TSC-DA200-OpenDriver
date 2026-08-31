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

make -C "$project_dir" clean all printer-app
rm -rf "$build_dir"
mkdir -p "$root/DEBIAN" "$root/usr/lib/cups/filter" \
  "$root/usr/share/ppd/tsc" "$root/usr/local/bin" \
  "$root/usr/share/doc/$package" "$root/usr/sbin" \
  "$root/usr/lib/systemd/system"

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
install -m 0755 "$project_dir/build/tsc-da200-printer-app" \
  "$root/usr/sbin/tsc-da200-printer-app"
install -m 0755 "$project_dir/debian/tsc-da200-airprint-setup" \
  "$root/usr/local/bin/tsc-da200-airprint-setup"
install -m 0644 "$project_dir/debian/tsc-da200-printer-app.service" \
  "$root/usr/lib/systemd/system/tsc-da200-printer-app.service"
install -m 0644 "$project_dir/README.md" "$project_dir/LICENSE" \
  "$project_dir/CHANGELOG.md" \
  "$project_dir/CONTRIBUTING.md" "$project_dir/SECURITY.md" \
  "$project_dir/THIRD_PARTY_NOTICES.md" \
  "$release_notes" \
  "$project_dir/docs/FEATURES.md" "$project_dir/docs/HARDWARE-TEST.md" \
  "$project_dir/docs/FIRMWARE.md" "$project_dir/docs/FUTURE-PROOFING.md" \
  "$project_dir/docs/SUPPORT.md" "$project_dir/docs/AIRPRINT.md" \
  "$root/usr/share/doc/$package/"

sed -e "s/@VERSION@/$version/g" -e "s/@ARCHITECTURE@/$architecture/g" \
  "$project_dir/debian/control.in" > "$root/DEBIAN/control"
install -m 0755 "$project_dir/debian/postinst" "$root/DEBIAN/postinst"

find "$root" -type d -exec chmod 0755 {} \;
dpkg-deb --root-owner-group --build "$root" "$output"
echo "$output"
