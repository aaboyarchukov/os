FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential \
        gcc g++ make cmake bear \
        gdb valgrind strace ltrace ltrace \
        libasan8 libubsan1 \
        linux-libc-dev \
        man-db manpages manpages-dev manpages-posix manpages-posix-dev \
        binutils elfutils file \
        procps psmisc lsof htop \
        git curl vim less \
        python3 \
    && rm -rf /var/lib/apt/lists/*

# man pages are stripped from the base image; restore them
RUN yes | unminimize || true

WORKDIR /work

CMD ["/bin/bash"]
