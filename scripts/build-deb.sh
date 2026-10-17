#!/usr/bin/env bash
set -euo pipefail
source_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
if (( $# )); then
    echo "Usage: $0" >&2
    exit 2
fi
cd "$source_dir"
export PACKAGE_OUTPUT="$source_dir/build-deb"
mkdir -p "$PACKAGE_OUTPUT"
source_package="$(dpkg-parsechangelog --show-field Source)"
version="$(dpkg-parsechangelog --show-field Version)"
arch="$(dpkg-architecture --query DEB_HOST_ARCH)"
stem="${source_package}_${version}_${arch}"
dpkg-buildpackage --build=binary --no-sign \
    --buildinfo-file="$PACKAGE_OUTPUT/$stem.buildinfo" \
    --buildinfo-option="-u$PACKAGE_OUTPUT" \
    --changes-file="$PACKAGE_OUTPUT/$stem.changes" \
    --changes-option="-u$PACKAGE_OUTPUT"
