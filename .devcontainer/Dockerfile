FROM mcr.microsoft.com/devcontainers/base:ubuntu-24.04

# Host build dependencies for MXE. MXE's build scripts pull additional
# cross libraries from source; the resulting Windows Qt SDK is installed
# separately by scripts/bootstrap-mxe.sh, not during image creation.
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
    autoconf automake autopoint bash bison bzip2 ca-certificates ccache cmake \
    flex g++ g++-multilib gettext git gperf intltool libtool-bin \
    libgdk-pixbuf2.0-dev libssl-dev libxml-parser-perl lzip make ninja-build \
    p7zip-full patch perl pkg-config python3 python3-mako python3-packaging \
    python3-pip python3-setuptools python3-six python3-yaml python-is-python3 \
    ruby sed unzip wget xz-utils zip libgl1-mesa-dev libegl1-mesa-dev \
    libgles2-mesa-dev libopengl-dev libglvnd-dev \
    && rm -rf /var/lib/apt/lists/*
