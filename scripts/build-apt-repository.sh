#!/bin/sh
# Build a signed, static Debian repository from already-built .deb packages.
set -eu

if [ "$#" -ne 2 ]; then
  echo "Usage: $0 PACKAGE_DIRECTORY OUTPUT_DIRECTORY" >&2
  exit 64
fi

packages_dir=$1
repository_dir=$2
suite=${APT_SUITE:-stable}
component=${APT_COMPONENT:-main}
fingerprint=${APT_GPG_KEY_FINGERPRINT:?APT_GPG_KEY_FINGERPRINT is required}

command -v apt-ftparchive >/dev/null
command -v dpkg-scanpackages >/dev/null
command -v dpkg-deb >/dev/null
command -v gpg >/dev/null

test -d "$packages_dir"
test -n "$suite"
test -n "$component"

case "$repository_dir" in
  ''|/|.|..|/*|../*|*/../*|*/..)
    echo "Refusing unsafe repository output path: $repository_dir" >&2
    exit 1
    ;;
esac

rm -rf "$repository_dir"
mkdir -p "$repository_dir/pool/$component/t/tsc-da200-cups"

found=0
for package in "$packages_dir"/*.deb; do
  test -f "$package" || continue
  found=1
  architecture=$(dpkg-deb -f "$package" Architecture)
  case "$architecture" in
    amd64|arm64|armhf) ;;
    *)
      echo "Unsupported APT package architecture: $architecture" >&2
      exit 1
      ;;
  esac
  cp "$package" "$repository_dir/pool/$component/t/tsc-da200-cups/"
done

test "$found" -eq 1 || {
  echo "No .deb packages found in $packages_dir" >&2
  exit 1
}

(
  cd "$repository_dir"
  for architecture in amd64 arm64 armhf; do
    directory="dists/$suite/$component/binary-$architecture"
    mkdir -p "$directory"
    dpkg-scanpackages --arch "$architecture" "pool/$component" /dev/null \
      > "$directory/Packages"
    gzip -9cn "$directory/Packages" > "$directory/Packages.gz"
  done

  apt-ftparchive \
    -o "APT::FTPArchive::Release::Origin=TSC DA200 Open Driver" \
    -o "APT::FTPArchive::Release::Label=TSC DA200 Open Driver" \
    -o "APT::FTPArchive::Release::Suite=$suite" \
    -o "APT::FTPArchive::Release::Codename=$suite" \
    -o "APT::FTPArchive::Release::Architectures=amd64 arm64 armhf" \
    -o "APT::FTPArchive::Release::Components=$component" \
    release "dists/$suite" > "dists/$suite/Release"

  gpg --batch --yes --local-user "$fingerprint" --armor --detach-sign \
    --output "dists/$suite/Release.gpg" "dists/$suite/Release"
  gpg --batch --yes --local-user "$fingerprint" --clearsign \
    --output "dists/$suite/InRelease" "dists/$suite/Release"
  gpg --batch --yes --export "$fingerprint" > public.gpg
  gpg --batch --yes --armor --export "$fingerprint" > public.asc
)
