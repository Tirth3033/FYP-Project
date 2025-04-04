FROM ubuntu:latest

# Set environment variables
ENV OPENWRT_VERSION=24.10.0
ENV TARGET_ARCH=mipsel
ENV TARGET_BOARD=ramips
ENV TARGET_SUBTARGET=mt7621
ENV TARGET_PROFILE="DEVICE_xiaomi_mi-router-4a-gigabit"
ENV SDK_DIR=/opt/openwrt-sdk

# Install dependencies
RUN apt update && apt install -y \
    dos2unix \
    build-essential \
    clang \
    flex \
    bison \
    gawk \
    unzip \
    wget \
    git \
    python3 \
    python3-pip \
    python3-setuptools \
    subversion \
    swig \
    rsync \
    file \
    libncurses5-dev \
    zlib1g-dev \
    g++ \
    libssl-dev \
    && rm -rf /var/lib/apt/lists/*

# Install some more dependencies
RUN apt update && apt install -y \
    zstd \
    tcpdump \
    wireshark \
    nmap \
    netcat-openbsd \
    iptables \
    net-tools \
    iproute2 \
    iputils-ping \
    traceroute \
    mtr \
    curl \
    wget \
    dnsutils \
    bridge-utils \
    ethtool \
    kmod \
    ncat \
    && rm -rf /var/lib/apt/lists/*

# Setup OpenWRT SDK
RUN mkdir -p $SDK_DIR
COPY ./sdk.tar.zst tmp/
RUN tar -xvf tmp/sdk.tar.zst -C $SDK_DIR

# Set working directory to OpenWRT SDK
WORKDIR /source

# Default command
CMD ["/bin/bash"]
