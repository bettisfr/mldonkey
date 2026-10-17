# MLDonkey — OCaml 5 port

An independent port of the **MLDonkey 3.2.1 server** to **OCaml 5**, intended
for Ubuntu 26.04. This is a community fork, not a new upstream release.

Fork: [bettisfr/mldonkey](https://github.com/bettisfr/mldonkey), with the port
developed on the `ocaml5` branch from the upstream 3.2.1 release.

MLDonkey supplies the `mlnet` core used by KMLDonkey. It includes eDonkey,
Kad, Overnet, BitTorrent, Direct Connect, Fasttrack and HTTP/FTP support.
The old GTK interface is not built; use the Qt6/KF6 KMLDonkey frontend.

## What changed

- Updated removed OCaml APIs, byte-buffer writes and format strings.
- Added `camlp-streams` and support for current OCaml and Num library layouts.
- Updated C bindings and wrapped native bzip2/Fasttrack handles in managed
  custom blocks, as required by OCaml 5.
- Removed dependencies on private runtime channel and seek functions.
- Added Debian packaging and a systemd service with a dedicated user.
- Added a nonfatal folder access check used by KMLDonkey’s graphical path settings.
- Added a command to change completed-download folders while preserving other
  shares, priorities and network restrictions, with explicit error replies.
- Fixed single-clause search expressions and DNS worker runtime handling.

## Status and compatibility

**Experimental.** The server builds with OCaml 5.4.0 and GCC 15 on Ubuntu
26.04 amd64. Initial startup, native DNS resolution, GUI protocol 33 login,
statistics and console replies have been checked over a sustained connection.
Initial manual use with KMLDonkey has also exercised searches, live downloads
and completed files. Long-running operation, older OCaml 5 releases and other
distributions are not verified. The latest folder-setting changes still need
broader regression checks.
The legacy GTK GUI and auxiliary tools are outside the port's current scope.

The executable includes the OCaml runtime: installing the `.deb` does not
require OCaml, Opam or Docker. Its system library dependencies are determined
from the build environment. A package built on Ubuntu 26.04 is intended for
that release and architecture.

## Build and install

Clone the port branch:

```sh
git clone --branch ocaml5 git@github.com:bettisfr/mldonkey.git
cd mldonkey
```

Install the dependencies from [BUILDING.md](BUILDING.md), then run:

```sh
./scripts/build-deb.sh
sudo apt install ./build-deb/mldonkey-server_3.2.1+ocaml5.5_amd64.deb
```

Or build using Docker, without installing development packages on the host:

```sh
./scripts/build-deb.sh --docker
```

The package installs `/usr/bin/mlnet` and enables the `mldonkey-server`
systemd service. Settings, downloads and working files are stored under
`/var/lib/mldonkey`, owned by the system user `mldonkey`.

In KMLDonkey, connect to **127.0.0.1**, port **4001**, user **admin**, initially
with an empty password. The web interface is at <http://127.0.0.1:4080>.
Administration interfaces bind to localhost; Telnet is disabled. These defaults
are set in `/etc/default/mldonkey-server`.

```sh
systemctl status mldonkey-server
journalctl -u mldonkey-server
sudo systemctl restart mldonkey-server
```

Stop the service before editing its `.ini` files manually, then start it again.
Running `mlnet` directly uses upstream's normal per-user defaults; use the
packaged service for the system installation described here.

## Uninstall

```sh
sudo apt remove mldonkey-server
```

Removal stops the service. Downloaded data in `/var/lib/mldonkey` is preserved,
including when purging the package.

## Upstream

Source: [ygrek/mldonkey release 3.2.1](https://github.com/ygrek/mldonkey/releases/tag/release-3-2-1).
Original archive SHA-256:
`9c0be397ab2193256f52f7bafaf210bf29c93bb4758bb1818f4633763b996bc7`.
See [README.upstream.md](README.upstream.md) for the original project notes.
