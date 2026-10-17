#!/usr/bin/env bash
set -euo pipefail
source_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$source_dir"
(cd config && autoconf && m4 Makefile.in > ../Makefile)
# The bundled C/C++ libraries predate C23 and C++17.
CFLAGS="${CFLAGS:--O2 -g} -std=gnu11" \
CXXFLAGS="${CXXFLAGS:--O2 -g} -std=gnu++14" \
./configure --enable-batch --disable-gui --enable-checks --disable-rpath --prefix=/usr
make depend
make --jobs="${BUILD_JOBS:-4}" mlnet
