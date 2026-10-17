#!/usr/bin/env bash
set -euo pipefail
source_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
if [[ "${1:-}" == "--docker" ]]; then
    docker build --tag mldonkey-build:ubuntu26.04 "$source_dir/packaging"
    exec docker run --rm --user "$(id -u):$(id -g)" \
        --env BUILD_JOBS="${BUILD_JOBS:-4}" \
        --mount "type=bind,src=$source_dir,dst=/src" --workdir /src \
        mldonkey-build:ubuntu26.04 ./scripts/build-deb.sh
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
