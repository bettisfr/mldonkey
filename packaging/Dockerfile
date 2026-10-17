FROM ubuntu:26.04
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
    build-essential autoconf m4 pkg-config debhelper ca-certificates \
    ocaml ocaml-findlib camlp4 libcamlp-streams-ocaml-dev libnum-ocaml-dev \
    zlib1g-dev libbz2-dev libgd-dev libmagic-dev \
    && rm -rf /var/lib/apt/lists/*
WORKDIR /src
