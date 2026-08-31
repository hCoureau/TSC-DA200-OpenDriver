#!/bin/sh
set -eu

project_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
version=${1:-$(sed -n '1p' "$project_dir/VERSION")}
package=tsc-da200-cups
build_dir="$project_dir/build/rpm"
top_dir="$build_dir/top"
stage_dir=$(mktemp -d "${TMPDIR:-/tmp}/tsc-da200-rpm.XXXXXX")

cleanup() {
  rm -rf "$stage_dir"
}
trap cleanup EXIT HUP INT TERM

"$project_dir/scripts/check-release.sh"

case "$build_dir" in
  "$project_dir"/build/rpm) ;;
  *) echo "Refusing unsafe build path: $build_dir" >&2; exit 1 ;;
esac

rm -rf "$build_dir"
mkdir -p "$top_dir/BUILD" "$top_dir/BUILDROOT" "$top_dir/RPMS" \
  "$top_dir/SOURCES" "$top_dir/SPECS" "$top_dir/SRPMS"

ln -s "$project_dir" "$stage_dir/$package-$version"
tar -h -C "$stage_dir" \
  --exclude="$package-$version/build" \
  --exclude="$package-$version/.git" \
  --exclude="$package-$version/outputs" \
  -czf "$top_dir/SOURCES/$package-$version.tar.gz" "$package-$version"

sed "s/@VERSION@/$version/g" "$project_dir/rpm/tsc-da200-cups.spec.in" \
  > "$top_dir/SPECS/tsc-da200-cups.spec"

rpmbuild -bb --define "_topdir $top_dir" "$top_dir/SPECS/tsc-da200-cups.spec"
find "$top_dir/RPMS" -type f -name '*.rpm' -print
