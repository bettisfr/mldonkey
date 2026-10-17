# Building MLDonkey with OCaml 5

## Ubuntu 26.04 dependencies

```sh
sudo apt update
sudo apt install build-essential autoconf m4 pkg-config debhelper \
    ocaml ocaml-findlib camlp4 libcamlp-streams-ocaml-dev libnum-ocaml-dev \
    zlib1g-dev libbz2-dev libgd-dev libmagic-dev
```

The compiler, Camlp4 and OCaml libraries must be built for the same OCaml
version. The native package build checks the Debian build dependencies.

## Build the server

```sh
./scripts/build.sh
```

The result is `./mlnet`. The build regenerates the Autoconf script and
Makefile from their source templates. It disables the old GTK GUI, enables
bounds checks, and uses GNU C11 / C++14 for the bundled legacy C libraries.
`BUILD_JOBS=4` is the default; set it to change build parallelism.

## Build the Debian package

```sh
./scripts/build-deb.sh
```

The `.deb`, `.buildinfo` and `.changes` files are written to `build-deb/`.
The package contains a stripped native binary, documentation, the default
service options and a systemd unit. Debhelper calculates shared-library
dependencies and generates service enable/start/stop/upgrade handling.
No compiler or development libraries are required at runtime.

The build runs directly on the host. No server is installed or started by
either build command. Tests are not run by the build.

## Scope

The supported build target is the server `mlnet`. The inherited GTK GUI,
legacy optional networks and auxiliary command-line tools have not been
ported or verified as separate build targets.
